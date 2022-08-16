/**
  * @file array_string.h
  *
  * @brief Convert any iterable container of std::string's to a wxArrayString.
  *
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 14 Aug 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#pragma once

#include <vector>

#include "wxpex/wxshim.h"


namespace wxpex
{


template<typename Converter, typename Vector>
wxArrayString MakeArrayString(Vector &&items)
{
    wxArrayString result;
    result.Alloc(items.size());

    for (auto &it: items)
    {
        result.Add(Converter::ToString(it));
    }

    return result;
}


} // namespace wxpex
