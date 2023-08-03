#pragma once


#include <memory>
#include <wx/sizer.h>


namespace wxpex
{


std::unique_ptr<wxSizer> BorderSizer(
    int pixels,
    std::unique_ptr<wxSizer> &&toWrap);


} // end namespace wxpex
