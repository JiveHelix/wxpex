#include <iostream>
#include <string>
#include <fields/fields.h>

#include <pex/pex.h>

#include <wxpex/field.h>
#include <wxpex/view.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/collapsible.h>
#include <wxpex/border_sizer.h>
#include <wx/statbox.h>


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
    return {{
        "passion fruit",
        "banana",
        "pointed stick"}};
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
    return {{
        1334706453,
        40.56923581063791,
        -111.63928609736942,
        3322.0}};
}


template<typename T>
struct DataFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::weapons, "weapons"),
        fields::Field(&T::gps, "gps"));
};


struct Data
{
    WeaponsPlain weapons;
    GpsPlain gps;

    static constexpr auto fields = DataFields<Data>::fields;

    static Data Default()
    {
        return {DefaultWeapons(), DefaultGps()};
    }
};


struct DataModel
{
    WeaponsModel weapons;
    GpsModel gps;

    DataModel()
    {
        auto defaultData = Data::Default();
        pex::Assign<DataFields>(*this, defaultData);
    }
};


struct DataControls
{
    WeaponsControl weapons;
    GpsControl gps;

    DataControls(DataModel &dataModel)
    {
        fields::AssignConvert<DataFields>(*this, dataModel);
    }
};


template<template<typename, typename> typename Widget>
class WeaponsWidget: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    WeaponsWidget(
        wxWindow *parent,
        const std::string &name,
        typename WeaponsGroup::Control controls,
        const LayoutOptions &layoutOptions)
        :
        wxpex::Collapsible(parent, name)
    {
        using namespace wxpex;

        auto pane = this->GetBorderPane();

        auto firstFruit = LabeledWidget(
            pane,
            "firstFruit",
            new Widget(pane, controls.firstFruit));

        auto secondFruit = LabeledWidget(
            pane,
            "secondFruit",
            new Widget(pane, controls.secondFruit));

        auto notFruit = LabeledWidget(
            pane,
            "notFruit",
            new Widget(pane, controls.notFruit));

        auto sizer = LayoutLabeled(
            layoutOptions,
            firstFruit,
            secondFruit,
            notFruit);

        this->ConfigureBorderPane(5, std::move(sizer));
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
        GpsControl controls,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxpex::Collapsible(parent, name)
    {
        using namespace wxpex;

        auto pane = this->GetBorderPane();

        auto time = LabeledWidget(
            pane,
            "time",
            new Widget(pane, controls.time));

        auto latitude = LabeledWidget(
            pane,
            "latitude",
            new Widget(pane, controls.latitude));

        auto longitude = LabeledWidget(
            pane,
            "longitude",
            new Widget(pane, controls.longitude));

        auto elevation = LabeledWidget(
            pane,
            "elevation",
            new Widget(pane, controls.elevation));

        auto sizer = LayoutLabeled(
            layoutOptions,
            time,
            latitude,
            longitude,
            elevation);

        this->ConfigureBorderPane(5, std::move(sizer));
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
        DataControls controls,
        const LayoutOptions &layoutOptions)
        :
        wxpex::Collapsible(parent, "GPS")
    {
        auto borderPane = this->GetBorderPane();

        auto gpsView =
            new GpsWidget<wxpex::View>(
                borderPane,
                "GPS View",
                controls.gps,
                layoutOptions);

        auto gpsEntry =
            new GpsWidget<wxpex::Field>(
                borderPane,
                "GPS Entry",
                controls.gps,
                layoutOptions);

        this->ConfigureBorderPane(
            5,
            wxpex::LayoutItems(wxpex::verticalItems, gpsView, gpsEntry));
    }
};


class WeaponsViews: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    WeaponsViews(
        wxWindow *parent,
        DataControls controls,
        const LayoutOptions &layoutOptions)
        :
        wxpex::Collapsible(parent, "Weapons")
    {
        auto borderPane = this->GetBorderPane();

        auto weaponsView =
            new WeaponsView(
                borderPane,
                "Weapons View",
                controls.weapons,
                layoutOptions);

        auto weaponsEntry =
            new WeaponsEntry(
                borderPane,
                "Weapons Entry",
                controls.weapons,
                layoutOptions);

        auto sizer = wxpex::LayoutItems(
            wxpex::verticalItems,
            weaponsView,
            weaponsEntry);

        this->ConfigureBorderPane(5, std::move(sizer));
    }
};


class DataView: public wxPanel
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    DataView(
        wxWindow *parent,
        DataControls controls,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxPanel(parent, wxID_ANY)
    {

        auto weapons = new WeaponsViews(this, controls, layoutOptions);
        auto gps = new GpsViews(this, controls, layoutOptions);

        this->SetSizerAndFit(
            wxpex::LayoutItems(wxpex::verticalItems, weapons, gps).release());
    }
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(DataControls controls)
        :
        wxFrame(nullptr, wxID_ANY, "Settings Demo")
    {
        auto dataView = new DataView(this, controls);
        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        sizer->Add(dataView, 1, wxALL | wxEXPAND, 10);
        this->SetSizerAndFit(sizer.release());
    }
};


class ExampleApp: public wxApp
{
public:
    ExampleApp()
        :
        data_{}
    {

    }

    bool OnInit() override
    {
        ExampleFrame *exampleFrame =
            new ExampleFrame(DataControls(this->data_));

        exampleFrame->Show();

        return true;
    }

private:
    DataModel data_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(ExampleApp)
