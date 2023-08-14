#include "wxpex/collapsible.h"
#include "wxpex/layout_top_level.h"
#include "wxpex/size.h"
#include "wxpex/border_sizer.h"


namespace wxpex
{


Collapsible::Collapsible(
    wxWindow *parent,
    const std::string &label)
    :
    wxCollapsiblePane(
        parent,
        wxID_ANY,
        label,
        wxDefaultPosition,
        wxDefaultSize,
        wxTAB_TRAVERSAL | wxCP_NO_TLW_RESIZE),
    borderPane_(nullptr),
    label_(label)
{
    this->Bind(
        wxEVT_COLLAPSIBLEPANE_CHANGED,
        &Collapsible::OnChanged_,
        this);
}


wxPanel * Collapsible::GetBorderPane(long borderStyle)
{
    if (this->borderPane_)
    {
        return this->borderPane_;
    }

    return this->borderPane_ = new wxPanel(
        this->GetPane(),
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        borderStyle);
}


void Collapsible::ConfigureBorderPane(
    int pixels,
    std::unique_ptr<wxSizer> &&sizer)
{
    auto borderSizer = wxpex::BorderSizer(std::move(sizer), pixels);

    this->borderPane_->SetSizer(borderSizer.get());
    borderSizer->SetSizeHints(this->borderPane_);
    borderSizer.release();

    auto paneSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    paneSizer->Add(this->borderPane_, 1, wxEXPAND);
    this->ConfigureTopSizer(std::move(paneSizer));
}


void Collapsible::ConfigureTopSizer(std::unique_ptr<wxSizer> &&sizer)
{
    this->GetPane()->SetSizer(sizer.get());
    sizer->Fit(this->GetPane());
    sizer.release();
}


void Collapsible::UpdateMinimumSize_() const
{
    if (this->borderPane_)
    {
        // If the borderPane_ has changed size, the sizer on its parent
        // (GetPane()) stubbornly refuses to acknowledge it, and continues to
        // report the initial minimum size.
        auto borderBestSize = this->borderPane_->GetBestSize();
        wxSizer *paneSizer = this->GetPane()->GetSizer();

        if (paneSizer)
        {
            paneSizer->SetMinSize(borderBestSize);
        }
    }
}

#ifdef __WXGTK__

wxSize Collapsible::DoGetBestSize() const
{
    // Always require the minimum width of expanded children.
    auto paneBestSize = this->GetPane()->GetBestSize();
    wxSize superBestSize = this->wxCollapsiblePane::DoGetBestSize();
    superBestSize.SetWidth(paneBestSize.GetWidth());

    return superBestSize;
}

#else

wxSize Collapsible::DoGetBestClientSize() const
{
    // Always require the minimum width of expanded children.
    auto paneBestSize = this->GetPane()->GetBestSize();
    wxSize superBestSize = this->wxCollapsiblePane::DoGetBestClientSize();
    superBestSize.SetWidth(paneBestSize.GetWidth());

    return superBestSize;
}

#endif

void Collapsible::OnChanged_(wxCollapsiblePaneEvent &event)
{
    // Allow parent windows to listen for this event.
    event.Skip();

    this->UpdateMinimumSize_();

    // Invalide best size of all parents.
    wxWindow *window;

    if (this->borderPane_)
    {
        window = this->borderPane_;
    }
    else
    {
        window = this->GetPane();
    }

#if 1
    window->InvalidateBestSize();
#else
    auto top = wxGetTopLevelParent(window);

    for (; window != top; window = window->GetParent())
    {
        window->InvalidateBestSize();
    }
#endif

    wxpex::LayoutTopLevel(this);
}


} // end namespace wxpex
