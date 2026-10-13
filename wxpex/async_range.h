#pragma once

#include <chrono>
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
using AsyncBounded = pex::MakeBoundedOptions
<
    pex::BoundedOptions
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
using DefaultAsyncBounded =
    AsyncBounded<T, pex::DefaultLimit, pex::DefaultLimit>;


} // end namespace wxpex
