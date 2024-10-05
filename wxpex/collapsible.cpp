#include <jive/scope_flag.h>
#include "wxpex/collapsible.h"
#include "wxpex/layout_top_level.h"
#include "wxpex/size.h"
#include "wxpex/border_sizer.h"
#include "wxpex/scrolled.h"
#include "wxpex/static_box.h"
#include "wxpex/splitter.h"
#include "wxpex/freezer.h"


namespace wxpex
{


Collapsible::Collapsible(
    wxWindow *parent,
    const std::string &label,
    long borderStyle)
    :
    wxCollapsiblePane(
        parent,
        wxID_ANY,
        label,
        wxDefaultPosition,
        wxDefaultSize,
        wxTAB_TRAVERSAL | wxCP_NO_TLW_RESIZE),
    Expandable(),
    borderPane_(nullptr),
    label_(label),
    stateEndpoint_(),
    hasStateEndpoint_(false),
    ignoreState_(false)
{
    if (borderStyle != wxBORDER_NONE)
    {
        this->borderPane_ = new wxPanel(
            this->m_pPane,
            wxID_ANY,
            wxDefaultPosition,
            wxDefaultSize,
            borderStyle);

        this->SetExpandableWindow(this->borderPane_);
    }
    else
    {
        this->SetExpandableWindow(this->m_pPane);
    }

    this->Bind(
        wxEVT_COLLAPSIBLEPANE_CHANGED,
        &Collapsible::OnChanged_,
        this);
}


Collapsible::Collapsible(
    wxWindow *parent,
    const std::string &label,
    StateControl stateControl,
    long borderStyle)
    :
    Collapsible(parent, label, borderStyle)
{
    this->stateEndpoint_ =
        StateEndpoint(this, stateControl, &Collapsible::OnState_);
    
    this->hasStateEndpoint_ = true;

    this->OnState_(stateControl.Get());
}


wxWindow * Collapsible::GetPanel()
{
    if (this->borderPane_)
    {
        return this->borderPane_;
    }

    return this->m_pPane;
}


void Collapsible::ConfigureBorderPane_(
    std::unique_ptr<wxSizer> &&sizer,
    int pixels)
{
    auto borderSizer = wxpex::BorderSizer(std::move(sizer), pixels);


    this->borderPane_->SetSizer(borderSizer.get());
    // borderSizer->SetSizeHints(this->borderPane_);
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

    this->m_pPane->SetSizer(paneSizer.get());
    paneSizer->Fit(this->m_pPane);
    paneSizer.release();
}


void Collapsible::ConfigureSizer(std::unique_ptr<wxSizer> &&sizer)
{
    if (this->borderPane_)
    {
        this->ConfigureBorderPane_(std::move(sizer), 3);
        return;
    }

#ifdef __WXMSW__
    // Add correction to prevent panel members from running off the right edge
    // of the pane.
    auto correctionSizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    correctionSizer->Add(sizer.release(), 1, wxEXPAND | wxRIGHT | wxBOTTOM, 5);
    this->m_pPane->SetSizer(correctionSizer.get());
    correctionSizer->Fit(this->m_pPane);
    correctionSizer.release();
#else
    this->m_pPane->SetSizer(sizer.get());
    sizer->Fit(this->m_pPane);
    sizer.release();
#endif
}


void Collapsible::UpdateMinimumSize_() const
{
    if (this->borderPane_)
    {
        // If the borderPane_ has changed size, the sizer on its parent
        // (m_pPane) stubbornly refuses to acknowledge it, and continues to
        // report the initial minimum size.
        auto borderBestSize = this->borderPane_->GetBestSize();
        wxSizer *paneSizer = this->m_pPane->GetSizer();

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
    auto paneBestSize = this->m_pPane->GetBestSize();
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


void Collapsible::HandleStateChange_()
{
    // Freezer freezer(this);

    this->UpdateMinimumSize_();

    this->FixLayout();

    if (this->ignoreState_)
    {
        return;
    }

    if (this->hasStateEndpoint_)
    {
        jive::ScopeFlag ignoreState(this->ignoreState_);
        this->stateEndpoint_.Set(this->IsExpanded());
    }
}


void Collapsible::OnChanged_(wxCollapsiblePaneEvent &event)
{
    this->HandleStateChange_();

    // Allow parent windows to listen for this event.
    event.Skip();
}


void Collapsible::OnState_(bool isExpanded)
{
    if (this->ignoreState_)
    {
        return;
    }

    jive::ScopeFlag ignoreState(this->ignoreState_);
    this->Collapse(!isExpanded);
    this->HandleStateChange_();
}


} // end namespace wxpex
