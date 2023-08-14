#include "wxpex/border_sizer.h"


namespace wxpex
{


std::unique_ptr<wxSizer> BorderSizer(
    std::unique_ptr<wxSizer> &&toWrap,
    int pixels)
{
    auto border = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    border->Add(toWrap.release(), 1, wxEXPAND | wxALL, pixels);

    return border;
}


std::unique_ptr<wxSizer> BorderSizer(
    wxWindow *window,
    int pixels)
{
    auto border = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    border->Add(window, 1, wxEXPAND | wxALL, pixels);

    return border;
}


} // end namespace wxpex
