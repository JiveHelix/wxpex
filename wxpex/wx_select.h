/**
  * @file wx_select.h
  *
  * @brief A utility to convert between pex::Select members and the wxString's
  * needed by the wxComboBox control.
  *
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 14 Aug 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#pragma once

#include <pex/converter.h>
#include "wxpex/array_string.h"
#include "wxpex/wxshim.h"


namespace wxpex
{


template<typename T, typename Convert = pex::Converter<T>>
struct WxSelect
{
    static wxString GetSelectionAsString(
        size_t index,
        const std::vector<T> &choices)
    {
        return wxString(Convert::ToString(choices[index]));
    }

    static wxArrayString GetChoicesAsStrings(const std::vector<T> &choices)
    {
        return MakeArrayString<Convert>(choices);
    }
};


} // namespace wxpex
