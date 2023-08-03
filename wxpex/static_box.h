#pragma once


#include <memory>
#include <wx/statbox.h>


namespace wxpex
{


class StaticBox: public wxStaticBox
{
public:
    StaticBox(wxWindow *parent, const std::string &label);
    void ConfigureSizer(std::unique_ptr<wxSizer> &&sizer);
};


} // end namespace wxpex
