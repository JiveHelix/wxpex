#pragma once


#include <memory>

#include "wxpex/ignores.h"

WXSHIM_PUSH_IGNORES
#include <wx/sizer.h>
WXSHIM_POP_IGNORES


namespace wxpex
{


std::unique_ptr<wxSizer> IndentSizer(
    int pixels,
    std::unique_ptr<wxSizer> &&toIndent);


} // end namespace wxpex
