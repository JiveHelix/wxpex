#include "wxpex/scrolled.h"


namespace wxpex
{


Scrolled::Scrolled(wxWindow *parent)
    :
    wxScrolled<wxPanel>(parent, wxID_ANY)
{

}


void Scrolled::ConfigureSizer(
    const ScrolledSettings &scrolledSettings,
    std::unique_ptr<wxSizer> &&sizer)
{
    auto borderSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    borderSizer->Add(
        sizer.get(),
        1,
        wxEXPAND | wxALL,
        scrolledSettings.borderPixels);

    sizer.release();
    auto topSizer = borderSizer.get();
    this->SetSizer(borderSizer.release());
    topSizer->FitInside(this);

    this->SetScrollRate(
        scrolledSettings.horizontalRate,
        scrolledSettings.verticalRate);

    this->SetMinSize(
        wxSize(
            scrolledSettings.minimumWidth,
            scrolledSettings.minimumHeight));
}


} // end namespace wxpex
