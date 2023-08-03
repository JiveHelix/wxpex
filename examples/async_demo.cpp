/**
  * @file async_demo.cpp
  *
  * @brief Demonstrates asynchronous communcation from a worker thread.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 22 Mar 2022
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/


#include <string>
#include <thread>
#include <chrono>
#include <array>
#include <tau/angles.h>
#include <jive/future.h>
#include <fields/fields.h>

#include <pex/group.h>
#include <pex/endpoint.h>
#include <pex/value.h>
#include <pex/signal.h>

#include "wxpex/wxshim.h"
#include "wxpex/view.h"
#include "wxpex/field.h"
#include "wxpex/labeled_widget.h"
#include "wxpex/async.h"
#include "wxpex/button.h"
#include "wxpex/window.h"



template<typename T, typename Enable = std::void_t<>>
struct HasControl_: std::false_type {};

template<typename T>
struct HasControl_
<
    T,
    std::void_t<typename T::Control>
>
: std::true_type {};

template<typename T>
inline constexpr bool HasControl = HasControl_<T>::value;


static_assert(HasControl<wxpex::Async<double>>);





template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::startingAngle, "startingAngle"),
        fields::Field(&T::currentAngle, "currentAngle"),
        fields::Field(&T::reset, "reset"),
        fields::Field(&T::start, "start"),
        fields::Field(&T::stop, "stop"));
};


static_assert(
    pex::IsMakeRange<pex::MakeRange<double, void, void, wxpex::Async>>);


template<template<typename> typename T>
struct DemoTemplate
{
    T<double> startingAngle;
    T<pex::MakeRange<double, void, void, wxpex::Async>> currentAngle;
    T<pex::MakeSignal> reset;
    T<pex::MakeSignal> start;
    T<pex::MakeSignal> stop;

    static constexpr auto fields = DemoFields<DemoTemplate>::fields;
    static constexpr auto fieldsTypeName = "Demo";
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate>;
using DemoControl = typename DemoGroup::Control;
using DemoModel = typename DemoGroup::Model;

using CurrentAngleModel = decltype(DemoModel::currentAngle);

static_assert(
    std::is_same_v<wxpex::Async<double, pex::model::RangeFilter<double>>,
    typename CurrentAngleModel::Value>);

static_assert(HasControl<typename CurrentAngleModel::Value>);

static_assert(pex::control::HasControl<typename CurrentAngleModel::Value>);

static_assert(
    std::is_same_v
    <
        CurrentAngleModel,
        typename decltype(DemoControl::currentAngle)::Upstream
    >);


using AngleControl = decltype(decltype(DemoControl::currentAngle)::value);


template<typename Upstream>
using RadiansControl = pex::control::Value<Upstream>;

template<typename Upstream>
auto MakeRadiansControl(Upstream &upstream)
{
    return RadiansControl<Upstream>(upstream);
}

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


template<typename Upstream>
using DegreesControl =
    pex::control::FilteredValue<Upstream, DegreesFilter>;


template<typename Upstream>
auto MakeDegreesControl(Upstream &upstream)
{
    return DegreesControl<Upstream>(upstream);
}


using WindowArray = std::array<wxpex::Window, 4>;


class ExampleApp: public wxApp
{
public:
    static constexpr auto observerName = "ExampleApp";

    ExampleApp()
        :
        mutex_{},
        model_{},
        control_(this->model_),
        currentAngleSetWait_(this->control_.currentAngle.value),
        endpoints_(this, this->control_),
        isRunning_{},
        reset_{},
        worker_{}
    {
        this->endpoints_.startingAngle.Connect(&ExampleApp::OnUpdate_);
        this->endpoints_.reset.Connect(&ExampleApp::OnReset_);
        this->endpoints_.start.Connect(&ExampleApp::OnStart_);
        this->endpoints_.stop.Connect(&ExampleApp::OnStop_);
    }

    bool OnInit() override;

    virtual ~ExampleApp()
    {
        this->OnStop_();
    }

private:
    void OnUpdate_(double value)
    {
        this->control_.currentAngle.value.Set(value);
    }

    void OnStart_()
    {
        if (this->isRunning_)
        {
            return;
        }

        this->isRunning_ = true;

        this->worker_ = std::thread(
            std::bind(&ExampleApp::WorkerThread_, this));
    }

    void OnReset_()
    {
        this->reset_ = true;
    }

    void OnStop_()
    {
        if (!this->isRunning_)
        {
            return;
        }

        this->isRunning_ = false;
        this->worker_.join();

        std::cout << "Stopped: " << std::endl;
        std::cout << fields::DescribeColorized(this->model_.Get()) << std::endl;
    }

    void WorkerThread_()
    {
        auto workerControl =
            this->control_.currentAngle.value.GetWorkerControl();

        while (this->isRunning_)
        {
            if (this->reset_)
            {
                this->currentAngleSetWait_.Set(
                    this->control_.startingAngle.Get());

                this->reset_ = false;

                continue;
            }

            auto next =
                this->model_.currentAngle.Get() + tau::Angles<double>::pi / 4.0;

            workerControl.Set(next);

            using namespace std::chrono_literals;
            std::this_thread::sleep_for(250ms);
        }
    }

private:
    std::mutex mutex_;
    DemoModel model_;
    DemoControl control_;
    wxpex::SetWait<AngleControl> currentAngleSetWait_;
    pex::EndpointGroup<ExampleApp, DemoControl> endpoints_;

    std::atomic_bool isRunning_;
    std::atomic_bool reset_;
    std::thread worker_;
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(DemoControl demoControl);
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP(ExampleApp)


bool ExampleApp::OnInit()
{
    auto exampleFrame = new ExampleFrame(DemoControl(this->model_));
    exampleFrame->Show();

    return true;
}


ExampleFrame::ExampleFrame(DemoControl demoControl)
    :
    wxFrame(nullptr, wxID_ANY, "wxpex::Field Demo")
{
    using namespace wxpex;

    auto radiansView =
        LabeledWidget(
            this,
            "Radians:",
            new View(this, demoControl.currentAngle.value));

    auto degreesView =
        LabeledWidget(
            this,
            "Degrees:",
            new View(this, MakeDegreesControl(demoControl.currentAngle.value)));

    auto radiansEntry =
        LabeledWidget(
            this,
            "Radians start:",
            new Field(this, demoControl.startingAngle));

    auto degreesEntry =
        LabeledWidget(
            this,
            "Degrees start:",
            new Field(
                this,
                MakeDegreesControl(demoControl.startingAngle)));

    auto startButton = new Button(this, "Start", demoControl.start);
    auto stopButton = new Button(this, "Stop", demoControl.stop);

    auto resetButton =
        new Button(this, "Reset to start value", demoControl.reset);

    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    auto fieldsSizer = LayoutLabeled(
        LayoutOptions{},
        radiansView,
        degreesView,
        radiansEntry,
        degreesEntry);

    sizer->Add(fieldsSizer.release(), 0, wxALL, 10);

    auto buttonSizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    buttonSizer->Add(resetButton, 0, wxRIGHT, 5);
    buttonSizer->Add(startButton, 0, wxRIGHT, 5);
    buttonSizer->Add(stopButton);

    sizer->Add(buttonSizer.release(), 0, wxTOP, 5);

    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    topSizer->Add(sizer.release(), 0, wxALL, 10);
    this->SetSizerAndFit(topSizer.release());
}
