#pragma once


#include <ostream>
#include <vector>
#include "wxpex/wxshim.h"


namespace wxpex
{


void RegisterWidgetName(wxWindow *window, const std::string &name);

std::string GetWidgetName(wxWindow *window);

std::vector<std::string> GetAncestry(wxWindow *window);

std::ostream & PrintAncestry(
    std::ostream &,
    const std::vector<std::string> &ancestry);


template<template<typename, typename> typename Widget, typename ...Args>
wxWindow * MakeWidget(
    const std::string &name,
    wxWindow *parent,
    Args &&...args)
{
    auto result = new Widget(parent, std::forward<Args>(args)...);
    RegisterWidgetName(result, name);

    return result;
}


} // end namespace wxpex
