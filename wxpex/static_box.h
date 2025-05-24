#pragma once

#include <memory>

#include "wxpex/ignores.h"

WXSHIM_PUSH_IGNORES
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/collpane.h>
WXSHIM_POP_IGNORES


namespace wxpex
{


namespace detail
{


class StaticBoxInternal: public wxStaticBox
{
public:
    using wxStaticBox::wxStaticBox;
};


} // end namespace detail

class StaticBox: public wxPanel
{
public:
    StaticBox(wxWindow *parent, const std::string &label);
    wxStaticBox * GetPanel();
    wxStaticBoxSizer * GetStaticBoxSizer();
    void ConfigureSizer(std::unique_ptr<wxSizer> &&sizer);

private:
    void OnCollapsibleChild_(wxCollapsiblePaneEvent &event);

    detail::StaticBoxInternal *staticBoxInternal_;

};


} // end namespace wxpex
