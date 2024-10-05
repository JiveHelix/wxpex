#pragma once


#include <memory>

#include "wxpex/ignores.h"

WXSHIM_PUSH_IGNORES
#include <wx/sizer.h>
#include <wx/scrolwin.h>
WXSHIM_POP_IGNORES


namespace wxpex
{


struct ScrolledSettings
{
    int horizontalRate;
    int verticalRate;
    int minimumWidth;
    int minimumHeight;
    int borderPixels;

    ScrolledSettings & HorizontalRate(int rate)
    {
        this->horizontalRate = rate;
        return *this;
    }

    ScrolledSettings & VerticalRate(int rate)
    {
        this->verticalRate = rate;
        return *this;
    }

    ScrolledSettings & MinimumWidth(int width)
    {
        this->minimumWidth = width;
        return *this;
    }

    ScrolledSettings & MinimumHeight(int height)
    {
        this->minimumHeight = height;
        return *this;
    }

    ScrolledSettings & BorderPixels(int border)
    {
        this->borderPixels = border;
        return *this;
    }
};


inline auto verticalScrolled = ScrolledSettings{0, 10, -1, 400, 5};
inline auto horizontalScrolled = ScrolledSettings{10, 0, 400, -1, 5};


class Scrolled: public wxScrolled<wxPanel>
{
public:
    Scrolled(wxWindow *parent);

    void ConfigureSizer(
        const ScrolledSettings &scrolledSettings,
        std::unique_ptr<wxSizer> &&sizer);
};


} // end namespace wxpex

