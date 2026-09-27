#include <iostream>
#include <string>
#include <fields/fields.h>

#include <pex/pex.h>

#include <wxpex/field.h>
#include <wxpex/view.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/layout_items.h>
#include <wxpex/collapsible.h>
#include <wxpex/static_box.h>
#include <wxpex/scrolled.h>
#include <wxpex/widget_names.h>
#include <wxpex/border_sizer.h>

#include <wxpex/wxshim_app.h>


template<template<typename> typename T>
struct WeaponsSchema
{
    T<std::string> firstFruit;
    T<std::string> secondFruit;
    T<std::string> notFruit;
};


using WeaponsGroup = pex::Group<WeaponsSchema>;
using WeaponsPlain = typename WeaponsGroup::Plain;
using WeaponsModel = typename WeaponsGroup::Model;

using WeaponsControl = typename WeaponsGroup::DefaultControl;


inline WeaponsPlain DefaultWeapons()
{
    return {
        "passion fruit",
        "banana",
        "pointed stick"};
};


template<template<typename> typename T>
struct ThingsSchema
{
    T<WeaponsGroup> weapons;
};


using ThingsGroup = pex::Group<ThingsSchema>;
using ThingsModel = typename ThingsGroup::Model;
using ThingsControl = typename ThingsGroup::template Control<ThingsModel>;
using ThingsPlain = typename ThingsGroup::Plain;


using ModelSchema = ThingsSchema<pex::ModelTailor>;
static_assert(std::same_as<ModelSchema, fields::ReflectorType<ThingsModel>>);

using PlainSchema = ThingsSchema<pex::Identity>;
static_assert(std::same_as<PlainSchema, fields::ReflectorType<ThingsPlain>>);

static_assert(fields::GetMemberCount<ModelSchema>() == 1);

static_assert(fields::GetMemberCount<PlainSchema>() == 1);

static_assert(
        fields::GetMemberCount<fields::ReflectorType<ThingsPlain>>() == 1);

static_assert(fields::Reflect<ThingsModel>::count == 1);
static_assert(fields::Reflect<ThingsPlain>::count == 1);


template<template<typename> typename T>
struct StuffSchema
{
    T<ThingsGroup> thing1;
    T<ThingsGroup> thing2;
};


using StuffGroup = pex::Group<StuffSchema>;
using StuffControl = typename StuffGroup::DefaultControl;
using StuffModel = typename StuffGroup::Model;


template<template<typename, typename> typename Widget, typename Super>
class WeaponsWidget: public Super
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    WeaponsWidget(
        wxWindow *parent,
        const std::string &name,
        const WeaponsControl &control,
        const LayoutOptions &layoutOptions)
        :
        Super(parent, name)
    {
        using namespace wxpex;

        auto pane = this->GetPanel();

        auto firstFruit = LabeledWidget(
            pane,
            "firstFruit",
            MakeWidget<Widget>("firstFruit", pane, control.firstFruit));

        auto secondFruit = LabeledWidget(
            pane,
            "secondFruit",
            MakeWidget<Widget>("secondFruit", pane, control.secondFruit));

        auto notFruit = LabeledWidget(
            pane,
            "notFruit",
            MakeWidget<Widget>("notFruit", pane, control.notFruit));

        auto sizer = LayoutLabeled(
            layoutOptions,
            firstFruit,
            secondFruit,
            notFruit);

        this->ConfigureSizer(std::move(sizer));
    }
};


using WeaponsView = WeaponsWidget<wxpex::View, wxpex::StaticBox>;
using WeaponsEntry = WeaponsWidget<wxpex::Field, wxpex::Collapsible>;


class WeaponsViews: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    WeaponsViews(
        wxWindow *parent,
        const WeaponsControl &control,
        const LayoutOptions &layoutOptions)
        :
        wxpex::Collapsible(parent, "Weapons")
    {
        auto panel = this->GetPanel();

        auto weaponsView =
            new WeaponsView(
                panel,
                "Weapons View",
                control,
                layoutOptions);

        auto weaponsEntry =
            new WeaponsEntry(
                panel,
                "Weapons Entry",
                control,
                layoutOptions);

        wxpex::RegisterWidgetName(weaponsView, "weaponsView");
        wxpex::RegisterWidgetName(weaponsEntry, "weaponsEntry");

        auto sizer = wxpex::LayoutItems(
            wxpex::verticalItems,
            weaponsView,
            weaponsEntry);

        this->ConfigureSizer(std::move(sizer));
    }
};


class StuffView: public wxPanel
{
public:
    StuffView(wxWindow *parent, const StuffControl &control)
        :
        wxPanel(parent, wxID_ANY)
    {
        auto weapons1 =
            new WeaponsViews(
                this,
                control.thing1.weapons,
                wxpex::LayoutOptions{});

        wxpex::RegisterWidgetName(weapons1, "weapons1");

        auto sizer = wxpex::LayoutItems(
            wxpex::verticalItems,
            weapons1);

        this->SetSizer(sizer.release());
    }
};


class StuffFrame: public wxpex::Scrolled
{
public:
    StuffFrame(wxWindow *parent, const StuffControl &control)
        :
        wxpex::Scrolled(parent)
    {
        auto stuffView = new StuffView(this, control);

        wxpex::RegisterWidgetName(stuffView, "stuffView");

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        sizer->Add(stuffView, 1, wxEXPAND);
        this->ConfigureSizer(wxpex::verticalScrolled, std::move(sizer));
    }
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(const StuffControl &control)
        :
        wxFrame(nullptr, wxID_ANY, "Settings Demo")
    {
        auto stuffFrame = new StuffFrame(this, control);

        wxpex::RegisterWidgetName(stuffFrame, "stuffFrame");

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        sizer->Add(stuffFrame, 1, wxEXPAND);
        this->SetSizer(sizer.release());
    }
};


class ExampleApp: public wxApp
{
public:
    ExampleApp()
        :
        stuff_{}
    {

    }

    bool OnInit() override
    {
        ExampleFrame *exampleFrame =
            new ExampleFrame(StuffControl(this->stuff_));

        wxpex::RegisterWidgetName(exampleFrame, "exampleFrame");

        exampleFrame->Show();

        return true;
    }

private:
    StuffModel stuff_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimAPP(ExampleApp)
