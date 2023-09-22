#pragma once


#include "wxpex/ignores.h"

WXSHIM_PUSH_IGNORES
#include <wx/window.h>
WXSHIM_POP_IGNORES


namespace wxpex
{

class Freezer
{
public:
    Freezer(wxWindow *window)
        :
        top_(wxGetTopLevelParent(window))
    {
        if (this->top_)
        {
            this->top_->Freeze();
        }
    }

    ~Freezer()
    {
        if (this->top_)
        {
            this->top_->Thaw();
        }
    }

    Freezer(const Freezer &) = delete;
    Freezer & operator=(const Freezer &) = delete;

    Freezer(Freezer &&other)
        :
        top_(other.top_)
    {
        other.top_ = nullptr;
    }

private:
    wxWindow *top_;
};


} // end namespace wxpex
