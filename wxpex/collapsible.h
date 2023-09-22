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
    Collapsible(
        wxWindow *parent,
        const std::string &label,
        long borderStyle = wxBORDER_NONE);

    void ConfigureTopSizer(std::unique_ptr<wxSizer> &&sizer);

#if defined(__WXGTK__)
    // WXGTK uses DoGetBestSize, and WXMAC/WXMSW ignore it.
    wxSize DoGetBestSize() const override;
#else
    wxSize DoGetBestClientSize() const override;
#endif

    wxWindow * GetPanel();


protected:
    void ConfigureBorderPane_(
        std::unique_ptr<wxSizer> &&sizer,
        int pixels);

    void OnChanged_(wxCollapsiblePaneEvent &);

    void UpdateMinimumSize_() const;

    void ReportWindowSize_(wxWindow *window, size_t depth);
    void FixCollapsibleSize_(wxWindow *window);
    void FixContainerSize_(wxWindow *window);

private:
    wxPanel *borderPane_;
    std::string label_;
};


} // end namespace wxpex
