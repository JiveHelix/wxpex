#include "wxpex/splitter.h"
#include "wxpex/size.h"


namespace wxpex
{


Splitter::Splitter(
    wxWindow *parent,
    long style)
    :
    wxSplitterWindow(
        parent,
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        style),
    firstPriority_(nullptr),
    secondPriority_(nullptr),
    userSashPosition_(-1)
{
    // this->SetAutoLayout(true);
}


void Splitter::SetSashWithWindow_(wxWindow *window, bool negativeSash)
{
    auto bestSize = window->GetBestSize();

    if (this->GetSplitMode() == wxSPLIT_VERTICAL)
    {
        this->SetSashPosition(
            (negativeSash)
                ? -bestSize.GetWidth()
                : bestSize.GetWidth());
    }
    else
    {
        this->SetSashPosition(
            (negativeSash)
                ? -bestSize.GetHeight()
                : bestSize.GetHeight());
    }
}


bool Splitter::CheckSashWithWindow_(
    wxWindow *window,
    bool leftOrTop,
    int newSashPosition)
{
    auto minimumSize = window->GetMinSize();
    int thisSize;
    int minimumChildSize;

    if (this->GetSplitMode() == wxSPLIT_VERTICAL)
    {
        thisSize = this->GetSize().GetWidth();
        minimumChildSize = minimumSize.GetWidth();
    }
    else
    {
        thisSize = this->GetSize().GetHeight();
        minimumChildSize = minimumSize.GetHeight();
    }

    if (leftOrTop)
    {
        return (newSashPosition >= minimumChildSize
            && newSashPosition < thisSize);
    }

    return (newSashPosition > 0
        && newSashPosition < thisSize - minimumChildSize);
}


bool Splitter::Layout()
{
    if (this->userSashPosition_ > -1)
    {
        // Once the user has selected their desired size, do not change it.
        this->wxSplitterWindow::Layout();

        return true;
    }

    if (this->firstPriority_)
    {
        this->SetSashWithWindow_(this->firstPriority_, false);
    }
    else if (this->secondPriority_)
    {
        this->SetSashWithWindow_(this->secondPriority_, true);
    }

    this->wxSplitterWindow::Layout();

    return true;
}


bool Splitter::OnSashPositionChange(int newSashPosition)
{
    bool result;

    if (this->firstPriority_)
    {
        result = this->CheckSashWithWindow_(
            this->firstPriority_,
            true,
            newSashPosition);
    }
    else if (this->secondPriority_)
    {
        result = this->CheckSashWithWindow_(
            this->secondPriority_,
            false,
            newSashPosition);
    }

    if (result)
    {
        this->userSashPosition_ = newSashPosition;
    }

    return result;
}


void Splitter::SplitVerticallyLeft(wxWindow *left, wxWindow *right)
{
    this->SplitVertically(left, right, 0);
    this->firstPriority_ = left;
    this->secondPriority_ = nullptr;

    this->Layout();
}


void Splitter::SplitVerticallyRight(wxWindow *left, wxWindow *right)
{
    this->SplitVertically(left, right, 0);
    this->firstPriority_ = nullptr;
    this->secondPriority_ = right;
    this->Layout();
}


void Splitter::SplitHorizontallyTop(wxWindow *top, wxWindow *bottom)
{
    this->SplitHorizontally(top, bottom, 0);
    this->firstPriority_ = top;
    this->secondPriority_ = nullptr;
    this->Layout();
}


void Splitter::SplitHorizontallyBottom(wxWindow *top, wxWindow *bottom)
{
    this->SplitHorizontally(top, bottom, 0);
    this->firstPriority_ = nullptr;
    this->secondPriority_ = bottom;
    this->Layout();
}


} // end namespace wxpex
