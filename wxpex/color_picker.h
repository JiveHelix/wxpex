#pragma once

#include <pex/endpoint.h>
#include "wxpex/static_box.h"
#include "wxpex/color.h"
#include "wxpex/slider.h"
#include "wxpex/knob.h"


namespace wxpex
{


using HsvGroup = tau::HsvGroup<double>;
using Hsv = typename HsvGroup::Plain;
using HsvModel = typename HsvGroup::Model;
using HsvControl = typename HsvGroup::Control;

using HsvaGroup = tau::HsvaGroup<double>;
using Hsva = typename HsvaGroup::Plain;
using HsvaModel = typename HsvaGroup::Model;
using HsvaControl = typename HsvaGroup::Control;

using RgbGroup = tau::RgbGroup<uint8_t>;
using Rgb = typename RgbGroup::Plain;
using RgbModel = typename RgbGroup::Model;
using RgbControl = typename RgbGroup::Control;

using RgbaGroup = tau::RgbaGroup<uint8_t>;
using Rgba = typename RgbaGroup::Plain;
using RgbaModel = typename RgbaGroup::Model;
using RgbaControl = typename RgbaGroup::Control;


template<typename Control, typename Derived>
class HsvKnobs_
{
public:
    using HueRange =
        pex::control::LinearRange<decltype(Control::hue), 10>;

    using SaturationRange =
        pex::control::LinearRange<decltype(Control::saturation), 100>;

    using ValueRange =
        pex::control::LinearRange<decltype(Control::value), 100>;

    using HueControl = decltype(Control::hue);
    using SaturationControl = decltype(Control::saturation);
    using ValueControl = decltype(Control::value);

    using HueKnob =
        wxpex::ValueKnob<HueRange, typename HueControl::Value, 5, 1>;

    using SaturationKnob =
        wxpex::ValueKnob
        <
            SaturationRange,
            typename SaturationControl::Value,
            4,
            2
        >;

    using ValueKnob =
        ::wxpex::ValueKnob<ValueRange, typename ValueControl::Value, 4, 2>;

    using Color = typename Control::Type;

public:
    HsvKnobs_(
        wxWindow *parent,
        Control control)
        :
        hue(
            new HueKnob(
                parent,
                HueRange(control.hue),
                control.hue.value,
                KnobSettings().StartAngle(0).Continuous(true))),
        saturation(
            new SaturationKnob(
                parent,
                SaturationRange(control.saturation),
                control.saturation.value)),
        value(
            new ValueKnob(
                parent,
                ValueRange(control.value),
                control.value.value)),
        parent_(parent),
        connect_(this, control, &HsvKnobs_::OnColorChanged_)
    {
        this->hue->GetKnob().SetColor(control.Get());
    }

    std::unique_ptr<wxSizer> MakeKnobs()
    {
        auto labeledHue = wxpex::LabeledWidget(this->parent_, "Hue", this->hue);

        auto labeledSaturation =
            wxpex::LabeledWidget(this->parent_, "Saturation", this->saturation);

        auto labeledValue =
            wxpex::LabeledWidget(this->parent_, "Value", this->value);

        if constexpr (tau::HasAlpha<Color>)
        {
            return LayoutLabeled(
                LayoutOptions{},
                labeledHue,
                labeledSaturation,
                labeledValue,
                static_cast<Derived *>(this)->MakeAlphaWidget());
        }
        else
        {
            return LayoutLabeled(
                LayoutOptions{},
                labeledHue,
                labeledSaturation,
                labeledValue);
        }
    }

    void OnColorChanged_(const Color &color)
    {
        this->hue->GetKnob().SetColor(color);
    }

public:
    HueKnob *hue;
    SaturationKnob *saturation;
    ValueKnob *value;

protected:
    wxWindow *parent_;
    pex::MakeConnector<HsvKnobs_, Control> connect_;
};


using HsvKnobs = HsvKnobs_<HsvControl, void>;


class HsvaKnobs: public HsvKnobs_<HsvaControl, HsvaKnobs>
{
public:
    using AlphaRange =
        pex::control::LinearRange<decltype(HsvaControl::alpha), 100>;

    using AlphaControl = decltype(HsvaControl::alpha);

    using AlphaKnob =
        ::wxpex::ValueKnob<AlphaRange, typename AlphaControl::Value, 4, 2>;

    HsvaKnobs(
        wxWindow *parent,
        HsvaControl control)
        :
        HsvKnobs_<HsvaControl, HsvaKnobs>(parent, control),
        alpha(
            new AlphaKnob(
                parent,
                AlphaRange(control.alpha),
                control.alpha.value))
    {

    }

    auto MakeAlphaWidget()
    {
        return wxpex::LabeledWidget(this->parent_, "Alpha", this->alpha);
    }

    AlphaKnob *alpha;
};


template<typename Knobs, typename Control>
class HsvPicker_: public StaticBox
{
public:
    static constexpr auto observerName = "HsvPicker";

    HsvPicker_(
        wxWindow *parent,
        const std::string &name,
        Control control)
        :
        StaticBox(parent, name),
        knobs_(this, control)
    {
        auto colorPreview = new ColorPreview(this, control);
        auto knobLayout = this->knobs_.MakeKnobs();
        auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
        sizer->Add(knobLayout.release(), 1, wxRight, 5);
        auto vertical = std::make_unique<wxBoxSizer>(wxVERTICAL);

        vertical->Add(
            colorPreview,
            0,
            wxALIGN_CENTER | wxALL,
            10);

        sizer->Add(vertical.release(), 0, wxEXPAND);
        this->ConfigureSizer(std::move(sizer));
    }

private:
    Knobs knobs_;
};


using HsvPicker = HsvPicker_<HsvKnobs, HsvControl>;
using HsvaPicker = HsvPicker_<HsvaKnobs, HsvaControl>;

/**************************************/

template<typename Control, typename Derived>
class RgbSliders_
{
public:
    using RedControl = decltype(Control::red);
    using GreenControl = decltype(Control::green);
    using BlueControl = decltype(Control::blue);

    using RedSlider =
        ::wxpex::ValueSlider<RedControl, typename RedControl::Value, 3>;

    using GreenSlider =
        ::wxpex::ValueSlider<GreenControl, typename GreenControl::Value, 3>;

    using BlueSlider =
        ::wxpex::ValueSlider<BlueControl, typename BlueControl::Value, 3>;

    using Color = typename Control::Type;

public:
    RgbSliders_(
        wxWindow *parent,
        Control control)
        :
        red(
            new RedSlider(
                parent,
                control.red,
                control.red.value)),
        green(
            new GreenSlider(
                parent,
                control.green,
                control.green.value)),
        blue(
            new BlueSlider(
                parent,
                control.blue,
                control.blue.value)),
        parent_(parent)
    {

    }

    std::unique_ptr<wxSizer> MakeSliders()
    {
        auto labeledRed = wxpex::LabeledWidget(this->parent_, "Red", this->red);

        auto labeledGreen =
            wxpex::LabeledWidget(this->parent_, "Green", this->green);

        auto labeledBlue =
            wxpex::LabeledWidget(this->parent_, "Blue", this->blue);

        if constexpr (tau::HasAlpha<Color>)
        {
            return LayoutLabeled(
                LayoutOptions{},
                labeledRed,
                labeledGreen,
                labeledBlue,
                static_cast<Derived *>(this)->MakeAlphaWidget());
        }
        else
        {
            return LayoutLabeled(
                LayoutOptions{},
                labeledRed,
                labeledGreen,
                labeledBlue);
        }
    }

public:
    RedSlider *red;
    GreenSlider *green;
    BlueSlider *blue;

protected:
    wxWindow *parent_;
};


using RgbSliders = RgbSliders_<RgbControl, void>;


class RgbaSliders: public RgbSliders_<RgbaControl, RgbaSliders>
{
public:
    using AlphaControl = decltype(RgbaControl::alpha);

    using AlphaSlider =
        ValueSlider<AlphaControl, typename AlphaControl::Value, 3>;

    RgbaSliders(
        wxWindow *parent,
        RgbaControl control)
        :
        RgbSliders_<RgbaControl, RgbaSliders>(parent, control),
        alpha(
            new AlphaSlider(
                parent,
                control.alpha,
                control.alpha.value))
    {

    }

    auto MakeAlphaWidget()
    {
        return wxpex::LabeledWidget(this->parent_, "Alpha", this->alpha);
    }

    AlphaSlider *alpha;
};


template<typename Sliders, typename Control>
class RgbPicker_: public StaticBox
{
public:
    static constexpr auto observerName = "HsvPicker";

    RgbPicker_(
        wxWindow *parent,
        const std::string &name,
        Control control)
        :
        StaticBox(parent, name),
        sliders_(this, control)
    {
        auto colorPreview = new ColorPreview(this, control);
        auto sliderLayout = this->sliders_.MakeSliders();
        auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
        sizer->Add(sliderLayout.release(), 1, wxRight, 5);
        auto vertical = std::make_unique<wxBoxSizer>(wxVERTICAL);

        vertical->Add(
            colorPreview,
            0,
            wxALIGN_CENTER | wxALL,
            10);

        sizer->Add(vertical.release(), 0, wxEXPAND);
        this->ConfigureSizer(std::move(sizer));
    }

private:
    Sliders sliders_;
};


using RgbPicker = RgbPicker_<RgbSliders, RgbControl>;
using RgbaPicker = RgbPicker_<RgbaSliders, RgbaControl>;


} // end namespace wxpex
