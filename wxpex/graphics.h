#pragma once


#include <wx/graphics.h>
#include <wx/dcgraph.h>
#include <fields/fields.h>
#include <jive/equal.h>
#include <jive/create_exception.h>
#include <tau/size.h>
#include <tau/scale.h>


namespace wxpex
{


CREATE_EXCEPTION(GraphicsError, std::runtime_error);


enum class AntialiasMode: int
{
    none = int(wxANTIALIAS_NONE),
    DEFAULT = int(wxANTIALIAS_DEFAULT)
};


enum class InterpolationQuality: int
{
    DEFAULT = wxINTERPOLATION_DEFAULT,
    none = wxINTERPOLATION_NONE,
    fast = wxINTERPOLATION_FAST,
    good = wxINTERPOLATION_GOOD,
    best = wxINTERPOLATION_BEST
};


enum class CompositionMode
{
    invalid = wxCOMPOSITION_INVALID,
    clear = wxCOMPOSITION_CLEAR,
    source = wxCOMPOSITION_SOURCE,
    over = wxCOMPOSITION_OVER,
    in = wxCOMPOSITION_IN,
    out = wxCOMPOSITION_OUT,
    atop = wxCOMPOSITION_ATOP,
    dest = wxCOMPOSITION_DEST,
    destOver = wxCOMPOSITION_DEST_OVER,
    destIn = wxCOMPOSITION_DEST_IN,
    destOut = wxCOMPOSITION_DEST_OUT,
    destAtop = wxCOMPOSITION_DEST_ATOP,
    XOR = wxCOMPOSITION_XOR,
    add = wxCOMPOSITION_ADD,
    diff = wxCOMPOSITION_DIFF
};


struct CompositionModeConverter
{
    static std::string ToString(CompositionMode compositionMode)
    {
        switch (compositionMode)
        {
            case (CompositionMode::invalid):
                return "invalid";

            case (CompositionMode::clear):
                return "clear";

            case (CompositionMode::source):
                return "source";

            case (CompositionMode::over):
                return "over";

            case (CompositionMode::in):
                return "in";

            case (CompositionMode::out):
                return "out";

            case (CompositionMode::atop):
                return "atop";

            case (CompositionMode::dest):
                return "dest";

            case (CompositionMode::destOver):
                return "dest over";

            case (CompositionMode::destIn):
                return "dest in";

            case (CompositionMode::destOut):
                return "dest out";

            case (CompositionMode::destAtop):
                return "dest atop";

            case (CompositionMode::XOR):
                return "XOR";

            case (CompositionMode::add):
                return "add";

            case (CompositionMode::diff):
                return "diff";

            default:
                throw std::logic_error("Unknown CompositionMode");
        }
    }
};


struct GraphicsMatrix
{
    GraphicsMatrix(const wxGraphicsMatrix &graphicsMatrix)
    {
        graphicsMatrix.Get(
            &this->a,
            &this->b,
            &this->c,
            &this->d,
            &this->tx,
            &this->ty);
    }

    void Convert(wxGraphicsMatrix &outGraphicsMatrix)
    {
        outGraphicsMatrix.Set(
            this->a,
            this->b,
            this->c,
            this->d,
            this->tx,
            this->ty);
    }

    tau::Scale<double> GetScale() const
    {
        // This assumes that rotation was applied last.
        // If scale was applied after rotation, you will not get what you
        // expect.
        return {
            std::sqrt(this->b * this->b + this->d * this->d),
            std::sqrt(this->a * this->a + this->c * this->c)};
    }

    double GetRotation() const
    {
        auto scale = this->GetScale();
        auto c_ = this->c / scale.horizontal;
        auto d_ = this->d / scale.vertical;
        return std::atan2(-c_, d_);
    }

    tau::Vector2d<double> GetTranslation() const
    {
        return {this->tx, this->ty};
    }

    bool IsIdentity() const
    {
        return (
            (jive::About(this->a) == 1.0)
            && (jive::About(this->b) == 0.0)
            && (jive::About(this->c) == 0.0)
            && (jive::About(this->d) == 1.0)
            && (jive::About(this->tx) == 0.0)
            && (jive::About(this->ty) == 0.0));
    }

    double a;
    double b;
    double c;
    double d;
    double tx;
    double ty;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&GraphicsMatrix::a, "a"),
        fields::Field(&GraphicsMatrix::b, "b"),
        fields::Field(&GraphicsMatrix::c, "c"),
        fields::Field(&GraphicsMatrix::d, "d"),
        fields::Field(&GraphicsMatrix::tx, "tx"),
        fields::Field(&GraphicsMatrix::ty, "ty"));
};


DECLARE_OUTPUT_STREAM_OPERATOR(GraphicsMatrix);


class GraphicsContext
{
public:
    template<typename T>
    GraphicsContext(T &&dc)
        :
        context_(wxGraphicsContext::Create(std::forward<T>(dc)))
    {

    }

    ~GraphicsContext()
    {
        if (this->context_)
        {
            this->context_->Flush();
            delete this->context_;
        }
    }

    GraphicsContext(const GraphicsContext &) = delete;
    GraphicsContext & operator=(const GraphicsContext &) = delete;

    wxGraphicsContext * operator->() { return this->context_; }

    operator bool () const
    {
        return !!this->context_;
    }

    operator wxGraphicsContext * ()
    {
        return this->context_;
    }

    tau::Size<double> GetSize() const
    {
        double width;
        double height;
        this->context_->GetSize(&width, &height);

        return {width, height};
    }

    tau::Vector2d<double> GetTranslation() const
    {
        return this->GetGraphicsMatrix().GetTranslation();
    }

    tau::Scale<double> GetScale() const
    {
        return this->GetGraphicsMatrix().GetScale();
    }

    double GetRotation() const
    {
        return this->GetGraphicsMatrix().GetRotation();
    }

    GraphicsMatrix GetGraphicsMatrix() const
    {
        return {this->context_->GetTransform()};
    }

private:
    wxGraphicsContext *context_;
};


class MaintainTransform
{
public:
    MaintainTransform(GraphicsContext &context)
        :
        context_(context),
        transform_(context->GetTransform())
    {

    }

    ~MaintainTransform()
    {
        this->context_->SetTransform(this->transform_);
    }

private:
    GraphicsContext &context_;
    wxGraphicsMatrix transform_;
};


} // end namespace wxpex
