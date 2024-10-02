/**
  * @file field_demo.cpp
  *
  * @brief Demonstrates the use of wxpex::Field.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 09 Aug 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#include <iostream>
#include <string>
#include <fields/fields.h>
#include <tau/angles.h>
#include <pex/group.h>
#include <pex/endpoint.h>
#include "wxpex/wxshim.h"
#include "wxpex/view.h"
#include "wxpex/field.h"
#include "wxpex/labeled_widget.h"


struct AngleFilter
{
    static constexpr auto minimum = -tau::Angles<double>::pi;
    static constexpr auto maximum = tau::Angles<double>::pi;

    static double Set(double input)
    {
        return std::min(maximum, std::max(minimum, input));
    }
};

template<typename T>
struct ApplicationFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::angle, "angle"),
        fields::Field(&T::theAnswer, "theAnswer"),
        fields::Field(&T::message, "message"));

    static constexpr auto fieldsTypeName = "AngleDemo";
};

template<template<typename> typename T>
struct ApplicationTemplate
{
    T<pex::Filtered<double, AngleFilter>> angle;
    T<std::optional<double>> theAnswer;
    T<std::string> message;
};


using ApplicationGroup = pex::Group<ApplicationFields, ApplicationTemplate>;
using Model = typename ApplicationGroup::Model;
using Control = typename ApplicationGroup::Control;


/** Allow a control to use degrees, while the model uses radians. **/
struct DegreesFilter
{
    /** Convert to degrees on retrieval **/
    static double Get(double value)
    {
        return tau::ToDegrees(value);
    }

    /** Convert back to radians on assignment **/
    static double Set(double value)
    {
        return tau::ToRadians(value);
    }
};


using DegreesControl =
    pex::control::FilteredValue
    <
        decltype(Control::angle),
        DegreesFilter
    >;


class ExampleApp: public wxApp
{
public:
    static constexpr auto observerName = "ExampleApp";

    ExampleApp()
        :
        model_{},
        angle_(this, this->model_.angle),
        theAnswer_(this, this->model_.theAnswer)
    {
        this->angle_.Connect(&ExampleApp::OnUpdate_);
        this->theAnswer_.Connect(&ExampleApp::OnTheAnswer_);
        this->model_.message.Set("This is the initial message");
    }

    bool OnInit() override;

private:
    void OnUpdate_(double value)
    {
        if (value < 0)
        {
            this->model_.theAnswer.Set(42.0);
        }
        else
        {
            this->model_.theAnswer.Set({});
        }

        this->model_.message.Set(
            "The angle has been updated to: " + std::to_string(value));
    }

    void OnTheAnswer_(const std::optional<double> &value)
    {
        if (value)
        {
            std::cout << "Has an answer: " << *value << std::endl;
        }
        else
        {
            std::cout << "No answer" << std::endl;
        }
    }

private:
    Model model_;
    pex::Endpoint<ExampleApp, decltype(Model::angle)> angle_;
    pex::Endpoint<ExampleApp, decltype(Model::theAnswer)> theAnswer_;
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(Control control);
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(ExampleApp)


bool ExampleApp::OnInit()
{
    ExampleFrame *exampleFrame =
        new ExampleFrame(Control(this->model_));

    exampleFrame->Show();

    return true;
}


ExampleFrame::ExampleFrame(Control control)
    :
    wxFrame(nullptr, wxID_ANY, "wxpex::Field Demo")
{
    using namespace wxpex;

    auto radiansView =
        LabeledWidget(
            this,
            "Radians:",
            new View(this, control.angle));

    auto degreesView =
        LabeledWidget(
            this,
            "Degrees:",
            new View(this, DegreesControl(control.angle)));

    auto radiansEntry =
        LabeledWidget(
            this,
            "Radians:",
            new Field(this, control.angle));

    auto degreesEntry =
        LabeledWidget(
            this,
            "Degrees:",
            new Field(this, DegreesControl(control.angle)));

    auto theAnswer =
        LabeledWidget(
            this,
            "The Answer:",
            CreateField<2>(this, control.theAnswer));

    auto messageField =
        LabeledWidget(
            this,
            "Message:",
            new View(this, control.message));

    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    auto sizer = LayoutLabeled(
        LayoutOptions{},
        radiansView,
        degreesView,
        radiansEntry,
        degreesEntry,
        theAnswer,
        messageField);

    topSizer->Add(sizer.release(), 1, wxALL | wxEXPAND, 10);

    this->SetSizerAndFit(topSizer.release());
}
