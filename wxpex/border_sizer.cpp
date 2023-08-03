#include "wxpex/border_sizer.h"


namespace wxpex
{


std::unique_ptr<wxSizer> BorderSizer(
    int pixels,
    std::unique_ptr<wxSizer> &&toWrap)
{
    auto border = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    border->Add(toWrap.release(), 1, wxEXPAND | wxALL, pixels);

    return border;
}


} // end namespace wxpex
