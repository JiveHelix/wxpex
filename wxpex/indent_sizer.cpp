#include "wxpex/indent_sizer.h"


namespace wxpex
{


std::unique_ptr<wxSizer> IndentSizer(
    int pixels,
    std::unique_ptr<wxSizer> &&toIndent)
{
    auto indented = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    indented->AddSpacer(pixels);
    indented->Add(toIndent.release(), 1, wxEXPAND);

    return indented;
}


} // end namespace wxpex
