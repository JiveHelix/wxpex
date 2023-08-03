#pragma once


#include <tau/color.h>
#include <pex/pex.h>
#include <pex/endpoint.h>
#include "wxpex/slider.h"
#include "wxpex/labeled_widget.h"


namespace wxpex
{


inline
wxColour ToWxColour(const tau::Rgb<uint8_t> &color)
{
    return wxColour(color.red, color.green, color.blue);
}


template<typename T>
wxColour ToWxColour(const tau::Hsv<T> &color)
{
    auto rgb = tau::HsvToRgb<uint8_t>(color);
    return ToWxColour(rgb);
}


inline
wxColour ToWxColour(const tau::Rgba<uint8_t> &color)
{
    return wxColour(color.red, color.green, color.blue, color.alpha);
}


template<typename T>
wxColour ToWxColour(const tau::Hsva<T> &color)
{
    auto rgba = tau::HsvToRgb<uint8_t>(color);
    return ToWxColour(rgba);
}


inline
tau::Rgb<uint8_t> RgbFromWxColour(const wxColour &color)
{
    return {{color.Red(), color.Green(), color.Blue()}};
}


template<typename T>
tau::Hsv<T> HsvFromWxColour(const wxColour &color)
{
    auto rgb = RgbFromWxColour(color);
    return RgbToHsv<T>(rgb);
}


template<typename ColorControl>
class ColorPreview: public wxPanel
{
public:
    using Color = typename ColorControl::Type;

    ColorPreview(
        wxWindow *parent,
        ColorControl control,
        wxSize size = wxSize(65, 65))
        :
        wxPanel(parent, wxID_ANY, wxDefaultPosition, size),
        connect_(this, control, &ColorPreview::SetColor)
    {
        this->SetColor(control.Get());
    }

    void SetColor(const Color &color)
    {
        this->SetBackgroundColour(ToWxColour(color));
        this->Refresh();
    }

private:
    pex::MakeConnector<ColorPreview, ColorControl> connect_;
};


} // end namespace wxpex
