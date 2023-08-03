#pragma once


#include <list>
#include <fields/fields.h>
#include "wxpex/color.h"
#include "wxpex/point.h"
#include "wxpex/graphics.h"


namespace polygon
{


template<typename T>
struct RecipeFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::sideCount, "sideCount"),
        fields::Field(&T::sideLength, "sideLength"),
        fields::Field(&T::position, "position"),
        fields::Field(&T::rotation_deg, "rotation_deg"));
};



template<template<typename> typename T>
struct RecipeTemplate
{
    T<pex::MakeRange<size_t, pex::Limit<3>, pex::Limit<1024>>> sideCount;
    T<double> sideLength;
    T<tau::Point2d<double> position;
    T<pex::MakeRange<double, pex::Limit<-180>, pex::Limit<180>> rotation_deg;
};


struct Recipe: public RecipeTemplate<pex::Identity>
{
    static Recipe Default()
    {
        return {{
            3,
            100,
            {{0, 0}},
            0}};
    }

    double GetWedgeAngle_rad()
    {
        return tau::Angles<double>::tau / double(this->sideCount);
    }

    double GetRadius(double halfWedgeAngle_rad) const
    {
        return this->sideLength / (2 * std::tan(halfWedgeAngle_rad));
    }
};


using RecipeGroup = pex::Group<RecipeFields, RecipeTemplate, Recipe>;
using RecipeControl = typename RecipeGroup::Control;


class Polygon
{
public:
    Polygon()
        :
        points_{}
    {

    }

    Polygon(const Recipe &recipe)
        :
        points_()
    {
        double wedgeAngle_rad = recipe.GetWedgeAngle_rad();
        double halfWedgeAngle_rad = wedgeAngle_rad / 2.0;
        double wedgeAngle_deg = tau::ToDegrees(wedgeAngle_rad);

        double radius = recipe.GetRadius(halfWedgeAngle_rad);
        double startAngle_deg = (wedgeAngle_deg / 2.0) + recipe.rotation_deg;
        tau::Vector2d<double> toStart{{radius, 0.0}};
        auto start = center + toStart.Rotate(startAngle_deg);
        this->points_.push_back(start);

        tau::Vector2d<double> side{{sideLength, 0.0}};
        side = side.Rotate(180 + recipe.rotation_deg);
        auto currentPoint = point;

        // Do not add the starting point twice
        sideCount -= 1;

        while (sideCount--)
        {
            currentPoint = currentPoint + side;
            this->points_.push_back(currentPoint);
            side.Rotate(wedgeAngle_deg);
        }
    }

    /*
     * @return true for points that fall inside or on the line.
     */
    bool Contains(const tau::Point2d<double> &point)
    {
        if (this->points_.size() < 2)
        {
            return false;
        }

        // Points are added clock-wise, so point will be on the right of every
        // segment if it is contained by the polygon.
        for (size_t start: jive::Range<size_t>(0, this->points_.size() - 1))
        {
            const auto &startPoint = this->points_[start];
            auto end = start + 1;

            auto segment =
                tau::Vector2d<double>(
                    this->points_[start],
                    this->points_[end]);

            auto test = tau::Vector2d<double>(startPoint, point);

            if (test.Cross(segment) < 0.0)
            {
                // point is on the left
                return false;
            }
        }

        // Check the last segment
        auto segment = tau::Vector2d<double>(
            this->points_.back(),
            this->points_.front());

        auto test = tau::Vector2d<double>(
            this->points_.back(),
            point);

        return test.Cross(segment) >= 0.0;
    }

    void CreatePath(wxGraphicsPath &path)
    {
        if (this->points_.empty())
        {
            return;
        }

        auto point = this->points_.begin();
        // auto start = *point;
        path.MoveToPoint(point->x, point->y);

        while (++point != this->points_.end())
        {
            path.AddLineToPoint(point->x, point->y);
        }

        // Close the path.
        // path.AddLineToPoint(start.x, start.y);
        path.CloseSubPath();
    }

private:
    std::vector<tau::Point2d<double>> points_;
};


template<typename T>
struct ColorFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::color, "color"),
        fields::Field(&T::compositionMode, "compositionMode"));
};


template<template<typename> typename T>
struct ColorTemplate
{
    T<tau::Rgba<uint8_t>> color;
    T<pex::MakeSelect<CompositionMode>> compositionMode;
};


using ColorGroup = pex::Group<ColorFields, ColorTemplate>;
using ColorControl = typename ColorGroup::Control;


template<typename T>
struct DrawFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::recipe, "recipe"),
        fields::Field(&T::color, "color"));
};


template<template<typename> typename T>
struct DrawTemplate
{
    T<pex::MakeGroup<RecipeGroup>> recipe;
    T<pex::MakeGroup<ColorGroup>> color;
};


using SettingsGroup = pex::Group<DrawFields, DrawTemplate>;
using SettingsModel = typename SettingsGroup::Model;
using SettingsControl = typename SettingsGroup::Control;


class DrawControl
{
public:
    DrawControl(const SettingsControl &settings)
        :
        recipe_(this, settings.recipe),
        color_(settings.color),
        polygon_(this->recipe_.control.Get())
    {
        this->recipe_.Connect(&DrawControl::OnRecipe_);
    }

    void Draw(GraphicsContext &graphics)
    {
        graphics->SetCompositionMode(this->color_.compositionMode.Get());
        graphics->SetBrush(wxBrush(ToWxColour(this->color_.color.Get())));
        auto path = graphics->CreatePath();
        this->polygon_.CreatePath(path);
        graphicsContext->FillPath(path);
    }

private:
    void OnRecipe_(const Recipe &recipe)
    {
        this->polygon_ = Polygon(recipe);
    }

private:
    pex::EndpointControl<Draw, RecipeControl> recipe_;
    ColorControl color_;
    Polygon polygon_;
};


class DrawModel
{
    RecipeModel recipe;
    ColorModel color;
    Polygon polygon;

publc:
    DrawModel()
        :
        recipe(Recipe::Default()),
        color(Color::Default()),
        polygon(this->recipe.Get()),
        recipeEndpoint_(this, this->recipe, &DrawModel::OnRecipe_)
    {

    }

    bool Contains(const tau::Point2d<double> &point)
    {
        return this->polygon_.Contains(point);
    }

private:
    void OnRecipe_(const Recipe &recipe)
    {
        this->polygon_ = Polygon(recipe);
    }

private:
    pex::Endpoint<DrawModel, RecipeControl> recipeEndpoint_;
};


class Stack
{
    using List = std::list<DrawModel> models_;
    using Iterator = typename List::iterator;

public:
    Stack()
        :
        models_()
    {

    }

    bool Select(const tau::Point2d<double> &point)
    {
        auto it = this->models_.begin();

        while (it != this->models_.end())
        {
            if (it->Contains(point))
            {
                this->selected_ = it;
                return true;
            }
        }

        return false;
    }

    bool DeleteSelected()
    {
        if (!this->selected_)
        {
            return false;
        }

        this->models_.erase(*this->selected_);

        return true;
    }

    void Add()
    {
        if (this->selected_)
        {
            this->models_.emplace(*this->selected_);

            // Select the newly-added model.
            --(*this->selected_);
        }
        else
        {
            // Place the new model on top.
            this->models_.emplace(*this->models_.begin());
            this->selected_ = this->models_.begin();
        }
    }

    std::optional<Iterator> GetSelected()
    {
        return this->selected_;
    }

    void RaiseSelected()
    {
        if (!this->selected_)
        {
            return;
        }

        if (*this->selected_ == this->models_.begin())
        {
            // Already at the top
            return;
        }

        auto previous = *this->selected_;
        --previous;

        // Use splice to move the selected iterator one element closer to the
        // front.
        this->models_.splice(previous, this->models_, this->selected_);
    }

    void LowerSelected()
    {
        if (!this->selected_)
        {
            return;
        }

        if (*this->selected_ == this->models_.rbegin())
        {
            // Already at the bottom
            return;
        }

        auto next = *this->selected_;
        ++next;

        // We should have already aborted if selected is the last item.
        assert(next != this->models_.end());
        ++next;

        this->models_.splice(next, this->models, this->selected_);
    }

private:
    std::list<DrawModel> models_;
    std::optional<Iterator> selected_;
};



} // end namespace polygon
