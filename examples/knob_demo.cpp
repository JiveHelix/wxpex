/**
  * @file knob_demo.cpp
  *
  * @brief A demonstration of wxpex::Knob.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 1 Apr 2023
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#include <pex/range.h>
#include <pex/converting_filter.h>
#include <fields/fields.h>
#include "wxpex/knob.h"


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::hue, "hue"),
        fields::Field(&T::playbackSpeed, "playbackSpeed"));
};

using pex::Limit;
using pex::MakeRange;

template<template<typename> typename T>
struct DemoTemplate
{
    T<MakeRange<double, Limit<0>, Limit<360>>> hue;
    T<MakeRange<float, Limit<0, 25, 100>, Limit<4>>> playbackSpeed;
};


struct Demo: public DemoTemplate<pex::Identity>
{
    static constexpr int defaultHue = 0;
    static constexpr float defaultPlaybackSpeed = 1.0f;

    static Demo MakeDefault()
    {
        return {{defaultHue, defaultPlaybackSpeed}};
    }
};

using DemoGroup = pex::Group<DemoFields, DemoTemplate, pex::PlainT<Demo>>;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::Control;

using HueControl = decltype(DemoControl::hue);
using HueValue = decltype(HueControl::value);

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
    static constexpr int width = 0;
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

using HueKnob =
    wxpex::ViewKnob<HueControl, HueValue, -1, precision>;


using HueFieldKnob =
    wxpex::FieldKnob<HueControl, HueValue, 4>;


using PlaybackSpeedKnob =
    wxpex::ViewKnobConvert
    <
        FilteredPlaybackSpeed,
        PlaybackSpeedValue,
        PlaybackSpeedConverter
    >;


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(DemoControl control);

    void OnKnobDone_(wxCommandEvent &)
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
    wxFrame(nullptr, wxID_ANY, "wxpex::Knob Demo")
{
    auto hueKnob =
        wxpex::CreateViewKnob<precision>(this, control.hue);

    auto hueFieldKnob =
        wxpex::CreateFieldKnob<4>(this, control.hue);

    auto playbackSpeedKnob =
        new PlaybackSpeedKnob(
            this,
            FilteredPlaybackSpeed(control.playbackSpeed),
            control.playbackSpeed.value);

    auto verticalSpeedKnob = new PlaybackSpeedKnob(
        this,
        FilteredPlaybackSpeed(control.playbackSpeed),
        control.playbackSpeed.value,
        wxpex::KnobSettings()
            .Layout(wxpex::Style::vertical)
            .Spacing(10));

    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    topSizer->Add(hueKnob, 0, wxALL | wxEXPAND, 10);
    topSizer->Add(hueFieldKnob, 0, wxALL | wxEXPAND, 10);
    topSizer->Add(playbackSpeedKnob, 0, wxALL | wxEXPAND, 10);
    topSizer->Add(verticalSpeedKnob, 0, wxALL | wxEXPAND, 10);

    this->SetSizerAndFit(topSizer.release());

    // Allow all views to adjust their size to show the new values.
    this->Bind(
        wxpex::KnobDone,
        &ExampleFrame::OnKnobDone_,
        this);
}
