#include "wxpex/gauge.h"


namespace wxpex
{


Gauge::Gauge(
    wxWindow *parent,
    GaugeControl control,
    long style)
    :
    Base(
        parent,
        wxID_ANY,
        control.maximum.Get(),
        wxDefaultPosition,
        wxDefaultSize,
        style),
    terminus_(this, control)
{
    this->terminus_.value.Connect(&Gauge::OnValue_);
    this->terminus_.maximum.Connect(&Gauge::OnMaximum_);
    this->SetValue(control.value.Get());
}


void Gauge::OnValue_(size_t value)
{
    size_t maximum = this->terminus_.maximum.Get();

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


} // end namespace wxpex
