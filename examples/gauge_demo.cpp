
#include <thread>
#include "wxpex/gauge.h"
#include "wxpex/button.h"


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::gauge1, "gauge1"),
        fields::Field(&T::gauge2, "gauge2"),
        fields::Field(&T::start, "start"),
        fields::Field(&T::stop, "stop"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<pex::MakeGroup<wxpex::GaugeGroup>> gauge1;
    T<pex::MakeGroup<wxpex::GaugeGroup>> gauge2;
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


class ExampleApp: public wxApp
{
public:
    ExampleApp()
        :
        model_{},
        terminus_(this, this->model_),
        isRunning_{false},
        worker1_{},
        worker2_{}
    {
        this->model_.gauge1.Set(wxpex::GaugeState{{0, 100}});
        this->model_.gauge2.Set(wxpex::GaugeState{{0, 100}});
        this->terminus_.start.Connect(&ExampleApp::OnStart_);
        this->terminus_.stop.Connect(&ExampleApp::OnStop_);
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

private:
    DemoModel model_;
    DemoTerminus<ExampleApp> terminus_;
    std::atomic_bool isRunning_;
    std::thread worker1_;
    std::thread worker2_;
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
    auto gauge2 = new Gauge(this, demoControl.gauge2);
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
