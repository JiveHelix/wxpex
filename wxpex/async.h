/**
  * @file async.h
  *
  * @brief Asynchronous communication between worker threads and wxWidgets
  * event loop.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 22 Mar 2022
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#pragma once


#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>
#include <optional>

#include <jive/comparison_operators.h>
#include <pex/endpoint.h>
#include <pex/value.h>
#include <pex/traits.h>
#include <pex/interface.h>
#include <pex/range.h>

#include "wxpex/wxshim.h"


namespace wxpex
{


template<typename T, typename Filter = pex::NoFilter>
class Async: public wxEvtHandler
{
public:
    static_assert(
        jive::HasEqualTo<T>,
        "type must support operator==");

    static constexpr auto observerName = "wxpex::Async";

    using Type = T;
    using ThreadSafe = pex::model::LockedValue<Type, Filter>;
    using Callable = typename ThreadSafe::Callable;

    template<typename>
    friend class pex::Reference;

    template<typename ControlFilter, typename ControlAccess>
    struct FilteredControl
        :
        public pex::control::Value_<ThreadSafe, ControlFilter, ControlAccess>
    {
        using Base =
            pex::control::Value_<ThreadSafe, ControlFilter, ControlAccess>;

        FilteredControl()
            :
            Base(),
            async_(nullptr)
        {

        }

        FilteredControl(const FilteredControl &other)
            :
            Base(other),
            async_(other.async_)
        {

        }

        FilteredControl & operator=(const FilteredControl &other)
        {
            if (&other == this)
            {
                return *this;
            }

            this->Base::operator=(other);
            this->async_ = other.async_;

            return *this;
        }

        template<typename, typename>
        friend struct FilteredControl;

        template<typename OtherFilter, typename OtherAccess>
        FilteredControl(const FilteredControl<OtherFilter, OtherAccess> &other)
            :
            Base(other),
            async_(other.async_)
        {

        }

        template<typename OtherFilter, typename OtherAccess>
        FilteredControl & operator=(
            const FilteredControl<OtherFilter, OtherAccess> &other)
        {
            this->Base::operator=(other);
            this->async_ = other.async_;

            return *this;
        }

        FilteredControl(ThreadSafe &threadSafe, Async *async)
            :
            Base(threadSafe),
            async_(async)
        {

        }

        FilteredControl GetWorkerControl()
        {
            if (!this->async_)
            {
                throw std::logic_error("Unitialized control");
            }

            return FilteredControl(this->async_->workerModel_, this->async_);
        }

    private:
        Async *async_;
    };

    using Control = FilteredControl<pex::NoFilter, pex::GetAndSetTag>;

    Async(pex::Argument<Type> value = Type{})
        :
        mutex_(),
        ignoredValue_(),
        model_(value),
        endpoint_(this, Control(this->model_, this)),
        workerQueuedValues_(),
        workerModel_(value),
        workerEndpoint_(this, Control(this->workerModel_, this))
    {
        this->Initialize_();
    }

    Async(pex::Argument<Type> value, Filter filter)
        :
        mutex_(),
        ignoredValue_(),
        model_(value, filter),
        endpoint_(this, Control(this->model_, this)),
        workerQueuedValues_(),
        workerModel_(value, filter),
        workerEndpoint_(this, Control(this->workerModel_, this))
    {
        this->Initialize_();
    }

    Async(Filter filter)
        :
        mutex_(),
        ignoredValue_(),
        model_(filter),
        endpoint_(this, Control(this->model_, this)),
        workerQueuedValues_(),
        workerModel_(filter),
        workerEndpoint_(this, Control(this->workerModel_, this))
    {
        this->Initialize_();
    }

    Async(const Async<Type, Filter> &) = delete;
    Async(Async<Type, Filter> &&) = delete;

protected:
    void Initialize_()
    {
        this->Bind(wxEVT_THREAD, &Async::OnWxEventLoop_, this);
        this->endpoint_.Connect(&Async::OnWxChanged_);
        this->workerEndpoint_.Connect(&Async::OnWorkerChanged_);
    }

public:
    Control GetWorkerControl()
    {
        return Control(this->workerModel_, this);
    }

    Control GetWxControl()
    {
        return Control(this->model_, this);
    }

    Async & operator=(pex::Argument<Type> value)
    {
        this->model_.Set(value);
        return *this;
    }

    void SetFilter(Filter filter)
    {
        this->model_.SetFilter(filter);
        this->workerModel_.SetFilter(filter);
    }

    void Set(pex::Argument<Type> value)
    {
        this->SetWithoutNotify_(value);
        this->DoNotify_();
    }

    Type Get() const
    {
        return this->model_.Get();
    }

    explicit operator Type () const
    {
        return this->model_.Get();
    }

    // The defaut control is for the wx event loop.
    operator Control ()
    {
        return Control(this->model_, this);
    }

    void Connect(void * observer, Callable callable)
    {
        this->model_.Connect(observer, callable);
    }

    void Disconnect(void * observer)
    {
        this->model_.Disconnect(observer);
    }

private:
    void OnWorkerChanged_(pex::Argument<Type> value)
    {
        if (this->ignoredValue_ && value == *this->ignoredValue_)
        {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            this->workerQueuedValues_.push(value);
        }

        // Queue the event for the wxWidgets event loop.
        this->QueueEvent(new wxThreadEvent());
    }

    void OnWxEventLoop_(wxThreadEvent &)
    {
        // Retrieve values from the worker queue
        Type value;

        {
            std::lock_guard<std::mutex> lock(this->mutex_);

            if (this->workerQueuedValues_.empty())
            {
                return;
            }

            value = this->workerQueuedValues_.front();
            this->workerQueuedValues_.pop();

            if (!this->workerQueuedValues_.empty())
            {
                // Repeat until all queued values have been forwarded.
                this->QueueEvent(new wxThreadEvent());
            }
        }

        this->ignoredValue_ = value;
        this->model_.Set(value);
        this->ignoredValue_.reset();
    }

    void OnWxChanged_(pex::Argument<Type> value)
    {
        if (this->ignoredValue_ && value == *this->ignoredValue_)
        {
            return;
        }

        this->ignoredValue_ = value;
        this->workerModel_.Set(value);
        this->ignoredValue_.reset();
    }


    void SetWithoutNotify_(pex::Argument<Type> value)
    {
        pex::detail::AccessReference<ThreadSafe>(this->model_)
            .SetWithoutNotify(value);
    }

    void DoNotify_()
    {
        pex::detail::AccessReference<ThreadSafe>(this->model_).DoNotify();
    }

private:
    mutable std::mutex mutex_;
    std::optional<Type> ignoredValue_;
    ThreadSafe model_;
    pex::Endpoint<Async, Control> endpoint_;
    std::queue<Type> workerQueuedValues_;
    ThreadSafe workerModel_;
    pex::Endpoint<Async, Control> workerEndpoint_;
};


template<typename T, typename Filter = pex::NoFilter>
using MakeAsync = pex::MakeCustom<Async<T, Filter>>;


class CallAfter: public wxEvtHandler
{
public:
    using Function = std::function<void()>;

    CallAfter(const Function &function)
        :
        function_(function)
    {
        this->Bind(wxEVT_THREAD, &CallAfter::OnWxEventLoop_, this);
    }

    void operator()()
    {
        this->QueueEvent(new wxThreadEvent());
    }

private:
    void OnWxEventLoop_(wxThreadEvent &)
    {
        this->function_();
    }

private:
    std::function<void()> function_;
};


class AsyncSignal: public wxEvtHandler
{
public:
    static constexpr auto observerName = "wxpex::AsyncSignal";

    using ThreadSafe = pex::model::Signal;
    using Callable = typename ThreadSafe::Callable;

    struct Control
        :
        public pex::control::Signal<>
    {
        using Base = pex::control::Signal<>;

        Control()
            :
            Base(),
            async_(nullptr)
        {

        }

        Control(const Control &other)
            :
            Base(other),
            async_(other.async_)
        {

        }

        Control & operator=(const Control &other)
        {
            if (&other == this)
            {
                return *this;
            }

            this->Base::operator=(other);
            this->async_ = other.async_;

            return *this;
        }

        Control(ThreadSafe &threadSafe, AsyncSignal *async)
            :
            Base(threadSafe),
            async_(async)
        {

        }

        Control GetWorkerControl()
        {
            if (!this->async_)
            {
                throw std::logic_error("Unitialized control");
            }

            return Control(this->async_->workerModel_, this->async_);
        }

    private:
        AsyncSignal *async_;
    };


    AsyncSignal()
        :
        mutex_(),
        ignoreTrigger_(),
        model_(),
        endpoint_(this, Control(this->model_, this)),
        workerModel_(),
        workerEndpoint_(this, Control(this->workerModel_, this))
    {
        this->Initialize_();
    }

    AsyncSignal(const AsyncSignal &) = delete;
    AsyncSignal(AsyncSignal &&) = delete;

protected:
    void Initialize_()
    {
        this->Bind(wxEVT_THREAD, &AsyncSignal::OnWxEventLoop_, this);
        this->endpoint_.Connect(&AsyncSignal::OnWxChanged_);
        this->workerEndpoint_.Connect(&AsyncSignal::OnWorkerChanged_);
    }

public:
    Control GetWorkerControl()
    {
        return Control(this->workerModel_, this);
    }

    Control GetWxControl()
    {
        return Control(this->model_, this);
    }

    void Trigger()
    {
        this->model_.Trigger();
    }
    // The defaut control is for the wx event loop.
    operator Control ()
    {
        return Control(this->model_, this);
    }

    void Connect(void * observer, Callable callable)
    {
        this->model_.Connect(observer, callable);
    }

    void Disconnect(void * observer)
    {
        this->model_.Disconnect(observer);
    }

private:
    void OnWorkerChanged_()
    {
        if (this->ignoreTrigger_)
        {
            return;
        }

        // Queue the event for the wxWidgets event loop.
        this->QueueEvent(new wxThreadEvent());
    }

    void OnWxEventLoop_(wxThreadEvent &)
    {
        this->ignoreTrigger_ = true;
        this->model_.Trigger();
        this->ignoreTrigger_ = false;
    }

    void OnWxChanged_()
    {
        if (this->ignoreTrigger_)
        {
            return;
        }

        this->ignoreTrigger_ = true;
        this->workerModel_.Trigger();
        this->ignoreTrigger_ = false;
    }

private:
    mutable std::mutex mutex_;
    bool ignoreTrigger_;
    ThreadSafe model_;
    pex::Endpoint<AsyncSignal, Control> endpoint_;
    ThreadSafe workerModel_;
    pex::Endpoint<AsyncSignal, Control> workerEndpoint_;
};


static_assert(pex::IsControlSignal<typename AsyncSignal::Control>);


template<typename Control>
class SetWait
{
public:
    using ValueType = typename Control::Type;
    using AsyncValue = Async<ValueType>;
    using WorkerControl = typename AsyncValue::Control;

    using Endpoint =
        pex::Endpoint<SetWait, WorkerControl>;

    SetWait(Control control)
        :
        mutex_(),
        condition_(),
        isWaiting_(false),
        control_(control),
        async_(control.Get()),
        endpoint_(this, this->async_.GetWxControl(), &SetWait::OnMainThread_),
        workerControl_(this->async_.GetWorkerControl())
    {

    }

    void Set(const ValueType &value)
    {
        this->isWaiting_ = true;
        this->workerControl_.Set(value);

        {
            std::unique_lock lock(this->mutex_);

            if (this->isWaiting_)
            {
                // Wait for the settings to be updated in the GUI thread.
                this->condition_.wait(
                    lock,
                    [this]() -> bool
                    {
                        return !this->isWaiting_;
                    });
            }
        }
    }

    Control GetControl() const
    {
        return this->control_;
    }

private:
    void OnMainThread_(pex::Argument<ValueType> value)
    {
        this->control_.Set(value);

        // The main thread has been notified.
        // Notify the waiting condition.
        std::lock_guard lock(this->mutex_);
        this->isWaiting_ = false;
        this->condition_.notify_one();
    }

private:
    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic_bool isWaiting_;

    Control control_;
    AsyncValue async_;
    Endpoint endpoint_;
    WorkerControl workerControl_;
};


class TriggerWait
{
public:
    using Control = pex::control::Signal<>;
    using WorkerControl = typename AsyncSignal::Control;

    using Endpoint =
        pex::Endpoint<TriggerWait, WorkerControl>;

    TriggerWait(const Control &control)
        :
        mutex_(),
        condition_(),
        isWaiting_(false),
        control_(control),
        asyncSignal_(),

        endpoint_(
            this,
            this->asyncSignal_.GetWxControl(),
            &TriggerWait::OnMainThread_),

        workerControl_(this->asyncSignal_.GetWorkerControl())
    {

    }

    void Trigger()
    {
        this->isWaiting_ = true;
        this->workerControl_.Trigger();

        {
            std::unique_lock lock(this->mutex_);

            if (this->isWaiting_)
            {
                // Wait for the settings to be updated in the GUI thread.
                this->condition_.wait(
                    lock,
                    [this]() -> bool
                    {
                        return !this->isWaiting_;
                    });
            }
        }
    }

    Control GetControl() const
    {
        return this->control_;
    }

private:
    void OnMainThread_()
    {
        this->control_.Trigger();

        // The main thread has been notified.
        // Notify the waiting condition.
        std::lock_guard lock(this->mutex_);
        this->isWaiting_ = false;
        this->condition_.notify_one();
    }

private:
    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic_bool isWaiting_;

    Control control_;
    AsyncSignal asyncSignal_;
    Endpoint endpoint_;
    WorkerControl workerControl_;
};


} // end namespace wxpex
