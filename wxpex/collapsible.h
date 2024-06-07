#pragma once

#include <memory>

#include <pex/model_value.h>
#include <pex/control_value.h>
#include <pex/endpoint.h>
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
    using StateModel = pex::model::Value<bool>;
    using StateControl = pex::control::Value<StateModel>;

    Collapsible(
        wxWindow *parent,
        const std::string &label,
        StateControl stateControl,
        long borderStyle = wxBORDER_NONE);

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
    void OnState_(bool isExpanded);

    void HandleStateChange_();

    wxPanel *borderPane_;
    std::string label_;

    using StateEndpoint = pex::Endpoint<Collapsible, StateControl>;
    StateEndpoint stateEndpoint_;
    bool hasStateEndpoint_;
    bool ignoreState_;
};


} // end namespace wxpex
