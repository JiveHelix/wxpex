/**
  * @file tile.h
  * 
  * @brief Tile windows to fill the screen.
  * 
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 28 Mar 2022
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#pragma once

#include <vector>
#include "wxpex/wxshim.h"


namespace wxpex
{


void ScaleWindow(wxWindow *window, int size, int orient);


void Tile(const std::vector<wxWindow *> windows, int orient);


} // end namespace wxpex
