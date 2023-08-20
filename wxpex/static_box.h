#pragma once

#include <memory>

#include "wxpex/ignores.h"

WXSHIM_PUSH_IGNORES
#include <wx/statbox.h>
WXSHIM_POP_IGNORES


namespace wxpex
{


class StaticBox: public wxStaticBox
{
public:
    StaticBox(wxWindow *parent, const std::string &label);
    void ConfigureSizer(std::unique_ptr<wxSizer> &&sizer);
};


} // end namespace wxpex
