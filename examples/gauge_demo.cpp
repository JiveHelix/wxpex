#include <thread>
#include <pex/endpoint.h>
#include "wxpex/gauge.h"
#include "wxpex/button.h"


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::gauge1, "gauge1"),
        fields::Field(&T::gauge2, "gauge2"),
        fields::Field(&T::start, "start"),
        fields::Field(&T::stop, "stop"),
        fields::Field(&T::values, "values"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<wxpex::GaugeGroupMaker> gauge1;
    T<wxpex::GaugeGroupMaker> gauge2;
    T<pex::MakeSignal> start;
    T<pex::MakeSignal> stop;
    T<wxpex::MakeAsync<int>> values;

    static constexpr auto fields = DemoFields<DemoTemplate>::fields;
    static constexpr auto fieldsTypeName = "Demo";
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate>;
using DemoControl = typename DemoGroup::Control;
using DemoModel = typename DemoGroup::Model;


class ExampleApp: public wxApp
{
public:
    static constexpr auto observerName = "ExampleApp";

    ExampleApp()
        :
        model_{},
        endpoints_(this, this->model_),
        observedValues_(),
        isRunning_{false},
        worker1_{},
        worker2_{}
    {
        this->model_.gauge1.Set(wxpex::GaugeState{{0, 100}});
        this->model_.gauge2.Set(wxpex::GaugeState{{0, 100}});
        this->endpoints_.start.Connect(&ExampleApp::OnStart_);
        this->endpoints_.stop.Connect(&ExampleApp::OnStop_);
        this->endpoints_.values.Connect(&ExampleApp::OnValues_);
    }

    ~ExampleApp()
    {
        if (this->isRunning_)
        {
            this->isRunning_ = false;
            this->worker1_.join();
            this->worker2_.join();
        }
    }

    bool OnInit() override;

private:
    void OnStart_()
    {
        if (this->isRunning_)
        {
            return;
        }

        this->isRunning_ = true;

        this->worker1_ = std::thread(
            std::bind(&ExampleApp::WorkerThread1_, this));

        this->worker2_ = std::thread(
            std::bind(&ExampleApp::WorkerThread2_, this));

        this->observedValues_.clear();

        this->worker3_ = std::thread(
            std::bind(&ExampleApp::WorkerThread3_, this));
    }

    void OnStop_()
    {
        if (!this->isRunning_)
        {
            return;
        }

        this->isRunning_ = false;
        this->worker1_.join();
        this->worker2_.join();
        this->worker3_.join();

        for (auto &i: this->observedValues_)
        {
            std::cout << i << std::endl;
        }
    }

    void OnValues_(int values)
    {
        this->observedValues_.push_back(values);
    }

    void WorkerThread1_()
    {
        auto valueControl = this->model_.gauge1.value.GetWorkerControl();
        auto maximumControl = this->model_.gauge1.maximum.GetWorkerControl();
        size_t maximum = 100;
        size_t value = 0;
        maximumControl.Set(100);
        valueControl.Set(0);

        while (this->isRunning_ && value < maximum)
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(100ms);
            valueControl.Set(++value);
        }
    }

    void WorkerThread2_()
    {
        auto valueControl = this->model_.gauge2.value.GetWorkerControl();
        auto maximumControl = this->model_.gauge2.maximum.GetWorkerControl();
        size_t maximum = 1000;
        size_t value = 0;
        maximumControl.Set(1000);
        valueControl.Set(0);

        while (this->isRunning_ && value < maximum)
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(10ms);
            valueControl.Set(++value);
        }
    }

    void WorkerThread3_()
    {
        auto control = this->model_.values.GetWorkerControl();

        for (int i = 0; i < 100; ++i)
        {
            if (i % 5 == 0)
            {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(20ms);
            }

            control.Set(i);
        }
    }

private:
    DemoModel model_;
    pex::EndpointGroup<ExampleApp, DemoControl> endpoints_;
    std::vector<int> observedValues_;
    std::atomic_bool isRunning_;
    std::thread worker1_;
    std::thread worker2_;
    std::thread worker3_;
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
    wxFrame(nullptr, wxID_ANY, "wxpex::Gauge Demo")
{
    using namespace wxpex;

    auto gauge1 = new Gauge(this, demoControl.gauge1);
    auto gauge2 = new ValueGauge(this, demoControl.gauge2);
    auto startButton = new Button(this, "Start", demoControl.start);
    auto stopButton = new Button(this, "Stop", demoControl.stop);

    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    sizer->Add(gauge1, 0, wxALL | wxEXPAND, 5);
    sizer->Add(gauge2, 0, wxALL | wxEXPAND, 5);

    auto buttonSizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    buttonSizer->Add(startButton, 0, wxRIGHT, 5);
    buttonSizer->Add(stopButton);

    sizer->Add(buttonSizer.release(), 0, wxTOP | wxCENTER | wxALIGN_CENTER, 5);

    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    topSizer->Add(sizer.release(), 0, wxALL | wxEXPAND, 10);
    this->SetSizerAndFit(topSizer.release());
}
