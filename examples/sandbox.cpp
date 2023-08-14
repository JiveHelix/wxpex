/**
  * @file sandbox.cpp
  *
  * @brief A sandbox for experimentation.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 07 Aug 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#include <iostream>
#include <string>
#include <jive/formatter.h>
#include <fields/fields.h>
#include <tau/angles.h>
#include <pex/signal.h>
#include <pex/value.h>
#include <pex/converter.h>
#include <wxpex/wxshim.h>
#include <wxpex/view.h>
#include <wxpex/knob.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>



template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::velocity, "velocity"),
        fields::Field(&T::azimuth, "azimuth"),
        fields::Field(&T::elevation, "elevation"));
};


using pex::Limit;
using pex::MakeRange;


template<template<typename> typename T>
struct DemoTemplate
{
    T<pex::MakeRange<double, Limit<0>, Limit<1000>>> velocity;
    T<pex::MakeRange<double, Limit<-90>, Limit<90>>> azimuth;
    T<pex::MakeRange<double, Limit<0>, Limit<90>>> elevation;

    static constexpr auto fields = DemoFields<DemoTemplate>::fields;
    static constexpr auto fieldsTypeName = "Demo";
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate>;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::Control;


class ExampleApp: public wxApp
{
public:
    ExampleApp()
        :
        model_{},
        endpoint_(this, DemoControl(this->model_), &ExampleApp::OnSettings_)
    {

    }

    void OnSettings_([[maybe_unused]] const DemoGroup::Plain &settings)
    {
        std::cout << fields::DescribeColorized(settings) << std::endl;
    }

    bool OnInit() override;

private:
    DemoModel model_;
    pex::Endpoint<ExampleApp, DemoControl> endpoint_;
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(DemoControl control);
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(ExampleApp)


bool ExampleApp::OnInit()
{
    ExampleFrame *exampleFrame = new ExampleFrame(DemoControl(this->model_));
    exampleFrame->Show();
    exampleFrame->Layout();
    exampleFrame->Update();
    return true;
}


ExampleFrame::ExampleFrame([[maybe_unused]] DemoControl control)
    :
    wxFrame(nullptr, wxID_ANY, "wxpex::Sandbox")
{
    auto velocityLabel = new wxStaticText(this, wxID_ANY, "Velocity");
    auto vv = new wxpex::View(this, control.velocity.value);
    auto vk = new wxpex::Knob(this, control.velocity);

    auto azimuthLabel = new wxStaticText(this, wxID_ANY, "Azimuth");
    auto av = new wxpex::View(this, control.azimuth.value);
    auto ak = new wxpex::Knob(this, control.azimuth);

    auto elevationLabel = new wxStaticText(this, wxID_ANY, "Elevation");
    auto ev = new wxpex::View(this, control.elevation.value);
    auto ek = new wxpex::Knob(this, control.elevation);

    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    auto velocitySizer =
        wxpex::LayoutItems(
            wxpex::horizontalItems,
            velocityLabel,
            vv,
            vk);

    auto azimuthSizer =
        wxpex::LayoutItems(
            wxpex::horizontalItems,
            azimuthLabel,
            av,
            ak);

    auto elevationSizer =
        wxpex::LayoutItems(
            wxpex::horizontalItems,
            elevationLabel,
            ev,
            ek);

    auto sizer = wxpex::LayoutItems(
        wxpex::verticalItems,
        velocitySizer.release(),
        azimuthSizer.release(),
        elevationSizer.release());

    topSizer->Add(sizer.release(), 1, wxALL, 5);

    this->SetSizerAndFit(topSizer.release());
}
