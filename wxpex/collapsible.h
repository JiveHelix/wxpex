#pragma once


#include "wxpex/wxshim.h"
#include <wx/collpane.h>


namespace wxpex
{


class Collapsible: public wxCollapsiblePane
{
public:
    Collapsible(wxWindow *parent, const std::string &label);

    void ConfigureTopSizer(wxSizer *sizer);

protected:
    void OnChanged_(wxCollapsiblePaneEvent &);
};


} // end namespace wxpex
