#pragma once


#include <pex/converter.h>


namespace wxpex
{


template<int base, int width, int precision>
struct ViewTraits:
    pex::ConverterTraits<base, width, precision, jive::flag::None>
{

};


template<typename Control, int precision>
using PrecisionConverter =
    pex::Converter<typename Control::Type, ViewTraits<10, 0, precision>>;


template<typename Control, int width>
using WidthConverter =
    pex::Converter<typename Control::Type, ViewTraits<10, width, -1>>;


template<typename Control, int width, int precision>
using Converter =
    pex::Converter<typename Control::Type, ViewTraits<10, width, precision>>;


} // end namespace wxpex
