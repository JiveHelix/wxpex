#pragma once

#include <string>
#include <pex/value.h>
#include <pex/choice_muxer.h>


enum class UnitSystem: uint8_t
{
    MKS,
    CGS,
    FPS,
    FFF
};


struct UnitHelpers
{
    static std::vector<UnitSystem> GetChoices()
    {
        return
            {
                UnitSystem::MKS,
                UnitSystem::CGS,
                UnitSystem::FPS,
                UnitSystem::FFF};
    }

    using Map = std::map<UnitSystem, std::string_view>;
    using ValueByName = std::map<std::string_view, UnitSystem>;

    static const Map shortNameByValue;
    static const Map longNameByValue;

    static ValueByName GetValueByName()
    {
        ValueByName result;

        for (const auto [value, name]: shortNameByValue)
        {
            result[name] = value;
        }

        return result;
    }

    static std::string ToString(UnitSystem value)
    {
        return std::string(UnitHelpers::shortNameByValue.at(value));
    }

    static UnitSystem ToValue(std::string_view asString)
    {
        static const auto valueByName = UnitHelpers::GetValueByName();

        return valueByName.at(asString);
    }
};


inline const UnitHelpers::Map UnitHelpers::shortNameByValue{
    {UnitSystem::MKS, "MKS"},
    {UnitSystem::CGS, "CGS"},
    {UnitSystem::FPS, "FPS"},
    {UnitSystem::FFF, "FFF"}};


inline const UnitHelpers::Map UnitHelpers::longNameByValue{
    {UnitSystem::MKS, "meter-kilogram-second"},
    {UnitSystem::CGS, "centimeter-gram-second"},
    {UnitSystem::FPS, "foot-pound-second"},
    {UnitSystem::FFF, "furlong-firkin-fortnight"}};


inline std::string ToString(UnitSystem value)
{
    return UnitHelpers::ToString(value);
}


inline UnitSystem ToValue(fields::Tag<UnitSystem>, std::string_view asString)
{
    return UnitHelpers::ToValue(asString);
}


struct ShortConverter
{
    static std::string ToString(UnitSystem value)
    {
        return UnitHelpers::ToString(value);
    }
};


struct LongConverter
{
    static std::string ToString(UnitSystem value)
    {
        return std::string(UnitHelpers::longNameByValue.at(value));
    }
};


using UnitsModel = pex::model::Value<UnitSystem>;
using UnitsControl = pex::control::Value<UnitsModel>;


using Select = pex::model::Select<UnitSystem, UnitHelpers>;
using SelectControl = pex::control::Select<Select>;


struct UnitSystemChoiceMaker
{
    using Key = bool;
    using Type = UnitSystem;
    using ChoiceMap = std::map<Key, std::vector<Type>>;

    static ChoiceMap GetChoiceMap()
    {
        return {
            {
                false,
                {
                    UnitSystem::MKS,
                    UnitSystem::CGS,
                    UnitSystem::FPS}},
            {
                true,
                {
                    UnitSystem::MKS,
                    UnitSystem::CGS,
                    UnitSystem::FPS,
                    UnitSystem::FFF}}};
    }
};


using UnitsMuxerModel = pex::ChoiceMuxerModel<UnitSystemChoiceMaker>;
using UnitsMuxerControl = pex::ChoiceMuxerControl<UnitSystemChoiceMaker>;
using UnitsMuxer = pex::ChoiceMuxer<UnitSystemChoiceMaker>;
