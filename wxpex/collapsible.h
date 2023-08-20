#pragma once

#include <memory>

#include "wxpex/ignores.h"

WXSHIM_PUSH_IGNORES
#include <wx/collpane.h>
WXSHIM_POP_IGNORES

#include "wxpex/size.h"


namespace wxpex
{


class Collapsible: public wxCollapsiblePane
{
public:
    Collapsible(wxWindow *parent, const std::string &label);

    void ConfigureTopSizer(std::unique_ptr<wxSizer> &&sizer);

    void ConfigureBorderPane(
        int pixels,
        std::unique_ptr<wxSizer> &&sizer);

#ifdef __WXGTK__
    // WXGTK uses DoGetBestSize, and WXMAC ignores it.
    // TODO: See what MSW does.
    // TODO: Submit a PR to make all three consistent.
    wxSize DoGetBestSize() const override;
#else
    wxSize DoGetBestClientSize() const override;
#endif

    wxPanel * GetBorderPane(long borderStyle = wxBORDER_SIMPLE);

protected:
    void OnChanged_(wxCollapsiblePaneEvent &);

    void UpdateMinimumSize_() const;

private:
    wxPanel *borderPane_;
    std::string label_;
};


} // end namespace wxpex
