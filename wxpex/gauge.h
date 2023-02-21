#pragma once

#include "wxpex/wxshim.h"

#include <wx/gauge.h>
#include <pex/group.h>
#include <wxpex/async.h>


namespace wxpex
{


class GaugeFilter
{
public:
    GaugeFilter()
        :
        maximum_(std::numeric_limits<size_t>::max())
    {

    }

    GaugeFilter(size_t maximum)
        :
        maximum_(maximum)
    {

    }

    size_t Get(size_t value) const
    {
        return value;
    }

    size_t Set(size_t value) const
    {
        return std::min(value, this->maximum_);
    }

private:
    size_t maximum_;
};


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
    T<wxpex::MakeAsync<size_t, GaugeFilter>> value;
    T<wxpex::MakeAsync<size_t>> maximum;
};


using GaugeGroup = pex::Group<GaugeFields, GaugeTemplate>;


struct GaugeModel: public GaugeGroup::Model
{
    GaugeModel()
        :
        GaugeGroup::Model(),
        internalMaximum_(this, this->maximum)
    {
        this->internalMaximum_.Connect(&GaugeModel::OnMaximum_);
    }

private:
    void OnMaximum_(size_t maximumValue)
    {
        this->value.SetFilter(GaugeFilter(maximumValue));
    }

    using Internal =
        pex::Terminus<GaugeModel, wxpex::Async<size_t>>;

    Internal internalMaximum_;
};


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
using GaugeGroupMaker = pex::MakeGroup<GaugeGroup, GaugeModel>;


class Gauge: public wxGauge
{
public:
    static constexpr auto observerName = "wxpex::Gauge";

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
