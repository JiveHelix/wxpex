#pragma once

#include <pex/group.h>
#include <pex/endpoint.h>

#include "wxpex/ignores.h"

WXSHIM_PUSH_IGNORES
#include <wx/gauge.h>
WXSHIM_POP_IGNORES

#include "wxpex/async.h"
#include "wxpex/style.h"


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

    static constexpr auto fields = GaugeFields<GaugeTemplate>::fields;
};


using GaugeGroup = pex::Group<GaugeFields, GaugeTemplate>;
using GaugeState = typename GaugeGroup::Plain;
using GaugeControl = typename GaugeGroup::Control;

static_assert(
    std::is_same_v
    <
        decltype(GaugeControl::maximum),
        typename Async<size_t>::Control
    >);

struct GaugeModel: public GaugeGroup::Model
{
    GaugeModel()
        :
        GaugeGroup::Model(GaugeState{{0, 1000}}),
        internalMaximum_(
            this,
            GaugeControl(*this).maximum,
            &GaugeModel::OnMaximum_)
    {

    }

private:
    void OnMaximum_(size_t maximumValue)
    {
        this->value.SetFilter(GaugeFilter(maximumValue));
    }

    using Internal =
        pex::Endpoint<GaugeModel, typename wxpex::Async<size_t>::Control>;

    Internal internalMaximum_;
};


struct GaugeWorker: public GaugeControl
{
public:
    GaugeWorker() = default;

    GaugeWorker(GaugeModel &model)
        :
        GaugeControl(model)
    {
        this->value = model.value.GetWorkerControl();
        this->maximum = model.maximum.GetWorkerControl();
    }
};


using GaugeGroupMaker = pex::MakeGroup<GaugeGroup, GaugeModel>;


class Gauge: public wxGauge
{
public:
    static constexpr auto observerName = "wxpex::Gauge";

    using Base = wxGauge;

    Gauge(
        wxWindow *parent,
        GaugeControl control,
        Style style = Style::horizontal);

private:
    void OnValue_(size_t value);

    void OnMaximum_(size_t maximum);

    pex::EndpointGroup<Gauge, GaugeControl> endpoints_;
};


class ValueGauge: public wxControl
{
public:
    using ValueControl = decltype(GaugeControl::value);

    ValueGauge(
        wxWindow *parent,
        GaugeControl control,
        Style style = Style::horizontal);

    void OnValue_(size_t);

private:
    pex::Terminus<ValueGauge, ValueControl> value_;
};


} // end namespace pex
