#include "wxpex/static_box.h"
#include "wxpex/size.h"

WXSHIM_PUSH_IGNORES
#include <wx/sizer.h>
WXSHIM_POP_IGNORES


namespace wxpex
{


StaticBox::StaticBox(wxWindow *parent, const std::string &label)
    :
    wxPanel(parent, wxID_ANY),
    staticBoxInternal_(new detail::StaticBoxInternal(this, wxID_ANY, label))
{
    this->SetSizer(new wxStaticBoxSizer(this->staticBoxInternal_, wxVERTICAL));
}


wxStaticBoxSizer * StaticBox::GetStaticBoxSizer()
{
    auto sizer = dynamic_cast<wxStaticBoxSizer *>(this->GetSizer());

    if (!sizer)
    {
        throw std::logic_error("Expected wxStaticBoxSizer");
    }

    return sizer;
}


wxStaticBox * StaticBox::GetPanel()
{
    return this->staticBoxInternal_;
}


void StaticBox::ConfigureSizer(std::unique_ptr<wxSizer> &&sizer)
{
    auto staticBoxSizer = this->GetStaticBoxSizer();

#if defined(__WXGTK__)

    // GTK fails to calculate enough space for the label.
    auto extraSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    extraSizer->Add(
        sizer.release(),
        1,
        wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT,
        25);

    staticBoxSizer->Add(extraSizer.release(), 1, wxEXPAND);

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

    staticBoxSizer->Add(extraTop.release());
#else
    staticBoxSizer->Add(sizer.release(), 1, wxEXPAND);
#endif

    staticBoxSizer->SetSizeHints(this);
}


} // end namespace wxpex
