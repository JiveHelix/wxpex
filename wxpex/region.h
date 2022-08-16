#pragma once

#include <pex/group.h>
#include <tau/region.h>

#include "wxpex/wxshim.h"
#include "wxpex/point.h"
#include "wxpex/size.h"


namespace wxpex
{


template<typename T, typename Style = tau::Round>
wxRect ToWxRect(const tau::Region<T> &region)
{
    return wxRect(
        ToWxPoint(region.topLeft),
        ToWxSize(region.size));
}


template<typename T>
tau::Region<T> ToRegion(const wxRect &rect)
{
    return {{ToWxPoint<T>(rect.GetTopLeft()), ToWxSize<T>(rect.GetSize())}};
}


} // end namespace wxpex
