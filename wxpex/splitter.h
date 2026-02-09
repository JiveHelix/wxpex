#pragma once


#include <wxpex/ignores.h>
WXSHIM_PUSH_IGNORES
#include <wx/splitter.h>
WXSHIM_POP_IGNORES


namespace wxpex
{


class Splitter: public wxSplitterWindow
{
public:
    Splitter(wxWindow *parent, long style = wxSP_3DSASH | wxSP_LIVE_UPDATE);

public:
    bool Layout() override;
    bool OnSashPositionChange(int newSashPosition) override;

    void SplitVerticallyLeft(wxWindow *left, wxWindow *right);
    void SplitVerticallyCenter(wxWindow *left, wxWindow *right);
    void SplitVerticallyRight(wxWindow *left, wxWindow *right);
    void SplitHorizontallyTop(wxWindow *top, wxWindow *bottom);
    void SplitHorizontallyCenter(wxWindow *top, wxWindow *bottom);
    void SplitHorizontallyBottom(wxWindow *top, wxWindow *bottom);

private:
    void SetSashWithWindow_(wxWindow *window, bool negativeSash);
    void CenterSash_();

    bool CheckSashWithWindow_(
        wxWindow *window,
        bool leftOrTop,
        int newSashPosition);

    void OnDoubleClick_(wxMouseEvent &);
    void OnSize_(wxSizeEvent &);

private:
    wxWindow *firstPriority_;
    wxWindow *secondPriority_;
    int userSashPosition_;
};


} // end namespace wxpex
