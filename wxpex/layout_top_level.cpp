#include "wxpex/layout_top_level.h"


namespace wxpex
{


// Call Layout on the top level parent of window.
void LayoutTopLevel(wxWindow *window)
{
    auto topLevel = wxGetTopLevelParent(window);

    if (!topLevel)
    {
        return;
    }

    topLevel->Layout();
}


} // end namespace wxpex
