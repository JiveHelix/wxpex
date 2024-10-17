#pragma once

#include <jive/type_traits.h>
#include <tau/vector2d.h>

#include "wxpex/wxshim.h"
#include <wx/geometry.h>


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
wxPoint2DDouble ToWxPoint2DDouble(const tau::Point2d<T> &point)
{
    if constexpr (std::is_same_v<double, T>)
    {
        // No conversion necessary
        return wxPoint2DDouble(point.x, point.y);
    }
    else
    {
        return ToWxPoint2DDouble(point.template Cast<double>());
    }
}


template<typename T, typename wxType>
tau::Point2d<T> ToPoint(const wxType &point)
{
    if constexpr (std::is_convertible_v<wxType, wxPoint>)
    {
        static_assert(!std::is_same_v<wxType, wxPoint2DDouble>);

        // Convert a wxPoint to the desired tau::Point2d type.
        return tau::Point2d<T>(tau::Point2d<int>(point.x, point.y));
    }
    else
    {
        return tau::Point2d<T>(point.m_x, point.m_y);
    }
}


} // end namespace wxpex
