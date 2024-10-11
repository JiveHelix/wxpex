#pragma once

#include <vector>
#include <optional>
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


#include "wxpex/combo_box.h"
#include "wxpex/labeled_widget.h"


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


enum class PenCap: int
{
    invalid = wxCAP_INVALID,
    round = wxCAP_ROUND,
    projecting = wxCAP_PROJECTING,
    butt = wxCAP_BUTT
};


struct PenCapChoices
{
    using Type = PenCap;
    static std::vector<PenCap> GetChoices();
};


using PenCapSelect = pex::MakeSelect<PenCapChoices>;
using PenCapModel = pex::ModelSelector<PenCapSelect>;
using PenCapControl = pex::ControlSelector<PenCapSelect>;


struct PenCapConverter
{
    static std::string ToString(PenCap compositionMode);
};


std::ostream & operator<<(std::ostream &, PenCap);


using PenCapComboBox = wxpex::ComboBox<PenCapControl, PenCapConverter>;


enum class PenStyle: int
{
    invalid = wxPENSTYLE_INVALID,
    solid = wxPENSTYLE_SOLID,
    dot = wxPENSTYLE_DOT,
    longDash = wxPENSTYLE_LONG_DASH,
    shortDash = wxPENSTYLE_SHORT_DASH,
    dotDash = wxPENSTYLE_DOT_DASH,
    userDash = wxPENSTYLE_USER_DASH,
    transparent = wxPENSTYLE_TRANSPARENT,
    stippleMaskOpaque = wxPENSTYLE_STIPPLE_MASK_OPAQUE,
    stippleMask = wxPENSTYLE_STIPPLE_MASK,
    stipple = wxPENSTYLE_STIPPLE,
    backwardDiagonalHatch = wxPENSTYLE_BDIAGONAL_HATCH,
    crossDiagonalHatch = wxPENSTYLE_CROSSDIAG_HATCH,
    forwardDiagonalHatch = wxPENSTYLE_FDIAGONAL_HATCH,
    crossHatch = wxPENSTYLE_CROSS_HATCH,
    horizontalHatch = wxPENSTYLE_HORIZONTAL_HATCH,
    verticalHatch = wxPENSTYLE_VERTICAL_HATCH
};


struct PenStyleChoices
{
    using Type = PenStyle;
    static std::vector<PenStyle> GetChoices();
};


using PenStyleSelect = pex::MakeSelect<PenStyleChoices>;
using PenStyleModel = pex::ModelSelector<PenStyleSelect>;
using PenStyleControl = pex::ControlSelector<PenStyleSelect>;


struct PenStyleConverter
{
    static std::string ToString(PenStyle compositionMode);
};


std::ostream & operator<<(std::ostream &, PenStyle);


using PenStyleComboBox = wxpex::ComboBox<PenStyleControl, PenStyleConverter>;


enum class PenJoin: int
{
    invalid = wxJOIN_INVALID,
    bevel = wxJOIN_BEVEL,
    miter = wxJOIN_MITER,
    round = wxJOIN_ROUND
};


struct PenJoinChoices
{
    using Type = PenJoin;
    static std::vector<PenJoin> GetChoices();
};


using PenJoinSelect = pex::MakeSelect<PenJoinChoices>;
using PenJoinModel = pex::ModelSelector<PenJoinSelect>;
using PenJoinControl = pex::ControlSelector<PenJoinSelect>;


struct PenJoinConverter
{
    static std::string ToString(PenJoin compositionMode);
};


std::ostream & operator<<(std::ostream &, PenJoin);


using PenJoinComboBox = wxpex::ComboBox<PenJoinControl, PenJoinConverter>;


enum class BrushStyle: int
{
    invalid = wxBRUSHSTYLE_INVALID,
    solid = wxBRUSHSTYLE_SOLID,
    transparent = wxBRUSHSTYLE_TRANSPARENT,
    stippleMaskOpaque = wxBRUSHSTYLE_STIPPLE_MASK_OPAQUE,
    stippleMask = wxBRUSHSTYLE_STIPPLE_MASK,
    stipple = wxBRUSHSTYLE_STIPPLE,
    backwardDiagonalHatch = wxBRUSHSTYLE_BDIAGONAL_HATCH,
    crossDiagonalHatch = wxBRUSHSTYLE_CROSSDIAG_HATCH,
    forwardDiagonalHatch = wxBRUSHSTYLE_FDIAGONAL_HATCH,
    crossHatch = wxBRUSHSTYLE_CROSS_HATCH,
    horizontalHatch = wxBRUSHSTYLE_HORIZONTAL_HATCH,
    verticalHatch = wxBRUSHSTYLE_VERTICAL_HATCH
};


struct BrushStyleChoices
{
    using Type = BrushStyle;
    static std::vector<BrushStyle> GetChoices();
};


using BrushStyleSelect = pex::MakeSelect<BrushStyleChoices>;
using BrushStyleModel = pex::ModelSelector<BrushStyleSelect>;
using BrushStyleControl = pex::ControlSelector<BrushStyleSelect>;


struct BrushStyleConverter
{
    static std::string ToString(BrushStyle compositionMode);
};


std::ostream & operator<<(std::ostream &, BrushStyle);


using BrushStyleComboBox =
    wxpex::ComboBox<BrushStyleControl, BrushStyleConverter>;


enum class Interpolation: int
{
    DEFAULT = wxINTERPOLATION_DEFAULT,
    none = wxINTERPOLATION_NONE,
    fast = wxINTERPOLATION_FAST,
    good = wxINTERPOLATION_GOOD,
    best = wxINTERPOLATION_BEST
};


struct InterpolationChoices
{
    using Type = Interpolation;
    static std::vector<Interpolation> GetChoices();
};


using InterpolationSelect = pex::MakeSelect<InterpolationChoices>;
using InterpolationModel = pex::ModelSelector<InterpolationSelect>;
using InterpolationControl = pex::ControlSelector<InterpolationSelect>;


struct InterpolationConverter
{
    static std::string ToString(Interpolation compositionMode);
};


std::ostream & operator<<(std::ostream &, Interpolation);



enum class Composition
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


struct CompositionChoices
{
    using Type = Composition;
    static std::vector<Composition> GetChoices();
};


using CompositionSelect = pex::MakeSelect<CompositionChoices>;
using CompositionModel = pex::ModelSelector<CompositionSelect>;
using CompositionControl = pex::ControlSelector<CompositionSelect>;


struct CompositionConverter
{
    static std::string ToString(Composition compositionMode);
};


std::ostream & operator<<(std::ostream &, Composition);


using CompositionComboBox =
    wxpex::ComboBox<CompositionControl, CompositionConverter>;


template<typename U, typename ...Ts>
concept IsOneOf = std::disjunction_v<std::is_same<U, Ts>...>;


template<typename Control>
auto CreateGraphicsControl(wxWindow *parent, Control control)
{
    static_assert(
        IsOneOf
        <
            Control,
            PenStyleControl,
            PenCapControl,
            PenJoinControl,
            BrushStyleControl,
            CompositionControl
        >,
        "Unsupported control type");

    if constexpr (std::is_same_v<Control, PenStyleControl>)
    {
        return wxpex::LabeledWidget(
            parent,
            "Pen Style",
            new wxpex::PenStyleComboBox(parent, control));
    }
    else if constexpr (std::is_same_v<Control, PenCapControl>)
    {
        return wxpex::LabeledWidget(
            parent,
            "Pen Cap",
            new wxpex::PenCapComboBox(parent, control));
    }
    else if constexpr (std::is_same_v<Control, PenJoinControl>)
    {
        return wxpex::LabeledWidget(
            parent,
            "Pen Join",
            new wxpex::PenJoinComboBox(parent, control));

    }
    else if constexpr (std::is_same_v<Control, BrushStyleControl>)
    {
        return wxpex::LabeledWidget(
            parent,
            "Brush Style",
            new wxpex::BrushStyleComboBox(parent, control));
    }
    else if constexpr (std::is_same_v<Control, CompositionControl>)
    {
        return wxpex::LabeledWidget(
            parent,
            "Composition",
            new wxpex::CompositionComboBox(parent, control));
    }
    else
    {
        throw std::logic_error("static_asert failed to detect anomaly");
    }
}


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

    void SetShear(const Shear<double> &shear)
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

    ~GraphicsContext();

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

    wxGCDC * GetGCDC();

    tau::Size<double> GetSize() const;

    tau::Vector2d<double> GetTranslation() const;

    tau::Scale<double> GetScale() const;

    double GetRotation() const;

    GraphicsMatrix GetGraphicsMatrix() const;

    void SetShear(const Shear<double> &shear);

    void SetComposition(Composition value);

    void SetInterpolation(Interpolation value);

    void SetAntialias(bool value);

    Composition GetComposition() const;

    Interpolation GetInterpolation() const;

    bool GetAntialias() const;

protected:
    wxGraphicsContext *context_;
    std::optional<wxGCDC> gcdc_;
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
