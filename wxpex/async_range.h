#pragma once

#include <wxpex/async.h>
#include <pex/interface.h>


namespace wxpex
{


template
<
    typename T,
    typename Low,
    typename High,
    typename Access = pex::GetAndSetTag
>
using AsyncRange = pex::MakeRange<T, Low, High, Access, wxpex::AsyncTypes>;


} // end namespace wxpex
