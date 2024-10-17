#pragma once


#include "wxpex/wxshim.h"


namespace wxpex
{


class Expandable
{
public:
    Expandable();

    Expandable(wxWindow *window);

    void SetExpandableWindow(wxWindow *window);

    void FixLayout();

    static void ReportWindowSize_(wxWindow *window, size_t depth);
    static void FixExpandableSize_(wxWindow *window);
    static void FixContainerSize_(wxWindow *window);

private:
    wxWindow *window_;
};


} // end namespace wxpex