#pragma once

#include <pex/range.h>
#include <pex/converter.h>

#include "wxpex/wxshim.h"
#include "wxpex/size.h"


namespace wxpex
{


template<typename Range>
class Knob: public wxControl
{
    static const double stepSizeDivisor_ = 100.0;
    static const double fineStepFactor_ = 0.01;

public:
    using Base = wxControl;
    using This = Knob<Range>;

    // Value and Limit are observed by This
    using Value =
        pex::Terminus<This, typename Range::Value>;

    using Limit =
        pex::Terminus<This, typename Range::Limit>;

    using Type = typename Value::Type;

    /**
     ** We use knobs in a clockwise-is-increasing sense, so we'll specify knob
     ** angles in the same way, with 0 degrees strait up, and increasing angle
     ** rotating to the right.
     **/

    static const unsigned defaultRadius = 15;
    static const double defaultStartAngle = -150.0;
    static const double defaultEndAngle = 150.0;

    Knob(
        wxWindow *parent,
        Range control,
        unsigned radius = defaultRadius,
        double startAngle = defaultStartAngle,
        double endAngle = defaultEndAngle)
        :
        Base(parent, wxID_ANY),
        value_(this, control.value),
        localValue_(static_cast<double>(this->value_.Get()),
        minimum_(this, control.minimum),
        maximum_(this, control.maximum),
        stepSize_(GetStepSize_(this->minimum_.Get(), this->maximum_.Get())),
        fineStepSize_(this->stepSize_ * fineStepFactor_),
        radius_(radius),
        startAngle_(startAngle),
        endAngle_(endAngle),
        hasCapturedMouse_(false),
        mousePosition_{}
    {
        this->value_.Connect(&Knob::OnValue_);
        this->minimum_.Connect(&Knob::OnMinimum_);
        this->maximum_.Connect(&Knob::OnMaximum_);

        this->Bind(wxEVT_PAINT, &Knob::OnPaint_);
        this->Bind(wxEVT_MOUSE_EVENTS, &Knob::OnMouseEvents_);

#ifdef _WIN32
        this->Bind(wxEVT_MOUST_CAPTURE_LOST, &Knob::OnMouseCaptureLost_);
#endif
    }

    wxSize DoGetBestSize() const override
    {
        auto side = static_cast<int>(this->radius_ * 2 + 2);
        return wxSize(side, side);
    }

private:
    template<typename T>
    static double GetStepSize_(T minimum, T maximum) const
    {
        return static_cast<double>(maximum - minimum) / stepSizeDivisor_;
    }

    void OnValue_(Type value)
    {
        this->localValue_ = static_cast<double>(value);
        this->Refresh();
    }

    void OnMinimum_(Type minimum)
    {
        this->stepSize_ = GetStepSize_(minimum, this->maximum_.Get());
        this->fineStepSize_ = this->stepSize_ * this->fineStepFactor_;
        this->Refresh();
    }

    void OnMaximum_(Type maximum)
    {
        this->stepSize_ = GetStepSize_(this->minimum_.Get(), maximum);
        this->fineStepSize_ = this->stepSize_ * this->fineStepFactor_;
        this->Refresh();
    }

    void OnPaint_(wxPaintEvent &)
    {
        auto size = this->GetClientSize();
        auto width = size.GetWidth();
        auto height = size.GetHeight();

        auto center = wxPoint
        wxPaintDC dc(this);
        
    }

    void OnMouseEvents_(wxMouseEvent &mouseEvent)
    {
        if (mouseEvent.LeftDown())
        {
            if (this->hasCapturedMouse_)
            {
                return;
            }

            this->CaptureMouse();
            this->hasCapturedMouse_ = true;
            this->mousePosition_ = mouseEvent.GetPosition();
        }
        else if (mouseEvent.LeftIsDown())
        {
            if (this->hasCapturedMouse_)
            {
                this->UpdateMousePosition_(
                    event.GetPosition(),
                    event.AltDown(),
                    event.ControlDown());
            }
        }
        else if (mouseEvent.LeftUp())
        {
            if (this->hasCapturedMouse_)
            {
                this->ReleaseMouse();
                this->hasCapturedMouse_ = false;
            }
        }
    }

public:
    static int GetChangeAmount(wxPoint position)
    {
        if (std::abs(position.x) > std::abs(position.y))
        {
            // X is larger than y, so use its value.
            return position.x;
        }

        // wx uses the top left corner as the origin, with y increasing
        // downwards. Scale the value 'up' when the user drags upwards.
        return -position.y;
    }

private:
    void UpdateMousePosition_(
        wxPoint position,
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
    void OnMoustCaptureLost_(wxMouseCaptureLostEvent &)
    {
        this->hasCapturedMouse_ = false;
    }
#endif

private:
    Value value_;
    double localValue_;
    Limit minimum_;
    Limit maximum_;
    double stepSize_;
    double fineStepSize_;
    unsigned radius_;
    double startAngle_;
    double endAngle_;
    bool hasCapturedMouse_;
    wxPosition mousePosition_;
};


} // namespace wxpex
