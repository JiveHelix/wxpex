#pragma once

#include <vector>
#include <fields/fields.h>
#include <jive/equal.h>
#include <jive/create_exception.h>
#include <tau/size.h>
#include <tau/scale.h>

#include "wxpex/ignores.h"

WXSHIM_PUSH_IGNORES
#include <wx/graphics.h>
#include <wx/dcgraph.h>
WXSHIM_POP_IGNORES


namespace wxpex
{


CREATE_EXCEPTION(GraphicsError, std::runtime_error);


template<typename T>
struct ShearFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::x, "x"),
        fields::Field(&T::y, "y"));
};


template<typename U>
struct ShearTemplate
{
    using ShearRange = pex::MakeRange<U, pex::Limit<-1>, pex::Limit<1>>;

    template<template<typename> typename T>
    struct Template
    {
        T<ShearRange> x;
        T<ShearRange> y;

        static constexpr auto fields = ShearFields<Template>::fields;
        static constexpr auto fieldsTypeName = "Shear";
    };
};


template<typename U>
struct Shear: public ShearTemplate<U>::template Template<pex::Identity>
{
    using Matrix = Eigen::Matrix<double, 2, 2>;

    Matrix GetMatrix() const
    {
        Matrix result = Matrix::Identity();
        result(0, 1) = this->x;
        result(1, 0) = this->y;

        return result;
    }
};


template<typename U>
using ShearGroup =
    pex::Group
    <
        ShearFields,
        ShearTemplate<U>::template Template,
        pex::PlainT<Shear<U>>
    >;

template<typename U>
using ShearModel = typename ShearGroup<U>::Model;

template<typename U>
using ShearControl = typename ShearGroup<U>::Control;


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


std::vector<InterpolationQuality> GetInterpolationQualities();


struct InterpolationQualityConverter
{
    static std::string ToString(InterpolationQuality interpolationQuality);
};


std::ostream & operator<<(std::ostream &, InterpolationQuality);


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


struct CompositionModeChoices
{
    using Type = CompositionMode;
    static std::vector<CompositionMode> GetChoices();
};


using CompositionModeSelect = pex::MakeSelect<CompositionModeChoices>;
using CompositionModeModel = pex::ModelSelector<CompositionModeSelect>;
using CompositionModeControl = pex::ControlSelector<CompositionModeSelect>;


struct CompositionModeConverter
{
    static std::string ToString(CompositionMode compositionMode);
};


std::ostream & operator<<(std::ostream &, CompositionMode);


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

    void ToWxGraphicsMatrix(wxGraphicsMatrix &outGraphicsMatrix)
    {
        outGraphicsMatrix.Set(
            this->a,
            this->b,
            this->c,
            this->d,
            this->tx,
            this->ty);
    }

    using Matrix = Eigen::Matrix<double, 3, 3>;
    using RotationMatrix = Eigen::Matrix<double, 2, 2>;

    Matrix GetMatrix() const
    {
        Matrix result = Matrix::Zero();
        result(0, 0) = this->a;
        result(0, 1) = this->b;
        result(0, 2) = this->tx;
        result(1, 0) = this->c;
        result(1, 1) = this->d;
        result(1, 2) = this->ty;

        result(2, 0) = 0.0;
        result(2, 1) = 0.0;
        result(2, 2) = 1.0;

        return result;
    }

    void SetMatrix(const Matrix &matrix)
    {
        this->a = matrix(0, 0);
        this->b = matrix(0, 1);
        this->c = matrix(1, 0);
        this->d = matrix(1, 1);
        this->tx = matrix(0, 2);
        this->ty = matrix(1, 2);
    }

    RotationMatrix GetRotationMatrix() const
    {
        RotationMatrix result = RotationMatrix::Zero();
        result(0, 0) = this->a;
        result(0, 1) = this->b;
        result(1, 0) = this->c;
        result(1, 1) = this->d;

        return result;
    }

    void SetRotationMatrix(const RotationMatrix &matrix)
    {
        this->a = matrix(0, 0);
        this->b = matrix(0, 1);
        this->c = matrix(1, 0);
        this->d = matrix(1, 1);
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

    Shear<double> GetShear() const
    {
        return {this->b, this->c};
    }

    void Shear(const Shear<double> &shear)
    {
        this->SetRotationMatrix(shear.GetMatrix() * this->GetRotationMatrix());
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

    wxGraphicsContext * Release()
    {
        wxGraphicsContext *result = this->context_;
        this->context_ = nullptr;

        return result;
    }

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

    void Shear(const Shear<double> &shear)
    {
        auto matrix = this->GetGraphicsMatrix();
        matrix.Shear(shear);
        wxGraphicsMatrix result = this->context_->GetTransform();
        matrix.ToWxGraphicsMatrix(result);

        this->context_->SetTransform(result);
    }

    void SetCompositionMode(CompositionMode value)
    {
        this->context_->SetCompositionMode(
            static_cast<wxCompositionMode>(value));
    }

    void SetInterpolationQuality(InterpolationQuality value)
    {
        this->context_->SetInterpolationQuality(
            static_cast<wxInterpolationQuality>(value));
    }

    CompositionMode GetCompositionMode() const
    {
        return static_cast<CompositionMode>(
            this->context_->GetCompositionMode());
    }

    InterpolationQuality GetInterpolationQuality() const
    {
        return static_cast<InterpolationQuality>(
            this->context_->GetInterpolationQuality());
    }

protected:
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
