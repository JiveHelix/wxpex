#include "graphics.h"


namespace wxpex
{


std::vector<PenCap> PenCapChoices::GetChoices()
{
    return {
        PenCap::round,
        PenCap::projecting,
        PenCap::butt};
}


std::string PenCapConverter::ToString(
    PenCap penCap)
{
    switch (penCap)
    {
        case (PenCap::invalid):
            return "invalid";

        case (PenCap::round):
            return "round";

        case (PenCap::projecting):
            return "projecting";

        case (PenCap::butt):
            return "butt";

        default:
            throw std::logic_error("Unknown PenCap");
    }
}


std::ostream & operator<<(std::ostream &output, PenCap value)
{
    return output << PenCapConverter::ToString(value);
}


std::vector<PenJoin> PenJoinChoices::GetChoices()
{
    return {
        PenJoin::round,
        PenJoin::bevel,
        PenJoin::miter};
}


std::string PenJoinConverter::ToString(PenJoin penJoin)
{
    switch (penJoin)
    {
        case (PenJoin::invalid):
            return "invalid";

        case (PenJoin::bevel):
            return "bevel";

        case (PenJoin::miter):
            return "miter";

        case (PenJoin::round):
            return "round";

        default:
            throw std::logic_error("Unknown PenJoin");
    }
}


std::ostream & operator<<(std::ostream &output, PenJoin value)
{
    return output << PenJoinConverter::ToString(value);
}


std::vector<PenStyle> PenStyleChoices::GetChoices()
{
    return {
        PenStyle::solid,
        PenStyle::dot,
        PenStyle::longDash,
        PenStyle::shortDash,
        PenStyle::dotDash,
        PenStyle::userDash,
        PenStyle::transparent,
        PenStyle::stippleMaskOpaque,
        PenStyle::stippleMask,
        PenStyle::stipple,
        PenStyle::backwardDiagonalHatch,
        PenStyle::crossDiagonalHatch,
        PenStyle::forwardDiagonalHatch,
        PenStyle::crossHatch,
        PenStyle::horizontalHatch,
        PenStyle::verticalHatch};
}


std::string PenStyleConverter::ToString(PenStyle penStyle)
{
    switch (penStyle)
    {
        case (PenStyle::invalid):
            return "invalid";

        case (PenStyle::solid):
            return "solid";

        case (PenStyle::dot):
            return "dot";

        case (PenStyle::longDash):
            return "longDash";

        case (PenStyle::shortDash):
            return "shortDash";

        case (PenStyle::dotDash):
            return "dotDash";

        case (PenStyle::userDash):
            return "userDash";

        case (PenStyle::transparent):
            return "transparent";

        case (PenStyle::stippleMaskOpaque):
            return "stippleMaskOpaque";

        case (PenStyle::stippleMask):
            return "stippleMask";

        case (PenStyle::stipple):
            return "stipple";

        case (PenStyle::backwardDiagonalHatch):
            return "backwardDiagonalHatch";

        case (PenStyle::crossDiagonalHatch):
            return "crossDiagonalHatch";

        case (PenStyle::forwardDiagonalHatch):
            return "forwardDiagonalHatch";

        case (PenStyle::crossHatch):
            return "crossHatch";

        case (PenStyle::horizontalHatch):
            return "horizontalHatch";

        case (PenStyle::verticalHatch):
            return "verticalHatch";

        default:
            throw std::logic_error("Unknown PenStyle");
    }
}


std::ostream & operator<<(std::ostream &output, PenStyle value)
{
    return output << PenStyleConverter::ToString(value);
}


std::vector<BrushStyle> BrushStyleChoices::GetChoices()
{
    return {
        BrushStyle::solid,
        BrushStyle::transparent,
        BrushStyle::stippleMaskOpaque,
        BrushStyle::stippleMask,
        BrushStyle::stipple,
        BrushStyle::backwardDiagonalHatch,
        BrushStyle::crossDiagonalHatch,
        BrushStyle::forwardDiagonalHatch,
        BrushStyle::crossHatch,
        BrushStyle::horizontalHatch,
        BrushStyle::verticalHatch};
}


std::string BrushStyleConverter::ToString(BrushStyle brushStyle)
{
    switch (brushStyle)
    {
        case (BrushStyle::invalid):
            return "invalid";

        case (BrushStyle::solid):
            return "solid";

        case (BrushStyle::transparent):
            return "transparent";

        case (BrushStyle::stippleMaskOpaque):
            return "stippleMaskOpaque";

        case (BrushStyle::stippleMask):
            return "stippleMask";

        case (BrushStyle::stipple):
            return "stipple";

        case (BrushStyle::backwardDiagonalHatch):
            return "backwardDiagonalHatch";

        case (BrushStyle::crossDiagonalHatch):
            return "crossDiagonalHatch";

        case (BrushStyle::forwardDiagonalHatch):
            return "forwardDiagonalHatch";

        case (BrushStyle::crossHatch):
            return "crossHatch";

        case (BrushStyle::horizontalHatch):
            return "horizontalHatch";

        case (BrushStyle::verticalHatch):
            return "verticalHatch";

        default:
            throw std::logic_error("Unknown BrushStyle");
    }
}


std::ostream & operator<<(std::ostream &output, BrushStyle value)
{
    return output << BrushStyleConverter::ToString(value);
}


std::vector<Interpolation> InterpolationChoices::GetChoices()
{
    return {
        Interpolation::DEFAULT,
        Interpolation::none,
        Interpolation::fast,
        Interpolation::good,
        Interpolation::best};
}


std::string InterpolationConverter::ToString(
    Interpolation interpolation)
{
    switch (interpolation)
    {
        case (Interpolation::DEFAULT):
            return "DEFAULT";

        case (Interpolation::none):
            return "none";

        case (Interpolation::fast):
            return "fast";

        case (Interpolation::good):
            return "good";

        case (Interpolation::best):
            return "best";

        default:
            throw std::logic_error("Unknown Interpolation");
    }
}


std::ostream & operator<<(std::ostream &output, Interpolation value)
{
    return output << InterpolationConverter::ToString(value);
}


std::vector<Composition> CompositionChoices::GetChoices()
{
    return {
        Composition::source,
        Composition::over,
        Composition::in,
        Composition::out,
        Composition::atop,
        Composition::dest,
        Composition::destOver,
        Composition::destIn,
        Composition::destOut,
        Composition::destAtop,
        Composition::XOR,
        Composition::add,
        Composition::diff};
}


std::string CompositionConverter::ToString(Composition compositionMode)
{
    switch (compositionMode)
    {
        case (Composition::invalid):
            return "invalid";

        case (Composition::clear):
            return "clear";

        case (Composition::source):
            return "source";

        case (Composition::over):
            return "over";

        case (Composition::in):
            return "in";

        case (Composition::out):
            return "out";

        case (Composition::atop):
            return "atop";

        case (Composition::dest):
            return "dest";

        case (Composition::destOver):
            return "dest over";

        case (Composition::destIn):
            return "dest in";

        case (Composition::destOut):
            return "dest out";

        case (Composition::destAtop):
            return "dest atop";

        case (Composition::XOR):
            return "XOR";

        case (Composition::add):
            return "add";

        case (Composition::diff):
            return "diff";

        default:
            throw std::logic_error("Unknown Composition");
    }
}


std::ostream & operator<<(std::ostream &output, Composition value)
{
    return output << CompositionConverter::ToString(value);
}


GraphicsContext::~GraphicsContext()
{
    if (this->gcdc_)
    {
        // The wxGCDC cleans up the context for us.
        return;
    }

    if (this->context_)
    {
        this->context_->Flush();
        delete this->context_;
    }
}


wxGCDC * GraphicsContext::GetGCDC()
{
    if (!this->gcdc_)
    {
        this->gcdc_.emplace(this->context_);
    }

    return &(*this->gcdc_);
}


tau::Size<double> GraphicsContext::GetSize() const
{
    double width;
    double height;
    this->context_->GetSize(&width, &height);

    return {width, height};
}

tau::Vector2d<double> GraphicsContext::GetTranslation() const
{
    return this->GetGraphicsMatrix().GetTranslation();
}

tau::Scale<double> GraphicsContext::GetScale() const
{
    return this->GetGraphicsMatrix().GetScale();
}

double GraphicsContext::GetRotation() const
{
    return this->GetGraphicsMatrix().GetRotation();
}

GraphicsMatrix GraphicsContext::GetGraphicsMatrix() const
{
    return {this->context_->GetTransform()};
}

void GraphicsContext::SetShear(const Shear<double> &shear)
{
    auto matrix = this->GetGraphicsMatrix();
    matrix.SetShear(shear);
    wxGraphicsMatrix result = this->context_->GetTransform();
    matrix.ToWxGraphicsMatrix(result);

    this->context_->SetTransform(result);
}

void GraphicsContext::SetComposition(Composition value)
{
    this->context_->SetCompositionMode(
        static_cast<wxCompositionMode>(value));
}

void GraphicsContext::SetInterpolation(Interpolation value)
{
    this->context_->SetInterpolationQuality(
        static_cast<wxInterpolationQuality>(value));
}

void GraphicsContext::SetAntialias(bool value)
{
    if (value)
    {
        this->context_->SetAntialiasMode(wxANTIALIAS_DEFAULT);
    }
    else
    {
        this->context_->SetAntialiasMode(wxANTIALIAS_NONE);
    }
}

Composition GraphicsContext::GetComposition() const
{
    return static_cast<Composition>(
        this->context_->GetCompositionMode());
}

Interpolation GraphicsContext::GetInterpolation() const
{
    return static_cast<Interpolation>(
        this->context_->GetInterpolationQuality());
}


bool GraphicsContext::GetAntialias() const
{
    return (this->context_->GetAntialiasMode() == wxANTIALIAS_DEFAULT);
}


} // end namespace wxpex
