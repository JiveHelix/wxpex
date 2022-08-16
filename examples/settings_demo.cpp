#include <iostream>
#include <string>
#include <fields/fields.h>

#include <pex/pex.h>

#include "wxpex/wxshim.h"
#include "wxpex/field.h"
#include "wxpex/view.h"
#include "wxpex/labeled_widget.h"


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

template<typename Observer>
using WeaponsControl = typename WeaponsGroup::Control<Observer>;


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

template<typename Observer>
using GpsControl = typename GpsGroup::Control<Observer>;


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


template<typename Observer>
struct DataControls
{
    WeaponsControl<Observer> weapons;
    GpsControl<Observer> gps;

    DataControls(DataModel &dataModel)
    {
        fields::AssignConvert<DataFields>(*this, dataModel);
    }

    template<typename Other>
    DataControls(const DataControls<Other> &other)
    {
        fields::AssignConvert<DataFields>(*this, other); 
    }
};


template<template<typename, typename> typename Widget>
class WeaponsWidget: public wxStaticBox
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    WeaponsWidget(
        wxWindow *parent,
        typename WeaponsGroup::Control<void> controls,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxStaticBox(parent, wxID_ANY, "Weapons")
    {
        using namespace wxpex;

        auto firstFruit = LabeledWidget(
            this,
            "firstFruit",
            new Widget(this, controls.firstFruit));

        auto secondFruit = LabeledWidget(
            this,
            "secondFruit",
            new Widget(this, controls.secondFruit));

        auto notFruit = LabeledWidget(
            this,
            "notFruit",
            new Widget(this, controls.notFruit));

        auto sizer = LayoutLabeled(
            layoutOptions,
            firstFruit,
            secondFruit,
            notFruit);

        this->SetSizerAndFit(sizer.release());
    }
};

using WeaponsView = WeaponsWidget<wxpex::View>;
using WeaponsEntry = WeaponsWidget<wxpex::Field>;


template<template<typename, typename> typename Widget>
class GpsWidget: public wxStaticBox
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    GpsWidget(
        wxWindow *parent,
        GpsControl<void> controls,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxStaticBox(parent, wxID_ANY, "Gps")
    {
        using namespace wxpex;

        auto time = LabeledWidget(
            this,
            "time",
            new Widget(this, controls.time));

        auto latitude = LabeledWidget(
            this,
            "latitude",
            new Widget(this, controls.latitude));

        auto longitude = LabeledWidget(
            this,
            "longitude",
            new Widget(this, controls.longitude));

        auto elevation = LabeledWidget(
            this,
            "elevation",
            new Widget(this, controls.elevation));

        auto sizer = LayoutLabeled(
            layoutOptions,
            time,
            latitude,
            longitude,
            elevation);

        this->SetSizerAndFit(sizer.release());
    }
};

using GpsView = GpsWidget<wxpex::View>;
using GpsEntry = GpsWidget<wxpex::Field>;


class DataView: public wxPanel
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    DataView(
        wxWindow *parent,
        DataControls<void> controls,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxPanel(parent, wxID_ANY)
    {
        auto weaponsView = wxpex::LabeledWidget(
            this,
            "Weapons View",
            new WeaponsView(
                this,
                controls.weapons,
                layoutOptions));

        auto weaponsEntry = wxpex::LabeledWidget(
            this,
            "Weapons Entry",
            new WeaponsEntry(
                this,
                controls.weapons,
                layoutOptions));

        auto gpsView = wxpex::LabeledWidget(
            this,
            "GPS View",
            new GpsView(
                this,
                controls.gps,
                layoutOptions));

        auto gpsEntry = wxpex::LabeledWidget(
            this,
            "GPS Entry",
            new GpsEntry(
                this,
                controls.gps,
                layoutOptions));

        this->SetSizerAndFit(
            LayoutLabeled(
                layoutOptions,
                weaponsView,
                weaponsEntry,
                gpsView,
                gpsEntry).release());
    }
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(DataControls<void> controls)
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
            new ExampleFrame(DataControls<void>(this->data_));

        exampleFrame->Show();

        return true;
    }

private:
    DataModel data_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP(ExampleApp)
