#include "wxpex/static_box.h"
#include <wx/sizer.h>


namespace wxpex
{


StaticBox::StaticBox(wxWindow *parent, const std::string &label)
    :
    wxStaticBox(parent, wxID_ANY, label)
{

}


void StaticBox::ConfigureSizer(std::unique_ptr<wxSizer> &&sizer)
{
#if defined(__WXGTK__)

    // GTK fails to calculate enough space for the label.
    auto extraSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    extraSizer->Add(
        sizer.release(),
        1,
        wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT,
        25);

    this->SetSizerAndFit(extraSizer.release());

#elif defined (__WXMSW__)

    // MSW fails (differently) to calculate enough space for the label.
    auto extraLeftBottom = std::make_unique<wxBoxSizer>(wxVERTICAL);
    auto extraTop = std::make_unique<wxBoxSizer>(wxVERTICAL);

    extraLeftBottom->Add(
        sizer.release(),
        1,
        wxEXPAND | wxLEFT | wxBOTTOM,
        10);

    extraTop->Add(
        extraLeftBottom.release(),
        1,
        wxEXPAND | wxTOP,
        15);

    this->SetSizerAndFit(extraTop.release());

#else
    this->SetSizerAndFit(sizer.release());
#endif
}


} // end namespace wxpex
