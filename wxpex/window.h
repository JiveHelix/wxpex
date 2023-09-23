#pragma once

#include "wxpex/wxshim.h"
#include <string>


namespace wxpex
{



template<typename T>
class Unclosed
{
public:
    Unclosed(): window_(nullptr) {}

    Unclosed(T *window)
        :
        window_(window)
    {
        this->BindCloseHandler_();
    }

    Unclosed(const Unclosed &other)
        :
        window_(other.window_)
    {
        this->BindCloseHandler_();
    }

    Unclosed & operator=(const Unclosed &other)
    {
        if (!other.window_)
        {
            return *this;
        }

        if (other.window_->IsBeingDeleted())
        {
            return *this;
        }

        this->window_ = other.window_;
        this->BindCloseHandler_();

        return *this;
    }

    void Close()
    {
        if (this->window_ && !this->window_->IsBeingDeleted())
        {
            this->UnbindCloseHandler_();
            this->window_->Close(true);
        }

        this->window_ = nullptr;
    }

    T * Get()
    {
        return this->window_;
    }

    void Clear()
    {
        this->UnbindCloseHandler_();
        this->window_ = nullptr;
    }

    operator bool () const
    {
        return (this->window_ != nullptr);
    }

    template<typename U>
    Unclosed<U> Copy() const
    {
        return Unclosed<U>(this->window_);
    }

private:
    void OnClose_(wxCloseEvent &event)
    {
        this->window_ = nullptr;
        event.Skip();
    }

    void BindCloseHandler_()
    {
        assert(this->window_ != nullptr);

        if (this->window_ && !this->window_->IsBeingDeleted())
        {
            this->window_->Bind(
                wxEVT_CLOSE_WINDOW,
                &Unclosed::OnClose_,
                this);
        }
    }

    void UnbindCloseHandler_()
    {
        if (this->window_ && !this->window_->IsBeingDeleted())
        {
            this->window_->Unbind(
                wxEVT_CLOSE_WINDOW,
                &Unclosed::OnClose_,
                this);
        }
    }

private:
    T *window_;
};


using UnclosedWindow = Unclosed<wxWindow>;
using UnclosedFrame = Unclosed<wxFrame>;


template<typename T>
class Closer
{
public:
    Closer(): window_(nullptr) {}

    Closer(T *window)
        :
        window_(window)
    {
        this->BindCloseHandler_();
    }

    Closer(const Closer &other) = delete;

    Closer & operator=(const Closer &other) = delete;

    Closer(Closer &&other)
        :
        window_(other.window_)
    {
        other.UnbindCloseHandler_();
        this->BindCloseHandler_();
        other.window_ = nullptr;
    }

    Closer & operator=(Closer &&other)
    {
        if (!other.window_)
        {
            return *this;
        }

        if (other.window_->IsBeingDeleted())
        {
            return *this;
        }

        this->Close();

        other.UnbindCloseHandler_();

        this->window_ = other.window_;
        this->BindCloseHandler_();

        other.window_ = nullptr;

        return *this;
    }

    ~Closer()
    {
        this->Close();
    }

    void Close()
    {
        if (this->window_ && !this->window_->IsBeingDeleted())
        {
            this->UnbindCloseHandler_();
            this->window_->Close(true);
        }

        this->window_ = nullptr;
    }

    T * Get()
    {
        return this->window_;
    }

    void Clear()
    {
        this->UnbindCloseHandler_();
        this->window_ = nullptr;
    }

    operator bool () const
    {
        return (this->window_ != nullptr);
    }

private:
    void OnClose_(wxCloseEvent &event)
    {
        this->window_ = nullptr;
        event.Skip();
    }

    void BindCloseHandler_()
    {
        assert(this->window_ != nullptr);

        if (this->window_ && !this->window_->IsBeingDeleted())
        {
            this->window_->Bind(wxEVT_CLOSE_WINDOW, &Closer::OnClose_, this);
        }
    }

    void UnbindCloseHandler_()
    {
        if (this->window_ && !this->window_->IsBeingDeleted())
        {
            this->window_->Unbind(wxEVT_CLOSE_WINDOW, &Closer::OnClose_, this);
        }
    }

private:
    T *window_;
};


using Window = Closer<wxWindow>;
using Frame = Closer<wxFrame>;


} // end namespace wxpex
