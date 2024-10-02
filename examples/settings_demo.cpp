#include <iostream>
#include <string>
#include <fields/fields.h>

#include <pex/pex.h>

#include <wxpex/field.h>
#include <wxpex/view.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include <wxpex/static_box.h>
#include <wxpex/scrolled.h>
#include <wxpex/widget_names.h>


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
struct GpsFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::time, "time"),
        fields::Field(&T::latitude, "latitude"),
        fields::Field(&T::longitude, "longitude"),
        fields::Field(&T::elevation, "elevation"));
};


template<template<typename> typename T>
struct GpsTemplate
{
    T<int64_t> time;
    T<double> latitude;
    T<double> longitude;
    T<double> elevation;

    static constexpr auto fields = GpsFields<GpsTemplate>::fields;
};


using GpsGroup = pex::Group<GpsFields, GpsTemplate>;
using GpsPlain = typename GpsGroup::Plain;
using GpsModel = typename GpsGroup::Model;

using GpsControl = typename GpsGroup::Control;


inline GpsPlain DefaultGps()
{
    return {
        1334706453,
        40.56923581063791,
        -111.63928609736942,
        3322.0};
}


template<typename T>
struct ThingsFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::weapons, "weapons"),
        fields::Field(&T::gps, "gps"));
};


template<template<typename> typename T>
struct ThingsTemplate
{
    T<WeaponsGroup> weapons;
    T<GpsGroup> gps;
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


template<template<typename, typename> typename Widget>
class WeaponsWidget: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    WeaponsWidget(
        wxWindow *parent,
        const std::string &name,
        WeaponsControl control,
        const LayoutOptions &layoutOptions)
        :
        wxpex::Collapsible(parent, name, wxBORDER_SIMPLE)
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

        this->ConfigureTopSizer(std::move(sizer));
    }
};


using WeaponsView = WeaponsWidget<wxpex::View>;
using WeaponsEntry = WeaponsWidget<wxpex::Field>;


template<template<typename, typename> typename Widget>
class GpsWidget: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    GpsWidget(
        wxWindow *parent,
        const std::string &name,
        GpsControl control,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxpex::Collapsible(parent, name)
    {
        using namespace wxpex;

        auto panel = this->GetPanel();

        auto time = LabeledWidget(
            panel,
            "time",
            MakeWidget<Widget>("time", panel, control.time));

        auto latitude = LabeledWidget(
            panel,
            "latitude",
            MakeWidget<Widget>("latitude", panel, control.latitude));

        auto longitude = LabeledWidget(
            panel,
            "longitude",
            MakeWidget<Widget>("longitude", panel, control.longitude));

        auto elevation = LabeledWidget(
            panel,
            "elevation",
            MakeWidget<Widget>("elevation", panel, control.elevation));

        auto sizer = LayoutLabeled(
            layoutOptions,
            time,
            latitude,
            longitude,
            elevation);

        this->ConfigureTopSizer(std::move(sizer));
    }
};


using GpsView = GpsWidget<wxpex::View>;
using GpsEntry = GpsWidget<wxpex::Field>;


class GpsViews: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    GpsViews(
        wxWindow *parent,
        GpsControl gps,
        const LayoutOptions &layoutOptions)
        :
        wxpex::Collapsible(parent, "GPS")
    {
        auto panel = this->GetPanel();

        auto gpsView =
            new GpsWidget<wxpex::View>(
                panel,
                "GPS View",
                gps,
                layoutOptions);

        auto gpsEntry =
            new GpsWidget<wxpex::Field>(
                panel,
                "GPS Entry",
                gps,
                layoutOptions);

        wxpex::RegisterWidgetName(gpsView, "gpsView");
        wxpex::RegisterWidgetName(gpsEntry, "gpsEntry");

        this->ConfigureTopSizer(
            wxpex::LayoutItems(wxpex::verticalItems, gpsView, gpsEntry));
    }
};


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

        this->ConfigureTopSizer(std::move(sizer));
    }
};


class ThingsView
    :
    public wxpex::StaticBox
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    ThingsView(
        wxWindow *parent,
        ThingsControl control,
        [[maybe_unused]] const std::string &name,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxpex::StaticBox(parent, name)
    {
        auto panel = this->GetPanel();

        // wxStaticBoxSizer *sizer = new wxStaticBoxSizer(wxVERTICAL, this, name);

        auto weapons = new WeaponsViews(
            panel,
            control.weapons,
            layoutOptions);

        auto gps = new GpsViews(
            panel,
            control.gps,
            layoutOptions);

        wxpex::RegisterWidgetName(weapons, "weapons");
        wxpex::RegisterWidgetName(gps, "gps");

        this->ConfigureSizer(
            wxpex::LayoutItems(wxpex::verticalItems, weapons, gps));
    }
};


class StuffView: public wxpex::Scrolled
{
public:
    StuffView(wxWindow *parent, StuffControl control)
        :
        wxpex::Scrolled(parent)
    {
        auto thing1 = new ThingsView(this, control.thing1, "Thing 1");
        auto thing2 = new ThingsView(this, control.thing2, "Thing 2");

        wxpex::RegisterWidgetName(thing1, "thing1");
        wxpex::RegisterWidgetName(thing2, "thing2");

        auto sizer = wxpex::LayoutItems(
            wxpex::verticalItems,
            thing1,
            thing2);

        this->ConfigureTopSizer(
            wxpex::verticalScrolled,
            std::move(sizer));
    }
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(StuffControl control)
        :
        wxFrame(nullptr, wxID_ANY, "Settings Demo")
    {
        auto stuffView = new StuffView(this, control);

        wxpex::RegisterWidgetName(stuffView, "stuffView");

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        sizer->Add(stuffView, 1, wxALL | wxEXPAND, 10);
        // this->SetSizerAndFit(sizer.release());
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
