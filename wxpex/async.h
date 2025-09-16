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


template<typename, typename, typename>
class AsyncMux;


// Adds GetWorkerControl to pex::control::Value_.
template
<
    typename Upstream,
    typename ControlFilter = pex::NoFilter,
    typename ControlAccess = pex::GetAndSetTag
>
class AsyncControl
    :
    public pex::control::Value_
        <
            typename Upstream::ThreadSafe,
            ControlFilter,
            ControlAccess
        >
{
public:
    using ThreadSafe = typename Upstream::ThreadSafe;

    using Base =
        pex::control::Value_<ThreadSafe, ControlFilter, ControlAccess>;

    AsyncControl()
        :
        Base(),
        async_(nullptr)
    {

    }

    AsyncControl(Upstream &upstream)
        :
        Base(upstream.node_),
        async_(&upstream)
    {

    }

    AsyncControl(const AsyncControl &other)
        :
        Base(other),
        async_(other.async_)
    {

    }

    AsyncControl & operator=(const AsyncControl &other)
    {
        if (&other == this)
        {
            return *this;
        }

        this->Base::operator=(other);
        this->async_ = other.async_;

        return *this;
    }

    template<typename, typename, typename>
    friend class AsyncControl;

    template<typename OtherFilter, typename OtherAccess>
    AsyncControl(const AsyncControl<Upstream, OtherFilter, OtherAccess> &other)
        :
        Base(other),
        async_(other.async_)
    {

    }

    template<typename OtherFilter, typename OtherAccess>
    AsyncControl & operator=(
        const AsyncControl<Upstream, OtherFilter, OtherAccess> &other)
    {
        this->Base::operator=(other);
        this->async_ = other.async_;

        return *this;
    }

    AsyncControl(ThreadSafe &threadSafe, Upstream *async)
        :
        Base(threadSafe),
        async_(async)
    {

    }

    AsyncControl GetWorkerControl()
    {
        if (!this->async_)
        {
            throw std::logic_error("Unitialized control");
        }

        return AsyncControl(this->async_->workerNode_, this->async_);
    }

    bool HasModel() const
    {
        return this->async_ != NULL;
    }

private:
    Upstream *async_;
};




template
<
    typename T,
    typename Filter = pex::NoFilter,
    typename Access_ = pex::GetAndSetTag
>
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
    using Access = Access_;

    template<typename>
    friend class pex::Reference;

    template<typename, typename, typename>
    friend class AsyncControl;

    template<typename, typename, typename>
    friend class AsyncMux;

    using Unfiltered = AsyncControl<Async, pex::NoFilter, Access>;


    Async(pex::Argument<Type> value = Type{})
        :
        mutex_(),
        ignoredValue_(),
        node_(value),

        endpoint_(
            PEX_THIS("wxpex::Async"),
            Unfiltered(PEX_MEMBER_PASS(node_), this)),

        workerQueuedValues_(),
        workerNode_(value),
        workerEndpoint_(this, Unfiltered(this->workerNode_, this))
    {
        this->Initialize_();
    }

    Async(pex::Argument<Type> value, Filter filter)
        :
        mutex_(),
        ignoredValue_(),
        node_(value, filter),

        endpoint_(
            PEX_THIS("wxpex::Async"),
            Unfiltered(PEX_MEMBER_PASS(node_), this)),

        workerQueuedValues_(),
        workerNode_(value, filter),
        workerEndpoint_(this, Unfiltered(this->workerNode_, this))
    {
        this->Initialize_();
    }

    Async(Filter filter)
        :
        mutex_(),
        ignoredValue_(),
        node_(filter),

        endpoint_(
            PEX_THIS("wxpex::Async"),
            Unfiltered(PEX_MEMBER_PASS(node_), this)),

        workerQueuedValues_(),
        workerNode_(filter),
        workerEndpoint_(this, Unfiltered(this->workerNode_, this))
    {
        this->Initialize_();
    }

    Async(const Async<Type, Filter> &) = delete;
    Async(Async<Type, Filter> &&) = delete;

protected:
    void Initialize_()
    {
        PEX_MEMBER(workerNode_);

        this->Bind(wxEVT_THREAD, &Async::OnWxEventLoop_, this);
        this->endpoint_.Connect(&Async::OnWxChanged_);
        this->workerEndpoint_.Connect(&Async::OnWorkerChanged_);
    }

public:
    Unfiltered GetWorkerControl()
    {
        return Unfiltered(this->workerNode_, this);
    }

    Unfiltered GetWxControl()
    {
        return Unfiltered(*this);
    }

    Async & operator=(pex::Argument<Type> value)
    {
        this->node_.Set(value);
        return *this;
    }

    void SetFilter(Filter filter)
    {
        this->node_.SetFilter(filter);
        this->workerNode_.SetFilter(filter);
    }

    void Set(pex::Argument<Type> value)
    {
        this->SetWithoutNotify_(value);
        this->Notify();
    }

    Type Get() const
    {
        return this->node_.Get();
    }

    explicit operator Type () const
    {
        return this->node_.Get();
    }

    // The defaut control is for the wx event loop.
    // operator Unfiltered ()
    // {
    //     return Unfiltered(this);
    // }

    void Connect(void * observer, Callable callable)
    {
        this->node_.Connect(observer, callable);
    }

    void Disconnect(void * observer)
    {
        this->node_.Disconnect(observer);
    }

    void Notify()
    {
        pex::detail::AccessReference<ThreadSafe>(this->node_).Notify();
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
        this->node_.Set(value);
        this->ignoredValue_.reset();
    }

    void OnWxChanged_(pex::Argument<Type> value)
    {
        if (this->ignoredValue_ && value == *this->ignoredValue_)
        {
            return;
        }

        this->ignoredValue_ = value;
        this->workerNode_.Set(value);
        this->ignoredValue_.reset();
    }


    void SetWithoutNotify_(pex::Argument<Type> value)
    {
        pex::detail::AccessReference<ThreadSafe>(this->node_)
            .SetWithoutNotify(value);
    }

private:
    mutable std::mutex mutex_;
    std::optional<Type> ignoredValue_;
    ThreadSafe node_;
    pex::Endpoint<Async, Unfiltered> endpoint_;
    std::queue<Type> workerQueuedValues_;
    ThreadSafe workerNode_;
    pex::Endpoint<Async, Unfiltered> workerEndpoint_;
};


template
<
    typename T,
    typename Filter,
    typename Access_
>
class AsyncMux
{
public:
    using Upstream = Async<T, Filter, Access_>;

    using Mux =
        pex::control::Mux<typename Upstream::ThreadSafe>;

    using ThreadSafe = Mux;

    using Type = typename Mux::Type;
    using Callable = typename Mux::Callable;
    using Access = typename Mux::Access;

    template<typename>
    friend class pex::Reference;

    template<typename, typename, typename>
    friend class AsyncControl;

    template<typename ControlFilter, typename ControlAccess>
    using Follow = AsyncControl<AsyncMux, ControlFilter, ControlAccess>;

    using Unfiltered = Follow<pex::NoFilter, Access>;

    AsyncMux()
        :
        node_(),
        workerNode_()
    {
        PEX_MEMBER(node_);
        PEX_MEMBER(workerNode_);
    }

    AsyncMux(Upstream &upstream)
        :
        node_(upstream.node_),
        workerNode_(upstream.workerNode_)
    {
        PEX_MEMBER(node_);
        PEX_MEMBER(workerNode_);
    }

    AsyncMux(const AsyncMux &) = delete;
    AsyncMux(AsyncMux &&) = delete;
    AsyncMux & operator=(const AsyncMux &) = delete;
    AsyncMux & operator=(AsyncMux &&) = delete;

    void ChangeUpstream(Upstream &upstream)
    {
        this->node_.ChangeUpstream(upstream.node_);
        this->workerNode_.ChangeUpstream(upstream.workerNode_);
    }

public:
    Unfiltered GetWorkerControl()
    {
        return Unfiltered(this->workerNode_, this);
    }

    Unfiltered GetWxControl()
    {
        return Unfiltered(this->node_, this);
    }

    AsyncMux & operator=(pex::Argument<Type> value)
    {
        this->node_.Set(value);

        return *this;
    }

    void SetFilter(Filter filter)
    {
        this->node_.SetFilter(filter);
    }

    void Set(pex::Argument<Type> value)
    {
        this->SetWithoutNotify_(value);
        this->Notify();
    }

    Type Get() const
    {
        return this->node_.Get();
    }

    explicit operator Type () const
    {
        return this->node_.Get();
    }

    // The defaut control is for the wx event loop.
    operator Unfiltered ()
    {
        return Unfiltered(this->node_, this);
    }

    void Connect(void * observer, Callable callable)
    {
        this->node_.Connect(observer, callable);
    }

    void Disconnect(void * observer)
    {
        this->node_.Disconnect(observer);
    }

    void Notify()
    {
        pex::detail::AccessReference<Mux>(this->node_).Notify();
    }

    bool HasModel() const
    {
        return this->node_.HasModel() && this->workerNode_.HasModel();
    }

private:
    void SetWithoutNotify_(pex::Argument<Type> value)
    {
        pex::detail::AccessReference<Mux>(this->node_)
            .SetWithoutNotify(value);
    }

private:
    Mux node_;
    Mux workerNode_;
};


template<typename T, typename ModelFilter, typename Access>
struct AsyncTypes
{
    using Type = T;
    using Model = Async<T, ModelFilter, Access>;

    template
    <
        typename Upstream,
        typename ControlFilter,
        typename ControlAccess
    >
    using Control = AsyncControl<Upstream, ControlFilter, ControlAccess>;

    using Mux = AsyncMux<T, ModelFilter, Access>;

    template<typename ControlFilter, typename ControlAccess>
    using Follow =
        typename Mux::template Follow<ControlFilter, ControlAccess>;
};


template<typename T, typename Filter = pex::NoFilter>
using MakeAsync = pex::DefineNodes<AsyncTypes<T, Filter, pex::GetAndSetTag>>;


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

    class Control
        :
        public pex::control::Signal<ThreadSafe>
    {
    public:
        using Base = pex::control::Signal<ThreadSafe>;

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

        endpoint_(
            PEX_THIS("wxpex::AsyncSignal"),
            Control(this->model_, this)),

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


static_assert(pex::IsSignalControl<typename AsyncSignal::Control>);


template<typename Control>
class SetWait
{
public:
    using ValueType = typename Control::Type;
    using AsyncValue = Async<ValueType>;
    using WorkerControl = typename AsyncValue::Unfiltered;

    using Endpoint =
        pex::Endpoint<SetWait, WorkerControl>;

    SetWait(Control control)
        :
        mutex_(),
        condition_(),
        isWaiting_(false),
        control_(control),
        async_(control.Get()),

        endpoint_(
            PEX_THIS("SetWait"),
            this->async_.GetWxControl(),
            &SetWait::OnMainThread_),

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
    using Control = pex::control::DefaultSignal;
    using WorkerControl = typename AsyncSignal::Control;

    using Endpoint =
        pex::Endpoint<TriggerWait, WorkerControl>;

    static constexpr auto observerName = "TriggerWait";

    TriggerWait(const Control &control)
        :
        mutex_(),
        condition_(),
        isWaiting_(false),
        control_(control),
        asyncSignal_(),

        endpoint_(
            PEX_THIS("TriggerWait"),
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
