#pragma once

#include "wxpex/wxshim.h"

#include <wx/gauge.h>
#include <pex/group.h>
#include <wxpex/async.h>


namespace wxpex
{


template<typename T>
struct GaugeFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::value, "value"),
        fields::Field(&T::maximum, "maximum"));
};


template<template<typename> typename T>
struct GaugeTemplate
{
    T<wxpex::MakeAsync<size_t>> value;
    T<wxpex::MakeAsync<size_t>> maximum;
};


using GaugeGroup = pex::Group<GaugeFields, GaugeTemplate>;

using GaugeModel = typename GaugeGroup::Model;
using GaugeControl = typename GaugeGroup::Control<void>;


struct GaugeWorker: public GaugeControl
{
public:
    GaugeWorker() = default;

    GaugeWorker(GaugeModel &model)
        :
        GaugeControl()
    {
        this->value = model.value.GetWorkerControl();

        this->maximum = model.maximum.GetWorkerControl();
    }
};


template<typename Observer>
using GaugeTerminus = typename GaugeGroup::Terminus<Observer>;

using GaugeState = typename GaugeGroup::Plain;


class Gauge: public wxGauge
{
public:
    using Base = wxGauge;

    Gauge(
        wxWindow *parent,
        GaugeControl control,
        long style = wxGA_HORIZONTAL);

private:
    void OnValue_(size_t value);
    void OnMaximum_(size_t maximum);

    GaugeTerminus<Gauge> terminus_;
};


} // end namespace pex
