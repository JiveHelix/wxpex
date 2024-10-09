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

#include <jive/optional.h>
#include <pex/range.h>

#include "wxpex/wxshim.h"
#include "wxpex/view.h"
#include "wxpex/spin_control.h"
#include "wxpex/field.h"
#include "wxpex/layout_top_level.h"
#include "wxpex/converter.h"
#include "wxpex/style.h"


namespace wxpex
{


namespace detail
{

struct StyleFilter
{
public:
    StyleFilter(Style style, int minimum, int maximum)
        :
        isVertical_(IsVertical(style)),
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


wxDEFINE_EVENT(SliderBegin, wxCommandEvent);
wxDEFINE_EVENT(SliderDone, wxCommandEvent);


template<typename T>
using RangeValue = typename T::Value;

template<typename T>
using RangeValueType = typename RangeValue<T>::Type;


template<typename T>
using NotOptional = jive::RemoveOptional<RangeValueType<T>>;


/** wxSlider uses `int`. If the Range type is integral, so the default filter will attempt to convert it to `int`. Floating-point types will default to a LinearRange that maps the range of possible values across the positions of the slider.
 **/
template<typename RangeControl, typename Enable = void>
struct FilteredRange_ {};


template<typename RangeControl>
struct FilteredRange_
<
    RangeControl,
    std::enable_if_t
    <
        std::is_same_v
        <
            NotOptional<RangeControl>,
            int
        >
    >
>
{
    // This RangeControl already has type 'int'
    using Type = RangeControl;
};


template<typename RangeControl>
struct FilteredRange_
<
    RangeControl,
    std::enable_if_t
    <
        std::is_floating_point_v
        <
            NotOptional<RangeControl>
        >
    >
>
{
    // This RangeControl has floating-point type.
    using Type = pex::control::LinearRange
        <
            typename RangeControl::Upstream,
            1000,
            typename RangeControl::Access
        >;
};


template<typename RangeControl>
struct FilteredRange_
<
    RangeControl,
    std::enable_if_t
    <
        !std::is_same_v<NotOptional<RangeControl>, int>
        && std::is_integral_v<NotOptional<RangeControl>>
    >
>
{
    // This RangeControl has integral type that is not int.
    using Type = pex::control::ConvertingRange
        <
            typename RangeControl::Upstream,
            int,
            typename RangeControl::Access
        >;
};


template<typename RangeControl>
using FilteredRange = typename FilteredRange_<RangeControl>::Type;


template<typename RangeControl>
int GetInitialValue(RangeControl range)
{
    using Range = FilteredRange<RangeControl>;

    using Type = typename Range::Type;

    auto value = Range(range).value.Get();

    if constexpr (jive::IsOptional<Type>)
    {
        if (!value)
        {
            return 0;
        }

        return *value;
    }
    else
    {
        return value;
    }
}


template
<
    typename RangeControl
>
class Slider : public wxSlider
{
public:
    static constexpr auto observerName = "wxpex::Slider";

    using Base = wxSlider;
    using This = Slider<RangeControl>;

    // FilteredRange automatically scales floating point types to int as
    // required by wxSlider.
    using Range = FilteredRange<RangeControl>;

    using Value = typename Range::Value;
    using ValueType = typename Value::Type;
    static_assert(std::is_same_v<int, jive::RemoveOptional<ValueType>>);
    static constexpr bool isOptional = jive::IsOptional<ValueType>;

    using Limit = typename Range::Limit;

    Slider(
        wxWindow *parent,
        RangeControl range,
        Style style = Style::horizontal)
        :
        Base(
            parent,
            wxID_ANY,

            detail::StyleFilter(
                style,
                Range(range).minimum.Get(),
                Range(range).maximum.Get())(GetInitialValue(range)),

            Range(range).minimum.Get(),
            Range(range).maximum.Get(),
            wxDefaultPosition,
            wxDefaultSize,
            SliderStyle(style)),

        value_(this, range.value),
        minimum_(this, range.minimum),
        maximum_(this, range.maximum),
        reset_(range.reset),

        styleFilter_(
            style,
            Range(range).minimum.Get(),
            Range(range).maximum.Get())
    {
        this->value_.Connect(&Slider::OnValue_);
        this->minimum_.Connect(&Slider::OnMinimum_);
        this->maximum_.Connect(&Slider::OnMaximum_);

        this->Bind(wxEVT_SLIDER, &Slider::OnSlider_, this);
        this->Bind(wxEVT_LEFT_DOWN, &Slider::OnSliderLeftDown_, this);
        this->Bind(wxEVT_LEFT_UP, &Slider::OnSliderLeftUp_, this);
        this->Bind(wxEVT_MOUSEWHEEL, &Slider::OnMousewheel_, this);

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

    void OnValue_(pex::Argument<ValueType> value)
    {
        if constexpr (isOptional)
        {
            if (value)
            {
                this->SetValue(this->styleFilter_(*value));
            }
        }
        else
        {
            this->SetValue(this->styleFilter_(value));
        }
    }

    void OnMinimum_(int minimum)
    {
        this->SetMin(minimum);
        this->styleFilter_.SetMinimum(minimum);
    }

    void OnMaximum_(int maximum)
    {
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
            this->reset_.Trigger();
        }
        else
        {
            event.Skip();
        }

        this->AddPendingEvent(wxCommandEvent(SliderBegin));
    }

    void OnSliderLeftUp_(wxMouseEvent &event)
    {
        event.Skip();
        this->AddPendingEvent(wxCommandEvent(SliderDone));
    }

    void ForwardMousewheelEvent(wxMouseEvent &event)
    {
        wxWindow *top = wxGetTopLevelParent(this);
        wxWindow *target = this->GetParent();

        while (true)
        {
            if (target->ProcessWindowEvent(event))
            {
                return;
            }

            if (target == top)
            {
                return;
            }

            target = target->GetParent();
        }
    }

    void OnMousewheel_(wxMouseEvent &event)
    {
        this->ForwardMousewheelEvent(event);
    }

private:
    pex::Terminus<Slider, Value> value_;
    pex::Terminus<Slider, Limit> minimum_;
    pex::Terminus<Slider, Limit> maximum_;
    pex::control::Signal<> reset_;
    detail::StyleFilter styleFilter_;
};


template
<
    typename RangeControl,
    typename ValueControl,
    typename Convert
>
class ValueSliderConvert : public wxControl
{
public:
    using Base = wxControl;
    using RangeSlider = Slider<RangeControl>;
    using SliderRange = typename RangeSlider::Range;
    using SliderValue = typename RangeSlider::Value;

    static constexpr auto observerName = "wxpex::ValueSliderConvert";

    // range is filtered to an int for direct use in the wx.Slider.
    // value is the value from the model for display in the view.
    ValueSliderConvert(
        wxWindow *parent,
        RangeControl range,
        ValueControl value,
        Style style = Style::horizontal)
        :
        Base(parent, wxID_ANY),
        sliderIsActive_(false),
        value_(this, SliderRange(range).value)

    {
        // Create slider and view as children of this wxWindow.
        // They are memory managed by the the wxWindow.
        auto slider = new Slider(this, range, style);
        auto view = new View<ValueControl, Convert>(this, value);

        // Use a mono-spaced font for display so that the width of the view
        // remains constant as the value changes.
        view->SetFont(wxFont(wxFontInfo().Family(wxFONTFAMILY_MODERN)));

        auto sizerStyle = SizerStyle(style);

        auto sizer = std::make_unique<wxBoxSizer>(sizerStyle);

        auto flag = IsHorizontal(style)
            ? wxRIGHT | wxEXPAND
            : wxBOTTOM | wxEXPAND;

        auto spacing = 5;
        sizer->Add(slider, 1, flag, spacing);

        sizer->Add(
            view,
            0,
            IsHorizontal(style)
                ? wxALIGN_CENTER_VERTICAL
                : wxALIGN_CENTER);

        this->SetSizer(sizer.release());

        this->Bind(
            SliderBegin,
            &ValueSliderConvert::OnSliderBegin_,
            this);

        this->Bind(
            SliderDone,
            &ValueSliderConvert::OnSliderDone_,
            this);

        this->value_.Connect(&ValueSliderConvert::OnValue_);
    }

private:
    void OnSliderBegin_(wxCommandEvent &event)
    {
        event.Skip();
        this->sliderIsActive_ = true;
    }

    void OnSliderDone_(wxCommandEvent &event)
    {
        event.Skip();
        this->Layout();
        this->sliderIsActive_ = false;
    }

    void OnValue_(int)
    {
        if (!this->sliderIsActive_)
        {
            // This is a value from the application model.
            // Do Layout now so that the value will be displayed properly.
            this->Layout();
        }
        // else
        // Layout will be called when slider has been released.
    }

private:
    bool sliderIsActive_;
    pex::Terminus<ValueSliderConvert, SliderValue> value_;
};


template<typename RangeControl, typename ValueControl, int precision>
using ValueSliderBase = ValueSliderConvert
    <
        RangeControl,
        ValueControl,
        PrecisionConverter<ValueControl, precision>
    >;


template
<
    typename RangeControl,
    typename ValueControl,
    int precision = 3
>
class ValueSlider
    :
    public ValueSliderBase<RangeControl, ValueControl, precision>
{
    using Base =
        ValueSliderBase<RangeControl, ValueControl, precision>;

public:
    ValueSlider(
        wxWindow *parent,
        RangeControl range,
        ValueControl value,
        Style style = Style::horizontal)
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
            new Slider<RangeControl>(this, range, Style::horizontal);

        auto spin =
            new SpinControl(this, range, 1, 0);

        auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
        sizer->Add(slider, 1, wxRIGHT | wxEXPAND, 3);
        sizer->Add(spin, 0, wxALIGN_CENTER);

        this->SetSizer(sizer.release());

        this->Bind(
            SliderDone,
            &SpinSlider::OnSliderDone_,
            this);
    }

private:
    void OnSliderDone_(wxCommandEvent &event)
    {
        event.Skip();
        this->Layout();
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

    FieldSliderConvert(
        wxWindow *parent,
        RangeControl range,
        ValueControl value,
        Style style = Style::horizontal)
        :
        wxControl(parent, wxID_ANY)
    {
        auto slider =
            new Slider<RangeControl>(this, range, style);

        auto field = new ValueField(this, value);

        auto sizerStyle = SizerStyle(style);

        auto sizer = std::make_unique<wxBoxSizer>(sizerStyle);

        auto flag = IsHorizontal(style)
            ? wxRIGHT | wxEXPAND
            : wxBOTTOM | wxEXPAND;

        auto spacing = 5;
        sizer->Add(slider, 1, flag, spacing);

        sizer->Add(
            field,
            0,
            IsHorizontal(style)
                ? wxALIGN_CENTER_VERTICAL
                : wxALIGN_CENTER);

        this->SetSizer(sizer.release());

        this->Bind(
            SliderDone,
            &FieldSliderConvert::OnSliderDone_,
            this);
    }

private:
    void OnSliderDone_(wxCommandEvent &event)
    {
        event.Skip();
        this->Layout();
    }
};


template<typename RangeControl, typename ValueControl, int precision>
using FieldSliderBase = FieldSliderConvert
    <
        RangeControl,
        ValueControl,
        PrecisionConverter<ValueControl, precision>
    >;


template
<
    typename RangeControl,
    typename ValueControl,
    int precision = 3
>
class FieldSlider
    :
    public FieldSliderBase<RangeControl, ValueControl, precision>
{
    using Base =
        FieldSliderBase<RangeControl, ValueControl, precision>;

public:
    FieldSlider(
        wxWindow *parent,
        RangeControl range,
        ValueControl value,
        Style style = Style::horizontal)
        :
        Base(parent, range, value, style)
    {

    }
};


template<int precision, typename RangeControl, typename ValueControl>
auto CreateFieldSlider(
    wxWindow *parent,
    RangeControl range,
    ValueControl value,
    Style style = Style::horizontal)
{
    using Result = FieldSlider<RangeControl, ValueControl, precision>;
    return new Result(parent, range, value, style);
}


template<int precision, typename RangeControl>
auto CreateFieldSlider(
    wxWindow *parent,
    RangeControl range,
    Style style = Style::horizontal)
{
    using Result =
        FieldSlider<RangeControl, decltype(RangeControl::value), precision>;

    return new Result(parent, range, range.value, style);
}


template<int precision, typename RangeControl, typename ValueControl>
auto CreateViewSlider(
    wxWindow *parent,
    RangeControl range,
    ValueControl value,
    Style style = Style::horizontal)
{
    using Result = ValueSlider<RangeControl, ValueControl, precision>;
    return new Result(parent, range, value, style);
}


template<int precision, typename RangeControl>
auto CreateViewSlider(
    wxWindow *parent,
    RangeControl range,
    Style style = Style::horizontal)
{
    using Result =
        ValueSlider<RangeControl, decltype(RangeControl::value), precision>;

    return new Result(parent, range, range.value, style);
}


} // namespace wxpex
