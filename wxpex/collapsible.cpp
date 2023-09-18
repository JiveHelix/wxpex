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
    label_(label),
    doLayoutTopLevel_(
        [this]() -> void
        {
            wxpex::LayoutTopLevel(this);
        })
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
    std::unique_ptr<wxSizer> &&sizer,
    int pixels)
{
    auto borderSizer = wxpex::BorderSizer(std::move(sizer), pixels);


    this->borderPane_->SetSizer(borderSizer.get());
    borderSizer->SetSizeHints(this->borderPane_);
    borderSizer.release();

    auto paneSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

#ifdef __WXMSW__
    // Add correction to prevent panel members from running off the right edge
    // of the pane.
    auto correctionSizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);

    correctionSizer->Add(
        this->borderPane_,
        1,
        wxEXPAND | wxRIGHT | wxBOTTOM,
        5);

    paneSizer->Add(correctionSizer.release(), 1, wxEXPAND);
#else
    paneSizer->Add(this->borderPane_, 1, wxEXPAND);
#endif

    this->GetPane()->SetSizer(paneSizer.get());
    paneSizer->Fit(this->GetPane());
    paneSizer.release();
}


void Collapsible::ConfigureTopSizer(std::unique_ptr<wxSizer> &&sizer)
{
#ifdef __WXMSW__
    // Add correction to prevent panel members from running off the right edge
    // of the pane.
    auto correctionSizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    correctionSizer->Add(sizer.release(), 1, wxEXPAND | wxRIGHT | wxBOTTOM, 5);
    this->GetPane()->SetSizer(correctionSizer.get());
    correctionSizer->Fit(this->GetPane());
    correctionSizer.release();
#else
    this->GetPane()->SetSizer(sizer.get());
    sizer->Fit(this->GetPane());
    sizer.release();
#endif
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

#if defined(__WXGTK__)

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
    // Adapted from the parent class's implementation of DoGetBestClientSize.
    auto size = this->m_sz->GetMinSize();
    size.SetWidth(std::max(size.x, this->m_pPane->GetBestSize().x));

    if (this->IsExpanded())
    {
        size.SetHeight(
            size.y + this->GetBorder() + this->m_pPane->GetBestSize().y);
    }

    return size;
}

#endif


void Collapsible::ReportWindowSize_(wxWindow *window, size_t depth)
{
    std::cout << std::string(depth * 4, ' ') << depth << std::endl;

    auto bestSize = window->GetBestSize();
    auto size = window->GetSize();

    std::cout << std::string(depth * 4, ' ')
        << "best size: " << wxpex::ToSize<int>(bestSize) << std::endl;

    std::cout << std::string(depth * 4, ' ')
        << "size: " << wxpex::ToSize<int>(size) << std::endl;
}


void Collapsible::FixWindowSize_(wxWindow *window)
{
    window->InvalidateBestSize();
    window->SetInitialSize();
}


void Collapsible::OnChanged_(wxCollapsiblePaneEvent &event)
{
    this->UpdateMinimumSize_();

    wxWindow *window;

    if (this->borderPane_)
    {
        window = this->borderPane_;
    }
    else
    {
        window = this->GetPane();
    }

    // Invalidate best size of all parents.
    auto top = wxGetTopLevelParent(window);

    std::vector<wxWindow *> windows;

    for (; window != top; window = window->GetParent())
    {
        windows.push_back(window);
    }

    windows.push_back(top);

    for (size_t depth = 0; depth < windows.size(); ++depth)
    {
        this->FixWindowSize_(windows[depth]);
    }

    this->doLayoutTopLevel_();

    // Allow parent windows to listen for this event.
    event.Skip();
}


} // end namespace wxpex
