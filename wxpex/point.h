#pragma once

#include <tau/vector2d.h>

#include "wxpex/wxshim.h"


namespace wxpex
{


template<typename T, typename Style = tau::Round>
wxPoint ToWxPoint(const tau::Point2d<T> &point)
{
    if constexpr (std::is_same_v<int, T>)
    {
        // No conversion necessary
        return wxPoint(point.x, point.y);
    }
    else
    {
        return ToWxPoint(point.template Cast<int, Style>());
    }
}


template<typename T>
tau::Point2d<T> ToPoint(const wxPoint &point)
{
    // Convert a wxPoint to the desired tau::Point2d type.
    return tau::Point2d<T>(tau::Point2d<int>(point.x, point.y));
}


} // end namespace wxpex
