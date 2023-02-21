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

#include <pex/terminus.h>
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
    static constexpr auto observerName = "wxpex::Async";

    using Type = T;
    using ThreadSafe = pex::model::Value_<Type, Filter>;
    using Callable = typename ThreadSafe::Callable;

    template<typename Observer>
    using Control = pex::control::Value<Observer, ThreadSafe>;

    template<typename Observer>
    using Terminus = pex::Terminus<Observer, Control<Observer>>;

    /* TODO: Using ThreadSafe as the upstream for Terminus should work, and it
     * does within this class.
     * But when it is used outside of this class, pex machinery tries to create
     * a pex::model::Value_<Observer, Filter>!!!
     */

    template<typename>
    friend class pex::Reference;

    Async(pex::Argument<Type> value = Type{})
        :
        mutex_(),
        ignoreEcho_(false),
        value_(value),
        model_(value),
        terminus_(this, this->model_),
        workerModel_(value),
        workerTerminus_(this, this->workerModel_)
    {
        this->Initialize_();
    }

    Async(pex::Argument<Type> value, Filter filter)
        :
        mutex_(),
        ignoreEcho_(false),
        value_(value),
        model_(value, filter),
        terminus_(this, this->model_),
        workerModel_(value, filter),
        workerTerminus_(this, this->workerModel_)
    {
        this->Initialize_();
    }

    Async(Filter filter)
        :
        mutex_(),
        ignoreEcho_(false),
        value_{},
        model_(this->value_, filter),
        terminus_(this, this->model_),
        workerModel_(this->value_, filter),
        workerTerminus_(this, this->workerModel_)
    {
        this->Initialize_();
    }

    Async(const Async<Type, Filter> &) = delete;
    Async(Async<Type, Filter> &&) = delete;

protected:
    void Initialize_()
    {
        this->Bind(wxEVT_THREAD, &Async::OnWxEventLoop_, this);
        this->terminus_.Connect(&Async::OnWxChanged_);
        this->workerTerminus_.Connect(&Async::OnWorkerChanged_);
    }

public:
    Control<void> GetWorkerControl()
    {
        return Control<void>(this->workerModel_);
    }

    Control<void> GetWxControl()
    {
        return Control<void>(this->model_);
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
        std::lock_guard<std::mutex> lock(this->mutex_);
        return this->value_;
    }

    explicit operator Type () const
    {
        std::lock_guard<std::mutex> lock(this->mutex_);
        return this->value_;
    }

    // The defaut control is for the wx event loop.
    template<typename Observer>
    explicit operator Control<Observer> ()
    {
        return Control<Observer>(this->model_);
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
        if (this->ignoreEcho_)
        {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            this->value_ = value;
        }

        // Queue the event for the wxWidgets event loop.
        this->QueueEvent(new wxThreadEvent());
    }

    void OnWxEventLoop_(wxThreadEvent &)
    {
        Type value;

        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            value = this->value_;
        }

        this->ignoreEcho_ = true;
        this->model_.Set(value);
        this->ignoreEcho_ = false;
    }

    void OnWxChanged_(pex::Argument<Type> value)
    {
        if (this->ignoreEcho_)
        {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            this->value_ = value;
        }

        this->ignoreEcho_ = true;
        this->workerModel_.Set(value);
        this->ignoreEcho_ = false;
    }


    void SetWithoutNotify_(pex::Argument<Type> value)
    {
        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            this->value_ = value;
        }

        pex::ReferenceSetter<ThreadSafe>(this->model_).SetWithoutNotify(value);
    }

    void DoNotify_()
    {
        // This will trigger OnWorkerChanged_, which will notify model_ in
        // the wx event loop.
        pex::ReferenceSetter<ThreadSafe>(this->model_).DoNotify();
    }

private:
    mutable std::mutex mutex_;
    bool ignoreEcho_;
    Type value_;
    ThreadSafe model_;
    Terminus<Async> terminus_;
    ThreadSafe workerModel_;
    Terminus<Async> workerTerminus_;
};


template<typename T, typename Filter = pex::NoFilter>
using MakeAsync = pex::MakeCustom<Async<T, Filter>>;


// Async can be used as the Value of a pex::model::Range, but the Range class
// uses a private value. This class provides access to the functions unique to
// Async.
template
<
    typename T,
    typename Minimum,
    typename Maximum,
    template<typename, typename> typename Value_
>
class AsyncRangeAccess
    :
    public pex::model::RangeAccess<T, Minimum, Maximum, Value_>
{
public:
    using Base = pex::model::RangeAccess<T, Minimum, Maximum, Value_>;
    using Value = typename Base::Value;
    using Control = typename Value::template Control<void>;

    AsyncRangeAccess(pex::model::Range<T, Minimum, Maximum, Value_> &range)
        :
        Base(range)
    {

    }

    Control GetWorkerControl()
    {
        return this->GetValue().GetWorkerControl();
    }

    Control GetWxControl()
    {
        return this->GetValue().GetWxControl();
    }
};


namespace experimental
{
// Don't rely on this being availabe in future versions of this library.

template<typename Pex>
class AddAsync: public wxEvtHandler
{
public:
    using UpstreamHolder = pex::UpstreamHolderT<Pex>;
    using UpstreamTerminus = pex::Terminus<AddAsync, UpstreamHolder>;
    using Type = typename UpstreamHolder::Type;
    using ThreadSafe = pex::model::Value<Type>;
    using WorkerTerminus = pex::Terminus<AddAsync, ThreadSafe>;
    using Callable = typename UpstreamHolder::Callable;

    template<typename>
    friend class pex::Reference;

    template<typename Observer>
    using UserControl = pex::control::Value<Observer, UpstreamHolder>;

    template<typename Observer>
    using Control = UserControl<Observer>;

    template<typename Observer>
    using WorkerControl = pex::control::Value<Observer, ThreadSafe>;

    AddAsync(pex::PexArgument<Pex> pex)
        :
        mutex_(),
        upstream_(pex),
        upstreamTerminus_(this, this->upstream_),
        ignoreEcho_(false),
        workerModel_(),
        workerTerminus_(this, this->workerModel_),
        value_(this->upstream_.Get())
    {
        this->Bind(wxEVT_THREAD, &AddAsync::OnWxEventLoop_, this);
        this->upstreamTerminus_.Connect(&AddAsync::OnUpstreamChanged_);
        this->workerTerminus_.Connect(&AddAsync::OnWorkerChanged_);
    }

    WorkerControl<void> GetWorkerControl()
    {
        return WorkerControl<void>(this->workerModel_);
    }

    UserControl<void> GetWxControl()
    {
        return UserControl<void>(this->upstream_);
    }

    Type Get() const
    {
        std::lock_guard<std::mutex> lock(this->mutex_);
        return this->value_;
    }

    explicit operator Type () const
    {
        std::lock_guard<std::mutex> lock(this->mutex_);
        return this->value_;
    }

    // The defaut control is for the wx event loop.
    template<typename Observer>
    explicit operator UserControl<Observer> ()
    {
        return UserControl<Observer>(this->upstream_);
    }

    void Connect(void * observer, Callable callable)
    {
        this->upstream_.Connect(observer, callable);
    }

    void Disconnect(void * observer)
    {
        this->upstream_.Disconnect(observer);
    }

private:
    void OnWorkerChanged_(pex::Argument<Type> value)
    {
        if (this->ignoreEcho_)
        {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            this->value_ = value;
        }

        // Queue the event for the wxWidgets event loop.
        this->QueueEvent(new wxThreadEvent());
    }

    void OnWxEventLoop_(wxThreadEvent &)
    {
        Type value;

        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            value = this->value_;
        }

        this->ignoreEcho_ = true;
        this->upstream_.Set(value);
        this->ignoreEcho_ = false;
    }

    void OnUpstreamChanged_(pex::Argument<Type> value)
    {
        if (this->ignoreEcho_)
        {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            this->value_ = value;
        }

        this->ignoreEcho_ = true;
        this->workerModel_.Set(value);
        this->ignoreEcho_ = false;
    }


    void SetWithoutNotify_(pex::Argument<Type> value)
    {
        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            this->value_ = value;
        }

        pex::ReferenceSetter<ThreadSafe>(this->upstream_).
            SetWithoutNotify(value);
    }

    void DoNotify_()
    {
        // This will trigger OnWorkerChanged_, which will notify upstream_ in
        // the wx event loop.
        pex::ReferenceSetter<ThreadSafe>(this->upstream_).DoNotify();
    }

private:
    mutable std::mutex mutex_;
    UpstreamHolder upstream_;
    UpstreamTerminus upstreamTerminus_;
    std::atomic_bool ignoreEcho_;
    ThreadSafe workerModel_;
    WorkerTerminus workerTerminus_;
    Type value_;
};


} // end namespace experimental


} // end namespace wxpex
