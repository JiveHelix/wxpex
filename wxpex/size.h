#pragma once

#include <pex/group.h>
#include <tau/size.h>

#include "wxpex/wxshim.h"


namespace wxpex
{


template<typename T, typename Style = tau::Round>
wxSize ToWxSize(const tau::Size<T> &size)
{
    if constexpr (std::is_same_v<int, T>)
    {
        // No conversion necessary
        return wxSize(size.width, size.height);
    }
    else
    {
        return ToWxSize(size.template Convert<int, Style>());
    }
}


template<typename T>
tau::Size<T> ToSize(const wxSize &size)
{
    // Convert a wxSize to the desired tau::Size type.
    return tau::Size<T>(tau::Size<int>(size.GetWidth(), size.GetHeight()));
}


} // end namespace wxpex
