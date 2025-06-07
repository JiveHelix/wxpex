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


template<typename T>
struct WeaponsFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::firstFruit, "firstFruit"),
        fields::Field(&T::secondFruit, "secondFruit"),
        fields::Field(&T::notFruit, "notFruit"));
};


template<template<typename> typename T>
struct WeaponsTemplate
{
    T<std::string> firstFruit;
    T<std::string> secondFruit;
    T<std::string> notFruit;
};


using WeaponsGroup = pex::Group<WeaponsFields, WeaponsTemplate>;
using WeaponsPlain = typename WeaponsGroup::Plain;
using WeaponsModel = typename WeaponsGroup::Model;

using WeaponsControl = typename WeaponsGroup::Control;


inline WeaponsPlain DefaultWeapons()
{
    return {
        "passion fruit",
        "banana",
        "pointed stick"};
};


template<typename T>
struct ThingsFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::weapons, "weapons"));
};


template<template<typename> typename T>
struct ThingsTemplate
{
    T<WeaponsGroup> weapons;
};


using ThingsGroup = pex::Group<ThingsFields, ThingsTemplate>;
using ThingsControl = typename ThingsGroup::Control;


template<typename T>
struct StuffFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::thing1, "thing1"),
        fields::Field(&T::thing2, "thing2"));
};


template<template<typename> typename T>
struct StuffTemplate
{
    T<ThingsGroup> thing1;
    T<ThingsGroup> thing2;
};


using StuffGroup = pex::Group<StuffFields, StuffTemplate>;
using StuffControl = typename StuffGroup::Control;
using StuffModel = typename StuffGroup::Model;


template<template<typename, typename> typename Widget, typename Super>
class WeaponsWidget: public Super
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    WeaponsWidget(
        wxWindow *parent,
        const std::string &name,
        WeaponsControl control,
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
        WeaponsControl control,
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
    StuffView(wxWindow *parent, StuffControl control)
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
    StuffFrame(wxWindow *parent, StuffControl control)
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
    ExampleFrame(StuffControl control)
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
wxshimIMPLEMENT_APP_CONSOLE(ExampleApp)
