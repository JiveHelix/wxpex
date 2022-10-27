#pragma once


#include <tau/color.h>
#include <pex/pex.h>
#include "wxpex/slider.h"
#include "wxpex/labeled_widget.h"


namespace wxpex
{


using HsvGroup = tau::HsvGroup<float>;

using Hsv = typename HsvGroup::Plain;
using HsvModel = typename HsvGroup::Model;
using HsvControl = typename HsvGroup::Control<void>;

template<typename Observer>
using HsvTerminus = typename HsvGroup::Terminus<Observer>;


using RgbGroup = tau::RgbGroup<uint8_t>;

using Rgb = typename RgbGroup::Plain;
using RgbModel = typename RgbGroup::Model;
using RgbControl = typename RgbGroup::Control<void>;

template<typename Observer>
using RgbTerminus = typename RgbGroup::Terminus<Observer>;


class ColorPreview: public wxPanel
{
public:
    ColorPreview(
        wxWindow *parent,
        const tau::Rgb<uint8_t> &color,
        wxSize size = wxSize(65, 65))
        :
        wxPanel(parent, wxID_ANY, wxDefaultPosition, size)
    {
        this->SetColor(color);
    }

    void SetColor(const tau::Rgb<uint8_t> &color)
    {
        this->SetBackgroundColour(wxColour(color.red, color.green, color.blue));
        this->Refresh();
    }
};


using HueRange =
    pex::control::LinearRange<void, decltype(HsvControl::hue), 2>;

using SaturationRange =
    pex::control::LinearRange<void, decltype(HsvControl::saturation), 100>;

using ValueRange =
    pex::control::LinearRange<void, decltype(HsvControl::value), 100>;


class HsvPicker: public wxStaticBox
{
public:

    HsvPicker(
        wxWindow *parent,
        HsvControl control)
        :
        wxStaticBox(parent, wxID_ANY, "HSV"),
        terminus_(this, control)
    {
        this->terminus_.Connect(&HsvPicker::OnColorChanged_);

        using HueSlider =
            SliderAndValue<HueRange, decltype(control.hue.value), 5>;

        auto hue = wxpex::LabeledWidget(
            this,
            "Hue",
            new HueSlider(
                this,
                HueRange(control.hue),
                control.hue.value));

        using SaturationSlider =
            SliderAndValue
            <
                SaturationRange,
                decltype(control.saturation.value),
                4
            >;

        auto saturation = wxpex::LabeledWidget(
            this,
            "Saturation",
            new SaturationSlider(
                this,
                SaturationRange(control.saturation),
                control.saturation.value));

        using ValueSlider =
            SliderAndValue<ValueRange, decltype(control.value.value), 4>;

        auto value = wxpex::LabeledWidget(
            this,
            "Value",
            new ValueSlider(
                this,
                ValueRange(control.value),
                control.value.value));

        this->colorPreview_ = new ColorPreview(
            this,
            tau::HsvToRgb<uint8_t>(control.Get()));

        auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);

        auto sliderLayout =
            LayoutLabeled(LayoutOptions{}, hue, saturation, value);

        sizer->Add(sliderLayout.release(), 1, wxRight, 5);

        auto vertical = std::make_unique<wxBoxSizer>(wxVERTICAL);

        vertical->Add(
            this->colorPreview_,
            0,
            wxALIGN_CENTER | wxALL,
            10);

        sizer->Add(vertical.release(), 0, wxEXPAND);

        this->Bind(wxpex::SliderDone, &HsvPicker::OnSliderDone_, this);

        this->SetSizerAndFit(sizer.release());
    }

    void OnSliderDone_(wxCommandEvent &)
    {
        this->Layout();
    }

private:
    void OnColorChanged_(const Hsv &hsv)
    {
        this->colorPreview_->SetColor(tau::HsvToRgb<uint8_t>(hsv));
    }

private:
    HsvTerminus<HsvPicker> terminus_;
    ColorPreview * colorPreview_;
};


using RedRange =
    pex::control::ConvertingRange<void, decltype(RgbControl::red), int>;

using GreenRange =
    pex::control::ConvertingRange<void, decltype(RgbControl::green), int>;

using BlueRange =
    pex::control::ConvertingRange<void, decltype(RgbControl::blue), int>;


class RgbPicker: public wxStaticBox
{
public:

    RgbPicker(
        wxWindow *parent,
        RgbControl control)
        :
        wxStaticBox(parent, wxID_ANY, "RGB"),
        terminus_(this, control)
    {
        this->terminus_.Connect(&RgbPicker::OnColorChanged_);

        using RedSlider =
            SliderAndValue<RedRange, decltype(control.red.value), 3>;

        auto red = wxpex::LabeledWidget(
            this,
            "Red",
            new RedSlider(
                this,
                RedRange(control.red),
                control.red.value));

        using GreenSlider =
            SliderAndValue<GreenRange, decltype(control.green.value), 3>;

        auto green = wxpex::LabeledWidget(
            this,
            "Green",
            new GreenSlider(
                this,
                GreenRange(control.green),
                control.green.value));

        using BlueSlider =
            SliderAndValue<BlueRange, decltype(control.blue.value), 3>;

        auto blue = wxpex::LabeledWidget(
            this,
            "Blue",
            new BlueSlider(
                this,
                BlueRange(control.blue),
                control.blue.value));

        this->colorPreview_ = new ColorPreview(this, control.Get());

        auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);

        auto sliderLayout =
            LayoutLabeled(LayoutOptions{}, red, green, blue);

        sizer->Add(sliderLayout.release(), 1, wxRight, 5);

        auto vertical = std::make_unique<wxBoxSizer>(wxVERTICAL);

        vertical->Add(
            this->colorPreview_,
            0,
            wxALIGN_CENTER | wxALL,
            10);

        sizer->Add(vertical.release(), 0, wxEXPAND);

        this->Bind(wxpex::SliderDone, &RgbPicker::OnSliderDone_, this);

        this->SetSizerAndFit(sizer.release());
    }

    void OnSliderDone_(wxCommandEvent &)
    {
        this->Layout();
    }

private:
    void OnColorChanged_(const Rgb &rgb)
    {
        this->colorPreview_->SetColor(rgb);
    }

private:
    RgbTerminus<RgbPicker> terminus_;
    ColorPreview * colorPreview_;
};


} // end namespace wxpex
