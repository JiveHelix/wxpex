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
    hasPendingUpdate_(false),
    isInitialized_(false),
    userSashPosition_(-1)
{
    this->Bind(wxEVT_LEFT_DCLICK, &Splitter::OnDoubleClick_, this);
    this->Bind(wxEVT_SIZE, &Splitter::OnSize_, this);
}


void Splitter::OnDoubleClick_(wxMouseEvent &event)
{
    // Interrupt the nonsensical default behavior.
    // Reset the default sash position instead.
    this->userSashPosition_ = -1;
    this->LayoutSash();
}


void Splitter::OnSize_(wxSizeEvent &event)
{
    event.Skip();

    if (this->hasPendingUpdate_ || this->isInitialized_)
    {
        return;
    }

    this->hasPendingUpdate_ = true;

    this->CallAfter(
        [this]()
        {
            this->hasPendingUpdate_ = false;
            auto size = this->GetSize();

            if (size.GetWidth() < 20 || size.GetHeight() < 20)
            {
                return;
            }

            this->LayoutSash();
            this->isInitialized_ = true;
        });
}


void Splitter::SetSashWithWindow_(wxWindow *window, bool negativeSash)
{
    auto bestSize = window->GetBestSize();

    if (this->GetSplitMode() == wxSPLIT_VERTICAL)
    {
        this->SetSashPosition(
            (negativeSash)
                ? -bestSize.GetWidth()
                : bestSize.GetWidth(),
            true);
    }
    else
    {
        this->SetSashPosition(
            (negativeSash)
                ? -bestSize.GetHeight()
                : bestSize.GetHeight(),
            true);
    }
}


void Splitter::CenterSash_()
{
    auto clientSize = this->GetClientSize();

    if (this->GetSplitMode() == wxSPLIT_VERTICAL)
    {
        this->SetSashPosition(clientSize.GetWidth() / 2);
    }
    else
    {
        this->SetSashPosition(clientSize.GetHeight() / 2);
    }
}


bool Splitter::CheckSashWithWindow_(
    wxWindow *window,
    bool leftOrTop,
    int newSashPosition)
{
    auto minimumSize = window->GetBestSize();
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


void Splitter::LayoutSash()
{
    if (this->firstPriority_)
    {
        this->SetSashWithWindow_(this->firstPriority_, false);
    }
    else if (this->secondPriority_)
    {
        this->SetSashWithWindow_(this->secondPriority_, true);
    }
    else
    {
        // Neither half has priority.
        // Put the sash in the middle.
        this->CenterSash_();
    }
}


bool Splitter::Layout()
{
    this->wxSplitterWindow::Layout();

    if (this->userSashPosition_ > -1)
    {
        // Once the user has selected their desired size, do not change it.
        return true;
    }

    this->LayoutSash();

    return true;
}


bool Splitter::OnSashPositionChange(int newSashPosition)
{
    bool result{};

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
    else
    {
        result = true;
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


void Splitter::SplitVerticallyCenter(wxWindow *left, wxWindow *right)
{
    this->SplitVertically(left, right, 0);
    this->firstPriority_ = nullptr;
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


void Splitter::SplitHorizontallyCenter(wxWindow *top, wxWindow *bottom)
{
    this->SplitHorizontally(top, bottom, 0);
    this->firstPriority_ = nullptr;
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
