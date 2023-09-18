#pragma once

#include <memory>

#include "wxpex/ignores.h"

WXSHIM_PUSH_IGNORES
#include <wx/collpane.h>
WXSHIM_POP_IGNORES

#include "wxpex/size.h"
#include "wxpex/async.h"


namespace wxpex
{


class Collapsible: public wxCollapsiblePane
{
public:
    Collapsible(wxWindow *parent, const std::string &label);

    void ConfigureTopSizer(std::unique_ptr<wxSizer> &&sizer);

    void ConfigureBorderPane(
        std::unique_ptr<wxSizer> &&sizer,
        int pixels);

#if defined(__WXGTK__)
    // WXGTK uses DoGetBestSize, and WXMAC/WXMSW ignore it.
    wxSize DoGetBestSize() const override;
#else
    wxSize DoGetBestClientSize() const override;
#endif

    wxPanel * GetBorderPane(long borderStyle = wxBORDER_SIMPLE);


protected:
    void OnChanged_(wxCollapsiblePaneEvent &);

    void UpdateMinimumSize_() const;

    void ReportWindowSize_(wxWindow *window, size_t depth);
    void FixWindowSize_(wxWindow *window);

private:
    wxPanel *borderPane_;
    std::string label_;
    wxpex::CallAfter doLayoutTopLevel_;
};


} // end namespace wxpex
