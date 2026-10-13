/**
  * @file slider.h
  *
  * @brief A wxSlider connected to a pex::Bounded.
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
#include <concepts>

#include <jive/optional.h>
#include <jive/scope_flag.h>
#include <pex/bounded.h>
#include <pex/endpoint.h>

#include "wxpex/wxshim.h"
#include "wxpex/view.h"
#include "wxpex/spin_control.h"
#include "wxpex/field.h"
#include "wxpex/layout_top_level.h"
#include "wxpex/converter.h"
#include "wxpex/style.h"
#include "wxpex/async.h"


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

    void SetRange(int minimum, int maximum)
    {
        this->minimum_ = minimum;
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


/** wxSlider uses `int`. If the Range type is integral, so the default filter will attempt to convert it to `int`. Floating-point types will default to a LinearBounded that maps the range of possible values across the positions of the slider.
 **/
template<typename BoundedControl, typename Enable = void>
struct FilteredBounded_ {};


template<typename BoundedControl>
struct FilteredBounded_
<
    BoundedControl,
    std::enable_if_t
    <
        std::is_same_v
        <
            NotOptional<BoundedControl>,
            int
        >
    >
>
{
    // This BoundedControl already has type 'int'
    using Type = BoundedControl;
};


template<typename BoundedControl>
struct FilteredBounded_
<
    BoundedControl,
    std::enable_if_t
    <
        std::is_floating_point_v
        <
            NotOptional<BoundedControl>
        >
    >
>
{
    // This BoundedControl has floating-point type.
    using Type = pex::control::LinearBounded
        <
            typename BoundedControl::Upstream,
            typename BoundedControl::Access
        >;
};


template<typename BoundedControl>
struct FilteredBounded_
<
    BoundedControl,
    std::enable_if_t
    <
        !std::is_same_v<NotOptional<BoundedControl>, int>
        && std::is_integral_v<NotOptional<BoundedControl>>
    >
>
{
    // This BoundedControl has integral type that is not int.
    using Type = pex::control::ConvertingBounded
        <
            typename BoundedControl::Upstream,
            int,
            typename BoundedControl::Access
        >;
};


template<typename BoundedControl>
using FilteredBounded = typename FilteredBounded_<BoundedControl>::Type;


template<typename BoundedControl>
std::unique_ptr<FilteredBounded<BoundedControl>> GetDefaultFilteredRange(
    const BoundedControl &range)
{
    using Result = FilteredBounded<BoundedControl>;

    if constexpr (std::is_floating_point_v<NotOptional<BoundedControl>>)
    {
        return std::make_unique<FilteredBounded<BoundedControl>>(
            range,
            typename Result::Filter(1000));
    }
    else
    {
        return std::make_unique<FilteredBounded<BoundedControl>>(range);
    }
}


template<typename Filter>
concept HasGetSlope = requires(Filter f)
{
    {f.GetSlope()} -> std::floating_point;
};


static_assert(HasGetSlope<pex::control::LinearFilter<double>>);


template
<
    typename BoundedControl
>
class Slider : public wxSlider
{
public:
    static constexpr auto observerName = "wxpex::Slider";

    using Base = wxSlider;
    using This = Slider<BoundedControl>;

    // FilteredBounded automatically scales floating point types to int as
    // required by wxSlider.
    using Bounded = FilteredBounded<BoundedControl>;
    using Unfiltered = typename Bounded::Unfiltered;
    static constexpr bool hasGetSlope = HasGetSlope<typename Bounded::Filter>;

    using Value = pex::control::Value<typename Bounded::Value>;
    using ValueType = typename Value::Type;
    static_assert(std::is_same_v<int, jive::RemoveOptional<ValueType>>);
    static constexpr bool isOptional = jive::IsOptional<ValueType>;

    using Limit = pex::control::Value<typename Bounded::Limit>;

    Slider(
        wxWindow *parent,
        BoundedControl range,
        Style style = Style::horizontal)
        :
        Base(
            parent,
            wxID_ANY,
            0,
            -100,
            100,
            wxDefaultPosition,
            wxDefaultSize,
            SliderStyle(style)),

        ignoreRange_(false),
        range_(GetDefaultFilteredRange(range)),
        value_(this, this->range_->value),
        minimum_(this, this->range_->minimum),
        maximum_(this, this->range_->maximum),
        reset_(range.reset),

        styleFilter_(
            style,
            this->range_->minimum.Get(),
            this->range_->maximum.Get()),

        adjustRange_(std::bind(&Slider::AdjustRange_, this))
    {
        PEX_NAME("Slider");

        this->SetRange(
            this->range_->minimum.Get(),
            this->range_->maximum.Get());

        auto value = this->range_->value.Get();

        this->OnValue_(value);

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

    void AdjustRange_()
    {
        if constexpr (hasGetSlope)
        {
            jive::ScopeFlag ignoreRange(this->ignoreRange_);

            int valueCount = this->maximum_.Get() - this->minimum_.Get();
            Unfiltered currentSlope = this->range_->GetFilter().GetSlope();

            if (valueCount != 1000)
            {
                currentSlope *= Unfiltered(1000) / Unfiltered(valueCount);
            }

            this->range_->SetFilter({currentSlope});

            int minimum = this->minimum_.Get();
            int maximum = this->maximum_.Get();

            this->styleFilter_.SetRange(minimum, maximum);
            this->SetRange(minimum, maximum);
            this->OnValue_(this->value_.Get());
        }
    }

    void OnMinimum_(int minimum)
    {
        if (this->ignoreRange_)
        {
            return;
        }

        if constexpr (hasGetSlope)
        {
            this->adjustRange_();
        }
        else
        {
            this->SetMin(minimum);
            this->styleFilter_.SetMinimum(minimum);
        }
    }

    void OnMaximum_(int maximum)
    {
        if (this->ignoreRange_)
        {
            return;
        }

        if constexpr (hasGetSlope)
        {
            this->adjustRange_();
        }
        else
        {
            this->SetMax(maximum);
            this->styleFilter_.SetMaximum(maximum);
        }
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
    bool ignoreRange_;
    std::unique_ptr<Bounded> range_;
    pex::Endpoint<Slider, Value> value_;
    pex::Endpoint<Slider, Limit> minimum_;
    pex::Endpoint<Slider, Limit> maximum_;
    pex::control::DefaultSignal reset_;
    detail::StyleFilter styleFilter_;
    wxpex::CallAfter adjustRange_;
};


template
<
    typename BoundedControl,
    typename ValueControl,
    typename Convert
>
class ValueSliderConvert : public wxControl
{
public:
    using Base = wxControl;
    using BoundedSlider = Slider<BoundedControl>;
    using SliderBounded = typename BoundedSlider::Bounded;
    using SliderValue = typename BoundedSlider::Value;

    static constexpr auto observerName = "wxpex::ValueSliderConvert";

    // range is filtered to an int for direct use in the wx.Slider.
    // value is the value from the model for display in the view.
    ValueSliderConvert(
        wxWindow *parent,
        BoundedControl range,
        ValueControl value,
        Style style = Style::horizontal)
        :
        Base(parent, wxID_ANY),
        sliderIsActive_(false),

        value_(
            PEX_THIS("wxpex::ValueSliderConvert"),
            value,
            &ValueSliderConvert::OnValue_)
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

    void OnValue_(typename ValueControl::Type)
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
    pex::Endpoint<ValueSliderConvert, ValueControl> value_;
};


template<typename BoundedControl, typename ValueControl, int precision>
using ValueSliderBase = ValueSliderConvert
    <
        BoundedControl,
        ValueControl,
        PrecisionConverter<ValueControl, precision>
    >;


template
<
    typename BoundedControl,
    typename ValueControl,
    int precision = 3
>
class ValueSlider
    :
    public ValueSliderBase<BoundedControl, ValueControl, precision>
{
    using Base =
        ValueSliderBase<BoundedControl, ValueControl, precision>;

public:
    ValueSlider(
        wxWindow *parent,
        BoundedControl range,
        ValueControl value,
        Style style = Style::horizontal)
        :
        Base(parent, range, value, style)
    {

    }

};


template<typename BoundedControl>
class SpinSlider: public wxControl
{
public:
    SpinSlider(wxWindow *parent, BoundedControl range)
        :
        wxControl(parent, wxID_ANY)
    {
        auto slider =
            new Slider<BoundedControl>(this, range, Style::horizontal);

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
    typename BoundedControl,
    typename ValueControl,
    typename Convert = pex::Converter<typename ValueControl::Type>
>
class FieldSliderConvert: public wxControl
{
public:
    using ValueField = Field<ValueControl, Convert>;

    FieldSliderConvert(
        wxWindow *parent,
        BoundedControl range,
        ValueControl value,
        Style style = Style::horizontal)
        :
        wxControl(parent, wxID_ANY)
    {
        auto slider =
            new Slider<BoundedControl>(this, range, style);

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


template<typename BoundedControl, typename ValueControl, int precision>
using FieldSliderBase = FieldSliderConvert
    <
        BoundedControl,
        ValueControl,
        PrecisionConverter<ValueControl, precision>
    >;


template
<
    typename BoundedControl,
    typename ValueControl,
    int precision = 3
>
class FieldSlider
    :
    public FieldSliderBase<BoundedControl, ValueControl, precision>
{
    using Base =
        FieldSliderBase<BoundedControl, ValueControl, precision>;

public:
    FieldSlider(
        wxWindow *parent,
        BoundedControl range,
        ValueControl value,
        Style style = Style::horizontal)
        :
        Base(parent, range, value, style)
    {

    }
};


template<int precision, typename BoundedControl, typename ValueControl>
auto CreateFieldSlider(
    wxWindow *parent,
    BoundedControl range,
    ValueControl value,
    Style style = Style::horizontal)
{
    using Result = FieldSlider<BoundedControl, ValueControl, precision>;
    return new Result(parent, range, value, style);
}


template<int precision, typename BoundedControl>
auto CreateFieldSlider(
    wxWindow *parent,
    BoundedControl range,
    Style style = Style::horizontal)
{
    using Result =
        FieldSlider<BoundedControl, decltype(BoundedControl::value), precision>;

    return new Result(parent, range, range.value, style);
}


template<int precision, typename BoundedControl, typename ValueControl>
auto CreateViewSlider(
    wxWindow *parent,
    BoundedControl range,
    ValueControl value,
    Style style = Style::horizontal)
{
    using Result = ValueSlider<BoundedControl, ValueControl, precision>;
    return new Result(parent, range, value, style);
}


template<int precision, typename BoundedControl>
auto CreateViewSlider(
    wxWindow *parent,
    BoundedControl range,
    Style style = Style::horizontal)
{
    using Result =
        ValueSlider<BoundedControl, decltype(BoundedControl::value), precision>;

    return new Result(parent, range, range.value, style);
}


} // namespace wxpex
