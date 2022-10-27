/**
  * @file slider_demo.cpp
  *
  * @brief A demonstration of wxpex::Slider.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 17 Aug 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#include <pex/range.h>
#include <pex/converting_filter.h>
#include <fields/fields.h>
#include "wxpex/slider.h"


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::position, "position"),
        fields::Field(&T::playbackSpeed, "playbackSpeed"));
};

using pex::Limit;
using pex::MakeRange;

template<template<typename> typename T>
struct DemoTemplate
{
    T<MakeRange<int, Limit<0>, Limit<1000>>> position;
    T<MakeRange<float, Limit<0, 25, 100>, Limit<4>>> playbackSpeed;
};


struct Demo: public DemoTemplate<pex::Identity>
{
    static constexpr int defaultPosition = 0;
    static constexpr float defaultPlaybackSpeed = 1.0f;

    static Demo MakeDefault()
    {
        return {{defaultPosition, defaultPlaybackSpeed}};
    }
};

using DemoGroup = pex::Group<DemoFields, DemoTemplate, Demo>;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::Control<void>;

using PositionControl = decltype(DemoControl::position);
using PositionValue = decltype(PositionControl::value);

using PlaybackSpeed = decltype(DemoModel::playbackSpeed);
using PlaybackSpeedControl = decltype(DemoControl::playbackSpeed);

// The unfiltered value used to display playback speed
using PlaybackSpeedValue = decltype(PlaybackSpeedControl::value);

static constexpr unsigned clicksPerOctave = 3;

template<typename T>
using PlaybackSpeedFilter =
    pex::control::LogarithmicFilter<T, 2, clicksPerOctave>;


struct PlaybackSpeedTraits: pex::DefaultConverterTraits
{
    static constexpr int width = 5;
    static constexpr int precision = 3;
};

struct PlaybackSpeedConverter:
    public pex::Converter<typename PlaybackSpeed::Type, PlaybackSpeedTraits>
{
    template<typename U>
    static std::string ToString(U &&value)
    {
        return ConvertToString::Call(std::forward<U>(value)) + "x";
    }
};


using FilteredPlaybackSpeed = ::pex::control::Range<
    void,
    PlaybackSpeed,
    PlaybackSpeedFilter<float>>;


class ExampleApp : public wxApp
{
public:
    ExampleApp()
        :
        model_(Demo::MakeDefault())
    {

    }

    bool OnInit() override;

private:
    DemoModel model_;
};


const int precision = 3;

using PositionSlider =
    wxpex::SliderAndValue<PositionControl, PositionValue, precision>;


using PositionFieldSlider =
    wxpex::FieldSlider<PositionControl, PositionValue, 1>;


using PlaybackSpeedSlider =
    wxpex::SliderAndValueConvert
    <
        FilteredPlaybackSpeed,
        PlaybackSpeedValue,
        PlaybackSpeedConverter
    >;


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(DemoControl control);

    void OnSliderDone_(wxCommandEvent &)
    {
        this->Layout();
    }
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP(ExampleApp)


bool ExampleApp::OnInit()
{
    ExampleFrame *exampleFrame = new ExampleFrame(DemoControl(this->model_));
    exampleFrame->Show();

    return true;
}



ExampleFrame::ExampleFrame(DemoControl control)
    :
    wxFrame(nullptr, wxID_ANY, "wxpex::Slider Demo")
{
    auto positionSlider =
        new PositionSlider(this, control.position, control.position.value);

    auto positionFieldSlider =
        new PositionFieldSlider(this, control.position, control.position.value);

    auto verticalSlider =
        new PositionSlider(
            this,
            control.position,
            control.position.value,
            wxSL_VERTICAL);

    auto playbackSpeedSlider =
        new PlaybackSpeedSlider(
            this,
            FilteredPlaybackSpeed(control.playbackSpeed),
            control.playbackSpeed.value);

    auto verticalSpeedSlider = new PlaybackSpeedSlider(
        this,
        FilteredPlaybackSpeed(control.playbackSpeed),
        control.playbackSpeed.value,
        wxSL_VERTICAL);

    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    topSizer->Add(positionSlider, 0, wxALL | wxEXPAND, 10);
    topSizer->Add(positionFieldSlider, 0, wxALL | wxEXPAND, 10);
    topSizer->Add(verticalSlider, 1, wxALL | wxEXPAND, 10);
    topSizer->Add(playbackSpeedSlider, 0, wxALL | wxEXPAND, 10);
    topSizer->Add(verticalSpeedSlider, 0, wxALL | wxEXPAND, 10);

    this->SetSizerAndFit(topSizer.release());

    this->Bind(
        wxpex::SliderDone,
        &ExampleFrame::OnSliderDone_,
        this);
}
