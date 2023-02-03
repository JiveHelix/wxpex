#pragma once

#include "wxpex/wxshim.h"
#include <string>


namespace wxpex
{


class Window
{
public:
    Window(): window_(nullptr) {}

    Window(wxWindow *window)
        :
        window_(window)
    {
        this->BindCloseHandler_();
    }

    Window(const Window &other) = delete;

    Window & operator=(const Window &other) = delete;

    Window(Window &&other)
        :
        window_(other.window_)
    {
        other.UnbindCloseHandler_();
        this->BindCloseHandler_();
        other.window_ = nullptr;
    }

    Window & operator=(Window &&other)
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

    ~Window()
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

    wxWindow * Get()
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
            this->window_->Bind(wxEVT_CLOSE_WINDOW, &Window::OnClose_, this);
        }
    }

    void UnbindCloseHandler_()
    {
        if (this->window_ && !this->window_->IsBeingDeleted())
        {
            this->window_->Unbind(wxEVT_CLOSE_WINDOW, &Window::OnClose_, this);
        }
    }

private:
    wxWindow *window_;
};


} // end namespace wxpex
