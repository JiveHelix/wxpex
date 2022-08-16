#pragma once

#include <tau/point.h>

#include "wxpex/wxshim.h"


namespace wxpex
{


template<typename T, typename Style = tau::Round>
wxPoint ToWxPoint(const tau::Point<T> &point)
{
    if constexpr (std::is_same_v<int, T>)
    {
        // No conversion necessary
        return wxPoint(point.x, point.y);
    }
    else
    {
        return ToWxPoint(point.template Convert<int, Style>());
    }
}


template<typename T>
tau::Point<T> ToPoint(const wxPoint &point)
{
    // Convert a wxPoint to the desired tau::Point type.
    return tau::Point<T>(tau::Point<int>(point.y, point.x));
}


} // end namespace wxpex
