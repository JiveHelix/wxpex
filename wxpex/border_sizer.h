#pragma once


#include <memory>
#include <wx/sizer.h>


namespace wxpex
{


std::unique_ptr<wxSizer> BorderSizer(
    std::unique_ptr<wxSizer> &&toWrap,
    int pixels);


std::unique_ptr<wxSizer> BorderSizer(
    wxWindow *window,
    int pixels);


} // end namespace wxpex
