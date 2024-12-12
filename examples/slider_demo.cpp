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
#include <pex/endpoint.h>
#include <pex/converting_filter.h>
#include <fields/fields.h>
#include "wxpex/slider.h"
#include "wxpex/check_box.h"


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::position, "position"),
        fields::Field(&T::changeRange, "changeRange"),
        fields::Field(&T::optional, "optional"),
        fields::Field(&T::playbackSpeed, "playbackSpeed"));
};

using pex::Limit;
using pex::MakeRange;

template<template<typename> typename T>
struct DemoTemplate
{
    T<MakeRange<double, Limit<0>, Limit<1>>> position;
    T<bool> changeRange;
    T<MakeRange<std::optional<int>, Limit<0>, Limit<100>>> optional;
    T<MakeRange<float, Limit<0, 25, 100>, Limit<4>>> playbackSpeed;
};


struct Demo: public DemoTemplate<pex::Identity>
{
    static constexpr int defaultPosition = 0;
    static constexpr float defaultPlaybackSpeed = 1.0f;

    static Demo MakeDefault()
    {
        return {{defaultPosition, false, defaultPlaybackSpeed, {}}};
    }
};

using DemoGroup = pex::Group<DemoFields, DemoTemplate, pex::PlainT<Demo>>;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::Control;

using OptionalControl = decltype(DemoControl::optional);
using OptionalValue = decltype(OptionalControl::value);

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
    PlaybackSpeed,
    PlaybackSpeedFilter<float>>;


class ExampleApp : public wxApp
{
public:
    ExampleApp()
        :
        model_(Demo::MakeDefault()),
        position_(this, this->model_.position, &ExampleApp::OnPosition_),

        changeRange_(
            this,
            this->model_.changeRange,
            &ExampleApp::OnChangeRange_)
    {

    }

    bool OnInit() override;

private:
    void OnPosition_(double position)
    {
        if (position > 0.5)
        {
            this->model_.optional.Set({});
        }
        else
        {
            this->model_.optional.Set(0);
        }
    }

    void OnChangeRange_(bool shortRange)
    {
        if (shortRange)
        {
            this->model_.position.SetMaximum(0.005);
        }
        else
        {
            this->model_.position.SetMaximum(1.0);
        }
    }

private:
    DemoModel model_;

    using PositionEndpoint =
        pex::Endpoint<ExampleApp, decltype(DemoModel::position)>;

    PositionEndpoint position_;

    using ChangeRangeEndpoint =
        pex::Endpoint<ExampleApp, decltype(DemoModel::changeRange)>;

    ChangeRangeEndpoint changeRange_;
};


const int precision = 3;


using PlaybackSpeedSlider =
    wxpex::ValueSliderConvert
    <
        FilteredPlaybackSpeed,
        PlaybackSpeedValue,
        PlaybackSpeedConverter
    >;

using TestRangeSlider = typename PlaybackSpeedSlider::RangeSlider;
using SelectedRange = typename TestRangeSlider::Range;

static_assert(std::is_same_v<SelectedRange, FilteredPlaybackSpeed>);


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
    auto positionSlider = wxpex::CreateViewSlider<3>(this, control.position);

    auto positionFieldSlider =
        wxpex::CreateFieldSlider<1>(this, control.position);

    auto optionalSlider =
        new wxpex::Slider(this, control.optional);

    auto vertical = wxpex::Style::vertical;

    auto verticalSlider =
        CreateViewSlider<precision>(this, control.position, vertical);

    auto checkBox =
        new wxpex::CheckBox(this, "Change range", control.changeRange);

    auto playbackSpeedSlider =
        new PlaybackSpeedSlider(
            this,
            FilteredPlaybackSpeed(control.playbackSpeed),
            control.playbackSpeed.value);

    auto verticalSpeedSlider = new PlaybackSpeedSlider(
        this,
        FilteredPlaybackSpeed(control.playbackSpeed),
        control.playbackSpeed.value,
        wxpex::Style::vertical);

    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    topSizer->Add(positionSlider, 0, wxALL | wxEXPAND, 10);
    topSizer->Add(positionFieldSlider, 0, wxALL | wxEXPAND, 10);
    topSizer->Add(optionalSlider, 0, wxALL | wxEXPAND, 10);
    topSizer->Add(verticalSlider, 1, wxALL | wxEXPAND, 10);
    topSizer->Add(checkBox, 1, wxALL | wxEXPAND, 10);
    topSizer->Add(playbackSpeedSlider, 0, wxALL | wxEXPAND, 10);
    topSizer->Add(verticalSpeedSlider, 0, wxALL | wxEXPAND, 10);

    this->SetSizerAndFit(topSizer.release());

    this->Bind(
        wxpex::SliderDone,
        &ExampleFrame::OnSliderDone_,
        this);
}
