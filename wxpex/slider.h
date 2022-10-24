/**
  * @file slider.h
  *
  * @brief A wxSlider connected to a pex::Range.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 14 Aug 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#pragma once

#include <limits>
#include <stdexcept>
#include <cstdint>

#include <pex/range.h>
#include <pex/converter.h>

#include "wxpex/wxshim.h"
#include "wxpex/view.h"
#include "wxpex/spin_control.h"
#include "wxpex/field.h"
#include "wxpex/layout_top_level.h"


namespace wxpex
{


namespace detail
{

struct StyleFilter
{
public:
    StyleFilter(long style, int minimum, int maximum)
        :
        isVertical_(style & wxSL_VERTICAL),
        minimum_(minimum),
        maximum_(maximum),
        offset_(maximum + minimum)
    {

    }

    int operator()(int value)
    {
        if (!this->isVertical_)
        {
            return value;
        }
        
        // Reverse the values so that the slider is at the top for the highest
        // value.
        return this->offset_ - value;
    }

    void SetMinimum(int minimum)
    {
        this->minimum_ = minimum;
        this->offset_ = this->maximum_ + this->minimum_;
    }

    void SetMaximum(int maximum)
    {
        this->maximum_ = maximum;
        this->offset_ = this->maximum_ + this->minimum_;
    }

private:
    bool isVertical_;
    int minimum_;
    int maximum_;
    int offset_;
};


} // end namespace detail


wxDEFINE_EVENT(SliderDone, wxCommandEvent);


/** wxSlider uses `int`, so the default filter will attempt to convert T to
 ** `int`.
 **/

template
<
    typename RangeControl
>
class Slider : public wxSlider
{
public:
    using Base = wxSlider;
    using This = Slider<RangeControl>;

    // Value and Limit are observed by This
    using Range = pex::control::ChangeObserver<This, RangeControl>;

    using Value = typename Range::Value;
    using Limit = typename Range::Limit;

    static_assert(
        std::is_same_v<int, typename Value::Type>,
        "Slider control uses int");

    static_assert(
        std::is_same_v<int, typename Limit::Type>,
        "Slider control uses int");

    Slider(
        wxWindow *parent,
        RangeControl range,
        long style = wxSL_HORIZONTAL)
        :
        Base(
            parent,
            wxID_ANY,
            detail::StyleFilter(
                style,
                range.minimum.Get(),
                range.maximum.Get())(range.value.Get()),
            range.minimum.Get(),
            range.maximum.Get(),
            wxDefaultPosition,
            wxDefaultSize,
            style),
        value_(this, range.value),
        minimum_(this, range.minimum),
        maximum_(this, range.maximum),
        defaultValue_(this->value_.Get()),
        styleFilter_(style, range.minimum.Get(), range.maximum.Get())
    {
        PEX_LOG(this);
        PEX_LOG("\nvalue_: ", &this->value_);
        PEX_LOG("\nminimum_: ", &this->minimum_);
        PEX_LOG("\nmaximum_: ", &this->maximum_);

        this->value_.Connect(&Slider::OnValue_);
        this->minimum_.Connect(&Slider::OnMinimum_);
        this->maximum_.Connect(&Slider::OnMaximum_);

        this->Bind(wxEVT_SLIDER, &Slider::OnSlider_, this);
        this->Bind(wxEVT_LEFT_DOWN, &Slider::OnSliderLeftDown_, this);
        this->Bind(wxEVT_LEFT_UP, &Slider::OnSliderLeftUp_, this);
        
        // wxSlider appears to underreport its minimum size, which causes the
        // thumb to be clipped.
        // TODO: Determine whether this affects platforms other than wxMac.
        auto bestSize = this->GetBestSize();
        auto bestHeight = bestSize.GetHeight();
        bestSize.SetHeight(static_cast<int>(bestHeight * 1.25));
        this->SetMinSize(bestSize);
    }

    ~Slider()
    {
        PEX_LOG(this);
    }

    void OnValue_(int value)
    {
        this->SetValue(this->styleFilter_(value));
    }

    void OnMinimum_(int minimum)
    {
        if (this->defaultValue_ < minimum)
        {
            this->defaultValue_ = minimum;
        }

        this->SetMin(minimum);
        this->styleFilter_.SetMinimum(minimum);
    }

    void OnMaximum_(int maximum)
    {
        if (this->defaultValue_ > maximum)
        {
            this->defaultValue_ = maximum;
        }

        this->SetMax(maximum);
        this->styleFilter_.SetMaximum(maximum);
    }

    void OnSlider_(wxCommandEvent &event)
    {
        // wx generates multiple wxEVT_SLIDER events with the same value.
        // We will only send changes.
        auto newValue = this->styleFilter_(event.GetInt());

        if (newValue != this->value_.Get())
        {
            this->value_.Set(newValue);
        }
    }

    void OnSliderLeftDown_(wxMouseEvent &event)
    {
        if (event.AltDown())
        {
            // Restore the default.
            this->value_.Set(this->defaultValue_);
        }
        else
        {
            event.Skip();
        }
    }

    void OnSliderLeftUp_(wxMouseEvent &event)
    {
        event.Skip();
        this->AddPendingEvent(wxCommandEvent(SliderDone));
    }

private:
    pex::Terminus<Slider, Value> value_;
    pex::Terminus<Slider, Limit> minimum_;
    pex::Terminus<Slider, Limit> maximum_;
    int defaultValue_;
    detail::StyleFilter styleFilter_;
};


template<int base, int width, int precision>
struct ViewTraits:
    pex::ConverterTraits<base, width, precision, jive::flag::None>
{

};


template
<
    typename RangeControl,
    typename ValueControl,
    typename Convert
>
class SliderAndValueConvert : public wxControl
{
public:
    using Base = wxControl;

    // range is filtered to an int for direct use in the wx.Slider.
    // value is the value from the model for display in the view.
    SliderAndValueConvert(
        wxWindow *parent,
        RangeControl range,
        ValueControl value,
        long style = wxSL_HORIZONTAL)
        :
        Base(parent, wxID_ANY)
    {
        // Create slider and view as children of the this wxWindow.
        // They are memory managed by the the wxWindow from their creation.
        auto slider = new Slider<RangeControl>(this, range, style);
        auto view = new View<ValueControl, Convert>(this, value);

        // Use a mono-spaced font for display so that the width of the view
        // remains constant as the value changes.
        view->SetFont(wxFont(wxFontInfo().Family(wxFONTFAMILY_MODERN)));

        auto sizerStyle =
            (wxSL_HORIZONTAL == style) ? wxHORIZONTAL : wxVERTICAL;

        auto sizer = std::make_unique<wxBoxSizer>(sizerStyle);

        auto flag = (wxSL_HORIZONTAL == style)
            ? wxRIGHT | wxEXPAND
            : wxBOTTOM | wxEXPAND;

        auto spacing = 5;

        sizer->Add(slider, 1, flag, spacing);

        sizer->Add(
            view,
            0,
            (wxSL_HORIZONTAL == wxHORIZONTAL)
                ? wxALIGN_CENTER
                : wxALIGN_CENTER_VERTICAL);

        this->SetSizerAndFit(sizer.release());
    }
};


template<typename RangeControl, typename ValueControl, int precision>
using SimplifiedSliderAndValue = SliderAndValueConvert
    <
        RangeControl,
        ValueControl,
        pex::Converter
        <
            typename ValueControl::Type,
            ViewTraits<10, 0, precision>
        >
    >;


template
<
    typename RangeControl,
    typename ValueControl,
    int precision = 3
>
class SliderAndValue
    :
    public SimplifiedSliderAndValue<RangeControl, ValueControl, precision>
{
    using Base =
        SimplifiedSliderAndValue<RangeControl, ValueControl, precision>;

public:
    SliderAndValue(
        wxWindow *parent,
        RangeControl range,
        ValueControl value,
        long style = wxSL_HORIZONTAL)
        :
        Base(parent, range, value, style)
    {

    }
};


template<typename RangeControl>
class SpinSlider: public wxControl
{
public:
    SpinSlider(wxWindow *parent, RangeControl range)
        :
        wxControl(parent, wxID_ANY)
    {
        auto slider =
            new Slider<RangeControl>(this, range, wxSL_HORIZONTAL);

        auto spin =
            new SpinControl(this, range, 1, 0);

        auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
        sizer->Add(slider, 1, wxRIGHT | wxEXPAND, 3);
        sizer->Add(spin, 0, wxALIGN_CENTER);

        this->SetSizerAndFit(sizer.release());
    }
};


template
<
    typename RangeControl,
    typename ValueControl,
    typename Convert = pex::Converter<typename ValueControl::Type>
>
class FieldSliderConvert: public wxControl
{
public:
    using ValueField = Field<ValueControl, Convert>;

    FieldSliderConvert(wxWindow *parent, RangeControl range, ValueControl value)
        :
        wxControl(parent, wxID_ANY)
    {
        auto slider =
            new Slider<RangeControl>(this, range, wxSL_HORIZONTAL);

        auto field = new ValueField(this, value);

        auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
        sizer->Add(slider, 1, wxRIGHT | wxEXPAND, 3);
        sizer->Add(field, 0, wxALIGN_CENTER);

        this->SetSizerAndFit(sizer.release());
    }
};


template<typename RangeControl, typename ValueControl, int precision>
using SimplifiedFieldSlider = FieldSliderConvert
    <
        RangeControl,
        ValueControl,
        pex::Converter
        <
            typename ValueControl::Type,
            ViewTraits<10, 0, precision>
        >
    >;


template
<
    typename RangeControl,
    typename ValueControl,
    int precision = 3
>
class FieldSlider
    :
    public SimplifiedFieldSlider<RangeControl, ValueControl, precision>
{
    using Base =
        SimplifiedFieldSlider<RangeControl, ValueControl, precision>;

public:
    FieldSlider(
        wxWindow *parent,
        RangeControl range,
        ValueControl value)
        :
        Base(parent, range, value)
    {

    }
};




} // namespace wxpex
