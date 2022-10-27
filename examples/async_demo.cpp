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
#include <pex/value.h>
#include <pex/signal.h>

#include "wxpex/wxshim.h"
#include "wxpex/view.h"
#include "wxpex/field.h"
#include "wxpex/labeled_widget.h"
#include "wxpex/async.h"
#include "wxpex/button.h"
#include "wxpex/window.h"


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::startingAngle, "startingAngle"),
        fields::Field(&T::currentAngle, "currentAngle"),
        fields::Field(&T::fail, "fail"),
        fields::Field(&T::threadFail, "threadFail"),
        fields::Field(&T::start, "start"),
        fields::Field(&T::stop, "stop"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<double> startingAngle;
    T<wxpex::MakeAsync<double>> currentAngle;
    T<pex::MakeSignal> fail;
    T<pex::MakeSignal> threadFail;
    T<pex::MakeSignal> start;
    T<pex::MakeSignal> stop;

    static constexpr auto fields = DemoFields<DemoTemplate>::fields;
    static constexpr auto fieldsTypeName = "Demo";
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate>;
using DemoControl = typename DemoGroup::Control<void>;
using DemoModel = typename DemoGroup::Model;

template<typename Observer>
using DemoTerminus = typename DemoGroup::template Terminus<Observer>;


template<typename Upstream>
using RadiansControl = pex::control::Value<void, Upstream>;

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
    pex::control::FilteredValue<void, Upstream, DegreesFilter>;


template<typename Upstream>
auto MakeDegreesControl(Upstream &upstream)
{
    return DegreesControl<Upstream>(upstream);
}


class ExtraWindow: public wxFrame
{
public:
    ExtraWindow(const std::string &name)
        :
        wxFrame(nullptr, wxID_ANY, name)
    {
        this->Show(true);
    }
};


using WindowArray = std::array<wxpex::Window, 4>;


class ExampleApp: public wxApp
{
public:
    ExampleApp()
        :
        mutex_{},
        model_{},
        terminus_(this, this->model_),
        isRunning_{},
        threadFail_{},
        worker_{},
        extraWindows_{std::make_unique<WindowArray>()}
    {
        for (size_t i = 0; i < 4; ++i)
        {
            this->extraWindows_->at(i) =
                wxpex::Window(
                    new ExtraWindow("Extra Window " + std::to_string(i)));
        }

        this->terminus_.startingAngle.Connect(&ExampleApp::OnUpdate_);
        this->terminus_.fail.Connect(&ExampleApp::OnFail_);
        this->terminus_.threadFail.Connect(&ExampleApp::OnThreadFail_);
        this->terminus_.start.Connect(&ExampleApp::OnStart_);
        this->terminus_.stop.Connect(&ExampleApp::OnStop_);
    }

    bool OnInit() override;

    virtual ~ExampleApp()
    {
        this->OnStop_();
    }

private:
    void OnClose_(wxCloseEvent &event)
    {
        this->extraWindows_.reset();
        event.Skip();
    }

    void OnUpdate_(double value)
    {
        auto control = this->model_.currentAngle.GetWxControl();
        control.Set(value);
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

    void OnFail_()
    {
        std::cerr << "Throwing a runtime_error from the event loop."
            << std::endl;

        throw std::runtime_error("fail");
    }

    void OnThreadFail_()
    {
        this->threadFail_ = true;
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
        auto workerControl = this->model_.currentAngle.GetWorkerControl();

        while (this->isRunning_)
        {
            if (this->threadFail_)
            {
                throw std::runtime_error("thread fail");
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
    DemoTerminus<ExampleApp> terminus_;

    std::atomic_bool isRunning_;
    std::atomic_bool threadFail_;
    std::thread worker_;

    std::unique_ptr<WindowArray> extraWindows_;
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

    exampleFrame->Bind(wxEVT_CLOSE_WINDOW, &ExampleApp::OnClose_, this);

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
            new View(this, demoControl.currentAngle));

    auto degreesView =
        LabeledWidget(
            this,
            "Degrees:",
            new View(this, MakeDegreesControl(demoControl.currentAngle)));

    auto radiansEntry =
        LabeledWidget(
            this,
            "Radians start:",
            new Field(this, demoControl.startingAngle));

    auto degreesEntry =
        LabeledWidget(
            this,
            "Degrees start:",
            new Field(this, MakeDegreesControl(demoControl.startingAngle)));

    auto startButton = new Button(this, "Start", demoControl.start);
    auto stopButton = new Button(this, "Stop", demoControl.stop);

    // fail button allows the user to test whether exceptions thrown in the
    // main thread propagate as expected.
    //
    // With exceptions disabled in wxWidgets, an unhandled exception will
    // terminate the program like normal. With wxWidgets exceptions enabled,
    // wxWidgets silently consumes all exceptions and exits without any error
    // message.
    auto failButton = new Button(this, "Fail", demoControl.fail);

    // Same as fail button, but throws an error from a child thread instead.
    auto threadFailButton =
        new Button(this, "Thread Fail", demoControl.threadFail);

    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    auto fieldsSizer = LayoutLabeled(
        LayoutOptions{},
        radiansView,
        degreesView,
        radiansEntry,
        degreesEntry);

    sizer->Add(fieldsSizer.release(), 0, wxALL, 10);

    auto buttonSizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    buttonSizer->Add(failButton, 0, wxRIGHT, 5);
    buttonSizer->Add(threadFailButton, 0, wxRIGHT, 5);
    buttonSizer->Add(startButton, 0, wxRIGHT, 5);
    buttonSizer->Add(stopButton);

    sizer->Add(buttonSizer.release(), 0, wxTOP, 5);

    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    topSizer->Add(sizer.release(), 0, wxALL, 10);
    this->SetSizerAndFit(topSizer.release());
}
