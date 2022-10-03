#include "wxpex/collapsible.h"
#include "wxpex/layout_top_level.h"


namespace wxpex
{


Collapsible::Collapsible(wxWindow *parent, const std::string &label)
    :
    wxCollapsiblePane(
        parent,
        wxID_ANY,
        label,
        wxDefaultPosition,
        wxDefaultSize,
        wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE)
{
    this->Bind(
        wxEVT_COLLAPSIBLEPANE_CHANGED,
        &Collapsible::OnChanged_,
        this); 
}


void Collapsible::ConfigureTopSizer(wxSizer *sizer)
{
    this->GetPane()->SetSizer(sizer);
    sizer->SetSizeHints(this->GetPane());
}


void Collapsible::OnChanged_(wxCollapsiblePaneEvent &)
{
    wxpex::LayoutTopLevel(this);
}


} // end namespace wxpex
