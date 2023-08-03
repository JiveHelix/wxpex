#pragma once


#include <memory>
#include <wx/sizer.h>


namespace wxpex
{


std::unique_ptr<wxSizer> IndentSizer(
    int pixels,
    std::unique_ptr<wxSizer> &&toIndent);


} // end namespace wxpex
