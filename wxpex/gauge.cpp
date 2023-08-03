#include "wxpex/gauge.h"
#include "wxpex/view.h"
#include "wxpex/converter.h"


namespace wxpex
{


Gauge::Gauge(
    wxWindow *parent,
    GaugeControl control,
    Style style)
    :
    Base(
        parent,
        wxID_ANY,
        control.maximum.Get(),
        wxDefaultPosition,
        wxDefaultSize,
        GaugeStyle(style)),
    endpoints_(this, control)
{
    this->endpoints_.value.Connect(&Gauge::OnValue_);
    this->endpoints_.maximum.Connect(&Gauge::OnMaximum_);
    this->SetValue(control.value.Get());
}


void Gauge::OnValue_(size_t value)
{
    size_t maximum = this->endpoints_.control.maximum.Get();

    if (maximum == 0)
    {
        // Then the maximum is 0, use the gauge in indeterminate mode.
        this->Pulse();
    }
    else
    {
        this->SetValue(value);
    }
}


void Gauge::OnMaximum_(size_t maximum)
{
    if (maximum != 0)
    {
        this->SetRange(maximum);
    }
}


ValueGauge::ValueGauge(
    wxWindow *parent,
    GaugeControl control,
    Style style)
    :
    wxControl(parent, wxID_ANY),
    value_(this, control.value)
{
    using ValueControl = decltype(control.value);
    using IntConverter = Converter<ValueControl, -1, -1>;
    auto gauge = new Gauge(this, control, style);
    auto view = new View<ValueControl, IntConverter>(this, control.value);

    // Use a mono-spaced font for display so that the width of the view
    // remains constant as the value changes.
    view->SetFont(wxFont(wxFontInfo().Family(wxFONTFAMILY_MODERN)));

    auto sizerStyle = SizerStyle(style);

    auto sizer = std::make_unique<wxBoxSizer>(sizerStyle);

    auto flag = IsHorizontal(style)
        ? wxRIGHT | wxALIGN_CENTER_VERTICAL
        : wxBOTTOM | wxALIGN_CENTER;

    auto spacing = 5;
    sizer->Add(gauge, 1, flag, spacing);

    sizer->Add(
        view,
        0,
        IsHorizontal(style)
            ? wxALIGN_CENTER_VERTICAL
            : wxALIGN_CENTER);

    this->SetSizerAndFit(sizer.release());
    this->value_.Connect(&ValueGauge::OnValue_);
}


void ValueGauge::OnValue_(size_t)
{
    this->Layout();
}


} // end namespace wxpex
