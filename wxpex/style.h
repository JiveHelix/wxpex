#pragma once


#include "wxpex/ignores.h"

WXSHIM_PUSH_IGNORES
#include <wx/sizer.h>
WXSHIM_POP_IGNORES


namespace wxpex
{


enum class Style: long
{
    horizontal = wxHORIZONTAL,
    vertical = wxVERTICAL
};


inline long SliderStyle(Style style)
{
    return (Style::horizontal == style) ? wxSL_HORIZONTAL : wxSL_VERTICAL;
}


inline long GaugeStyle(Style style)
{
    return (Style::horizontal == style) ? wxGA_HORIZONTAL : wxGA_VERTICAL;
}


inline long SizerStyle(Style style)
{
    return static_cast<long>(style);
}


inline bool IsVertical(Style style)
{
    return (style == Style::vertical);
}


inline bool IsHorizontal(Style style)
{
    return (style == Style::horizontal);
}


} // end namespace style
