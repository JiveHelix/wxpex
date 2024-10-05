#include "wxpex/knob.h"


namespace wxpex
{


std::unique_ptr<wxBoxSizer> MakeSizer(
    const KnobSettings &knobSettings,
    wxWindow *knob,
    wxWindow *view)
{
    auto sizerStyle = SizerStyle(knobSettings.layout);

    auto sizer = std::make_unique<wxBoxSizer>(sizerStyle);

    auto flag = IsHorizontal(knobSettings.layout)
        ? wxRIGHT | wxEXPAND
        : wxBOTTOM | wxEXPAND;

    sizer->Add(knob, 1, flag, knobSettings.spacing);

    sizer->Add(
        view,
        0,
        IsHorizontal(knobSettings.layout)
            ? wxALIGN_CENTER_VERTICAL
            : wxALIGN_CENTER);

    return sizer;
}


} // end namespace wxpex
