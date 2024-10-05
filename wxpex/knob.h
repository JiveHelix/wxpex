#pragma once

#include <tau/angles.h>
#include <pex/range.h>
#include <pex/converter.h>

#include "wxpex/view.h"
#include "wxpex/spin_control.h"
#include "wxpex/field.h"
#include "wxpex/layout_top_level.h"
#include "wxpex/wxshim.h"
#include "wxpex/size.h"
#include "wxpex/point.h"
#include "wxpex/converter.h"
#include "wxpex/style.h"
#include "wxpex/color.h"
#include "wxpex/graphics.h"
#include "wxpex/style.h"


#ifdef __WXMSW__
WXSHIM_PUSH_IGNORES
#include <wx/dcbuffer.h>
WXSHIM_POP_IGNORES
#endif // __WXMSW__


namespace wxpex
{


wxDEFINE_EVENT(KnobDone, wxCommandEvent);


class KnobSettings
{
public:
    using Rgb = tau::Rgb<uint8_t>;

    static constexpr unsigned defaultRadius = 20;

    /**
     ** We use knobs in a clockwise-is-increasing sense, so we'll specify knob
     ** angles in the same way, with 0 degrees strait up, and increasing angle
     ** rotating to the right.
     **/

    // -150 is at 7 o'clock
    static constexpr double defaultStartAngle = -150.0;

    // 150 is at 5 o'clock
    static constexpr double defaultEndAngle = 150.0;

    static constexpr double defaultSteps = 256.0;
    static constexpr double defaultFineStep = 0.01;

    KnobSettings()
        :
        radius(defaultRadius),
        startAngle(defaultStartAngle),
        endAngle(defaultEndAngle),
        steps(defaultSteps),
        fineStep(defaultFineStep),
        color{{128, 128, 128}},
        continuous(false),
        layout(Style::horizontal),
        spacing(5)
    {

    }

    // All setting functions return a reference to this instance so they can be
    // chained.
    //
    // settings.Radius(20).StartAngle(-90);

    KnobSettings & Radius(unsigned value)
    {
        this->radius = value;
        return *this;
    }

    KnobSettings & StartAngle(double value)
    {
        this->startAngle = value;
        return *this;
    }

    KnobSettings & EndAngle(double value)
    {
        this->endAngle = value;
        return *this;
    }

    KnobSettings & Steps(double value)
    {
        this->steps = value;
        return *this;
    }

    KnobSettings & FineStep(double value)
    {
        this->fineStep = value;
        return *this;
    }

    KnobSettings & Color(const Rgb &value)
    {
        this->color = value;
        return *this;
    }

    KnobSettings & Continuous(bool value)
    {
        this->continuous = value;
        return *this;
    }

    KnobSettings & Layout(Style value)
    {
        this->layout = value;
        return *this;
    }

    KnobSettings & Spacing(int value)
    {
        this->spacing = value;
        return *this;
    }

    double GetAngleRange() const
    {
        if (this->continuous)
        {
            return 360.0;
        }

        return this->endAngle - this->startAngle;
    }

    wxColour GetBaseColor() const
    {
        auto hsv = tau::RgbToHsv<double>(this->color);
        hsv.value = std::min(hsv.value, 0.8);
        return ToWxColour(tau::HsvToRgb<uint8_t>(hsv));
    }

    wxColour GetHighlightColor() const
    {
        auto hsv = tau::RgbToHsv<double>(this->color);
        hsv.value += 0.5;
        hsv.value = std::min(hsv.value, 1.0);
        return ToWxColour(tau::HsvToRgb<uint8_t>(hsv));
    }

    wxColour GetOutlineColor() const
    {
        auto hsv = tau::RgbToHsv<double>(this->color);

        if (hsv.value < 0.5)
        {
            hsv.value += 0.5;
        }
        else
        {
            hsv.value -= 0.5;
        }

        hsv.value = std::max(0.0, std::min(hsv.value, 1.0));

        return ToWxColour(tau::HsvToRgb<uint8_t>(hsv));
    }

    unsigned radius;
    double startAngle;
    double endAngle;
    double steps;
    double fineStep;
    Rgb color;
    bool continuous;
    Style layout;
    int spacing;
};


template<typename Range>
class Knob: public wxWindow
{

public:
    static constexpr auto observerName = "wxpex::Knob";

    using Base = wxWindow;
    using This = Knob<Range>;
    using Rgb = typename KnobSettings::Rgb;

    // Value and Limit are observed by This
    using ValueTerminus = pex::Terminus<This, typename Range::Value>;

    using LimitTerminus =
        pex::Terminus<This, typename Range::Limit>;

    using Type = typename Range::Value::Type;

    Knob(
        wxWindow *parent,
        Range control,
        const KnobSettings &settings = KnobSettings())
        :
        Base(parent, wxID_ANY),
        value_(this, control.value, &Knob::OnValue_),
        reset_(control.reset),
        localValue_(static_cast<double>(this->value_.Get())),
        minimum_(this, control.minimum, &Knob::OnMinimum_),
        maximum_(this, control.maximum, &Knob::OnMaximum_),
        settings_(settings),
        stepSize_(
            this->GetStepSize_(this->minimum_.Get(), this->maximum_.Get())),
        fineStepSize_(this->GetFineStepSize_(this->stepSize_)),
        radius_(settings.radius),
        startAngle_(settings.startAngle),
        endAngle_(settings.endAngle),
        continuous_(settings.continuous),
        angleRange_(settings.GetAngleRange()),
        valueRange_(
            static_cast<double>(this->maximum_.Get() - this->minimum_.Get())),
        valueOffset_(-static_cast<double>(this->minimum_.Get())),
        hasCapturedMouse_(false),
        mousePosition_(),
        color_(settings.GetBaseColor()),
        highlight_(settings.GetHighlightColor()),
        outline_(settings.GetOutlineColor())
    {
#ifdef __WXMSW__
        this->SetBackgroundStyle(wxBG_STYLE_PAINT);
#else
        this->SetBackgroundStyle(wxBG_STYLE_SYSTEM);
#endif
        this->Bind(wxEVT_PAINT, &Knob::OnPaint_, this);
        this->Bind(wxEVT_LEFT_DOWN, &Knob::OnMouseEvents_, this);
        this->Bind(wxEVT_MOTION, &Knob::OnMouseEvents_, this);
        this->Bind(wxEVT_LEFT_UP, &Knob::OnMouseEvents_, this);

#ifdef _WIN32
        this->Bind(wxEVT_MOUSE_CAPTURE_LOST, &Knob::OnMouseCaptureLost_, this);
#endif
    }

    wxSize DoGetBestClientSize() const override
    {
        auto side = static_cast<int>(this->radius_ * 2 + 2);
        return wxSize(side, side);
    }

    void SetColor(const Rgb &rgb)
    {
        this->settings_.Color(rgb);
        this->color_ = this->settings_.GetBaseColor();
        this->highlight_ = this->settings_.GetHighlightColor();
        this->outline_ = this->settings_.GetOutlineColor();
        this->Refresh();
    }

    void SetColor(const tau::Hsv<double> &hsv)
    {
        this->SetColor(tau::HsvToRgb<uint8_t>(hsv));
    }

    void SetColor(const tau::Hsva<double> &hsva)
    {
        this->SetColor(
            tau::Hsv<double>{{hsva.hue, hsva.saturation, hsva.value}});
    }

    void SetHue(double hue)
    {
        auto hsv = tau::RgbToHsv<double>(this->settings_.color);
        hsv.hue = std::min(360.0, std::max(0.0, hue));
        this->SetColor(hsv);
    }

private:
    double GetStepSize_(Type minimum, Type maximum) const
    {
        return static_cast<double>(maximum - minimum) / this->settings_.steps;
    }

    double GetFineStepSize_(double stepSize) const
    {
        return stepSize * this->settings_.fineStep;
    }

    void OnValue_(Type value)
    {
        if (!this->hasCapturedMouse_)
        {
            this->localValue_ = value;
        }

        this->Refresh();
    }

    void OnMinimum_(Type minimum)
    {
        this->stepSize_ = this->GetStepSize_(minimum, this->maximum_.Get());
        this->fineStepSize_ = this->GetFineStepSize_(this->stepSize_);

        this->valueRange_ =
            static_cast<double>(this->maximum_.Get() - minimum);

        this->valueOffset_ = -minimum;

        this->Refresh();
    }

    void OnMaximum_(Type maximum)
    {
        this->stepSize_ = this->GetStepSize_(this->minimum_.Get(), maximum);
        this->fineStepSize_ = this->GetFineStepSize_(this->stepSize_);

        this->valueRange_ =
            static_cast<double>(maximum - this->minimum_.Get());

        this->Refresh();
    }

    double GetAngle_() const
    {
        // scaled ranges from 0 to 1, and is a measure of how far we have
        // progressed through the possible range of values.
        double scaled =
            (this->value_.Get()
                - static_cast<double>(this->minimum_.Get()))
            / this->valueRange_;

        // 0 degrees is straight up, with positive angles clockwise.
        // Our coordinate system (with the origin in the top left) is 90
        // degrees ahead of the desired interpretation.
        return this->startAngle_ + (scaled * this->angleRange_) - 90.0;
    }

    void OnPaint_(wxPaintEvent &)
    {
#ifdef __WXMSW__
        wxBufferedPaintDC dc(this);
#else
        wxPaintDC dc(this);
#endif

        auto size = ToSize<double>(this->GetClientSize());

        auto center = (size / 2).ToPoint2d();
        auto gradientOffset = center;
        auto radius = double(this->radius_);
        auto offset = radius / 4;

        gradientOffset.x -= offset;
        gradientOffset.y -= offset;
        auto gradientBegin = ToWxPoint(gradientOffset);
        auto gradientEnd = ToWxPoint(center);

#ifdef __WXMSW__
        auto backgroundColor = this->GetBackgroundColour();
        dc.SetBrush(wxBrush(backgroundColor));
        dc.DrawRectangle(wxPoint(0, 0), ToWxSize(size));
#endif

        GraphicsContext graphicsContext(dc);

        // Draw gradient.
        graphicsContext->SetBrush(
            graphicsContext->CreateRadialGradientBrush(
                gradientBegin.x,
                gradientBegin.y,
                gradientEnd.x,
                gradientEnd.y,
                radius,
                this->highlight_,
                this->color_));

        auto circleBegin = center - radius;

        graphicsContext->SetPen(
            graphicsContext->CreatePen(wxPen(this->outline_, 1)));

        graphicsContext->DrawEllipse(
            circleBegin.x,
            circleBegin.y,
            int(radius * 2),
            int(radius * 2));

        // Draw indicator.
        graphicsContext->SetPen(
            graphicsContext->CreatePen(wxPen(this->outline_, 2)));

        auto indicatorAngle = tau::ToRadians(this->GetAngle_());

        auto indicatorVector = tau::Point2d<double>(
            std::cos(indicatorAngle),
            std::sin(indicatorAngle));

        auto indicatorBegin = center + indicatorVector * 0.66 * radius;
        auto indicatorEnd = center + indicatorVector * radius;

        graphicsContext->StrokeLine(
            indicatorBegin.x,
            indicatorBegin.y,
            indicatorEnd.x,
            indicatorEnd.y);
    }

    void OnMouseEvents_(wxMouseEvent &mouseEvent)
    {
        if (mouseEvent.LeftDown())
        {
            if (this->hasCapturedMouse_)
            {
                return;
            }

            if (mouseEvent.AltDown())
            {
                this->reset_.Trigger();
            }

            this->CaptureMouse();
            this->hasCapturedMouse_ = true;
            this->mousePosition_ = ToPoint<int>(mouseEvent.GetPosition());
        }
        else if (mouseEvent.LeftIsDown())
        {
            if (this->hasCapturedMouse_)
            {
                this->UpdateMousePosition_(
                    ToPoint<int>(mouseEvent.GetPosition()),
                    mouseEvent.ShiftDown(),
                    mouseEvent.ControlDown());
            }
        }
        else if (mouseEvent.LeftUp())
        {
            if (this->hasCapturedMouse_)
            {
                this->ReleaseMouse();
                this->hasCapturedMouse_ = false;

                // On mouse up, the value has finished adjusting. Update
                // localValue_ to the final value.
                this->localValue_ = static_cast<double>(
                    this->value_.Get());

                this->AddPendingEvent(wxCommandEvent(KnobDone));
            }
        }
    }

public:
    static int GetChangeAmount(tau::Point2d<int> position)
    {
        if (std::abs(position.x) > std::abs(position.y))
        {
            // X is larger than y, so use its value.
            return position.x;
        }

        // The top left corner is the origin, with y increasing
        // downwards. Scale the value 'up' when the user drags upwards.
        return -position.y;
    }

private:
    void UpdateMousePosition_(
        const tau::Point2d<int> &position,
        bool isFine,
        bool /* isDetented */)
    {
        int amount = GetChangeAmount(position - this->mousePosition_);
        this->mousePosition_ = position;

        double delta;

        if (isFine)
        {
            delta = static_cast<double>(amount) * this->fineStepSize_;
        }
        else
        {
            delta = static_cast<double>(amount) * this->stepSize_;
        }

        this->localValue_ += delta;

        if (this->continuous_)
        {
            double offsetValue = this->localValue_ + this->valueOffset_;

            offsetValue = std::fmod(
                offsetValue + this->valueRange_,
                this->valueRange_);

            this->localValue_ = offsetValue - this->valueOffset_;
        }

        if constexpr (std::is_integral_v<Type>)
        {
            this->value_.Set(
                static_cast<Type>(std::round(this->localValue_)));
        }
        else
        {
            this->value_.Set(static_cast<Type>(this->localValue_));
        }
    }

#ifdef _WIN32
    void OnMouseCaptureLost_(wxMouseCaptureLostEvent &)
    {
        this->hasCapturedMouse_ = false;
    }
#endif

private:
    ValueTerminus value_;

    pex::control::Signal<> reset_;

    double localValue_;
    LimitTerminus minimum_;
    LimitTerminus maximum_;
    KnobSettings settings_;
    double stepSize_;
    double fineStepSize_;
    double radius_;
    double startAngle_;
    double endAngle_;
    bool continuous_;
    double angleRange_;
    double valueRange_;
    double valueOffset_;
    bool hasCapturedMouse_;
    tau::Point2d<int> mousePosition_;
    wxColour color_;
    wxColour highlight_;
    wxColour outline_;
};


std::unique_ptr<wxBoxSizer> MakeSizer(
    const KnobSettings &knobSettings,
    wxWindow *knob,
    wxWindow *value);


template
<
    typename RangeControl,
    typename ValueControl,
    typename Convert
>
class ViewKnobConvert : public wxControl
{
public:
    using Base = wxControl;

    // range may be filtered to a type other than value.
    // value is the value from the model for display in the view.
    ViewKnobConvert(
        wxWindow *parent,
        RangeControl range,
        ValueControl value,
        const KnobSettings &knobSettings)
        :
        Base(parent, wxID_ANY)
    {
        this->knob_ = new Knob<RangeControl>(this, range, knobSettings);
        auto view = new View<ValueControl, Convert>(this, value);

        // Use a mono-spaced font for display so that the width of the view
        // remains constant as the value changes.
        view->SetFont(wxFont(wxFontInfo().Family(wxFONTFAMILY_MODERN)));

        this->SetSizerAndFit(
            MakeSizer(knobSettings, this->knob_, view).release());

        this->Bind(
            KnobDone,
            &ViewKnobConvert::OnKnobDone_,
            this);
    }

    // Constructors offering default options
    ViewKnobConvert(
        wxWindow *parent,
        RangeControl range,
        ValueControl value)
        :
        ViewKnobConvert(
            parent,
            range,
            value,
            KnobSettings())
    {

    }

    Knob<RangeControl> & GetKnob()
    {
        return *this->knob_;
    }


private:
    void OnKnobDone_(wxCommandEvent &event)
    {
        event.Skip();
        this->Layout();
    }

    Knob<RangeControl> *knob_;
};


template<typename RangeControl, typename ValueControl, int width, int precision>
using ViewKnobBase = ViewKnobConvert
    <
        RangeControl,
        ValueControl,
        Converter<ValueControl, width, precision>
    >;


template
<
    typename RangeControl,
    typename ValueControl,
    int width,
    int precision
>
class ViewKnob
    :
    public ViewKnobBase<RangeControl, ValueControl, width, precision>
{
    using Base =
        ViewKnobBase<RangeControl, ValueControl, width, precision>;

public:
    ViewKnob(
        wxWindow *parent,
        RangeControl range,
        ValueControl value,
        const KnobSettings &knobSettings)
        :
        Base(parent, range, value, knobSettings)
    {

    }

    ViewKnob(
        wxWindow *parent,
        RangeControl range,
        ValueControl value)
        :
        Base(parent, range, value)
    {

    }
};


template<typename RangeControl>
class SpinKnob: public wxControl
{
public:
    SpinKnob(
        wxWindow *parent,
        RangeControl range,
        const KnobSettings &knobSettings = KnobSettings())
        :
        wxControl(parent, wxID_ANY)
    {
        this->knob_ = new Knob<RangeControl>(this, range, knobSettings);

        auto spin =
            new SpinControl(this, range, 1, 0);

        this->SetSizerAndFit(
            MakeSizer(knobSettings, this->knob_, spin).release());

        this->Bind(
            KnobDone,
            &SpinKnob::OnKnobDone_,
            this);
    }

    Knob<RangeControl> & GetKnob()
    {
        return *this->knob_;
    }

private:
    void OnKnobDone_(wxCommandEvent &event)
    {
        event.Skip();
        this->Layout();
    }

    Knob<RangeControl> *knob_;
};


template
<
    typename RangeControl,
    typename ValueControl,
    typename Convert = pex::Converter<typename ValueControl::Type>
>
class FieldKnobConvert: public wxControl
{
public:
    using ValueField = Field<ValueControl, Convert>;

    FieldKnobConvert(
        wxWindow *parent,
        RangeControl range,
        ValueControl value,
        const KnobSettings &knobSettings = KnobSettings())
        :
        wxControl(parent, wxID_ANY)
    {
        this->knob_ = new Knob<RangeControl>(this, range, knobSettings);
        auto field = new ValueField(this, value);

        this->SetSizerAndFit(
            MakeSizer(knobSettings, this->knob_, field).release());

        this->Bind(
            KnobDone,
            &FieldKnobConvert::OnKnobDone_,
            this);
    }

    Knob<RangeControl> & GetKnob()
    {
        return *this->knob_;
    }

private:
    void OnKnobDone_(wxCommandEvent &event)
    {
        event.Skip();
        this->Layout();
    }

    Knob<RangeControl> *knob_;
};


template<typename RangeControl, typename ValueControl, int precision>
using FieldKnobBase = FieldKnobConvert
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
class FieldKnob
    :
    public FieldKnobBase<RangeControl, ValueControl, precision>
{
    using Base =
        FieldKnobBase<RangeControl, ValueControl, precision>;

public:
    FieldKnob(
        wxWindow *parent,
        RangeControl range,
        ValueControl value,
        const KnobSettings &knobSettings = KnobSettings())
        :
        Base(parent, range, value, knobSettings)
    {

    }
};


template<int precision, typename RangeControl, typename ValueControl>
auto CreateViewKnob(
    wxWindow *parent,
    RangeControl range,
    ValueControl value,
    const KnobSettings &knobSettings = KnobSettings())
{
    using Result = ViewKnob<RangeControl, ValueControl, -1, precision>;
    return new Result(parent, range, value, knobSettings);
}


template<int precision, typename RangeControl>
auto CreateViewKnob(
    wxWindow *parent,
    RangeControl range,
    const KnobSettings &knobSettings = KnobSettings())
{
    using Result =
        ViewKnob<RangeControl, decltype(RangeControl::value), -1, precision>;
    return new Result(parent, range, range.value, knobSettings);
}


template<int precision, typename RangeControl, typename ValueControl>
auto CreateFieldKnob(
    wxWindow *parent,
    RangeControl range,
    ValueControl value,
    const KnobSettings &knobSettings = KnobSettings())
{
    using Result = FieldKnob<RangeControl, ValueControl, precision>;
    return new Result(parent, range, value, knobSettings);
}


template<int precision, typename RangeControl>
auto CreateFieldKnob(
    wxWindow *parent,
    RangeControl range,
    const KnobSettings &knobSettings = KnobSettings())
{
    using Result =
        FieldKnob<RangeControl, decltype(RangeControl::value), precision>;

    return new Result(parent, range, range.value, knobSettings);
}




} // namespace wxpex
