#pragma once


#include <memory>

#include "wxpex/ignores.h"

WXSHIM_PUSH_IGNORES
#include <wx/sizer.h>
WXSHIM_POP_IGNORES


namespace wxpex
{


std::unique_ptr<wxSizer> BorderSizer(
    std::unique_ptr<wxSizer> &&toWrap,
    int pixels);


std::unique_ptr<wxSizer> BorderSizer(
    wxWindow *window,
    int pixels);


} // end namespace wxpex
