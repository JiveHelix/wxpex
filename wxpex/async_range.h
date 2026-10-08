#pragma once

#include <wxpex/async.h>
#include <pex/interface.h>


namespace wxpex
{


template
<
    typename T,
    pex::IsLimit Low,
    pex::IsLimit High,
    typename Access = pex::GetAndSetTag
>
using AsyncRange = pex::MakeRangeOptions
<
    pex::RangeOptions
    <
        T,
        Low,
        High,
        pex::NoFilter,
        wxpex::AsyncTypes
    >,
    Access
>;


template<typename T>
using DefaultAsyncRange = AsyncRange<T, pex::DefaultLimit, pex::DefaultLimit>;


} // end namespace wxpex
