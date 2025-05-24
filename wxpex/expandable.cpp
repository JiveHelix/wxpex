#include "wxpex/expandable.h"
#include <iostream>
#include "wxpex/size.h"
#include "wxpex/scrolled.h"
#include "wxpex/splitter.h"
#include "wxpex/static_box.h"
#include "wxpex/collapsible.h"
#include <pex/reference.h>

// #define VERBOSE_WIDGET_NAMES

#ifdef VERBOSE_WIDGET_NAMES
#include "wxpex/widget_names.h"

#define LOG_WIDGET_NAME(message, widget) \
    std::cout << message << GetWidgetName(widget) << std::endl;

#else

#define LOG_WIDGET_NAME(message, widget)

#endif


namespace wxpex
{


Expandable::Expandable()
    :
    window_(nullptr)
{

}


Expandable::Expandable(wxWindow *window)
    :
    window_(window)
{

}


void Expandable::SetExpandableWindow(wxWindow *window)
{
    this->window_ = window;
}


void Expandable::FixLayout()
{
    if (!this->window_)
    {
        throw std::logic_error("Unitialized class");
    }

#ifdef VERBOSE_WIDGET_NAMES
    PrintAncestry(std::cout, GetAncestry(this->window_)) << std::endl;
#endif

    // Invalidate best size of all parents.
    // And find the highest level window that is an instance of Expandable.
    auto top = wxGetTopLevelParent(this->window_);

    wxWindow *topExpandable = nullptr;
    wxWindow *topCollapsible = nullptr;

    std::vector<wxWindow *> allWindows;

    auto window = this->window_;

    for (; window != top; window = window->GetParent())
    {
        Expandable *maybeTopExpandable = dynamic_cast<Expandable *>(window);

        if (maybeTopExpandable)
        {
            topExpandable = window;
        }
        // else
        // Do not write over the stored pointer to the highest (so far)
        // Expandable parent.

        Collapsible *maybeTopCollapsible = dynamic_cast<Collapsible *>(window);

        if (maybeTopCollapsible)
        {
            topCollapsible = window;
        }

#if 0
        if (dynamic_cast<Splitter *>(window))
        {
            // Do not fix anything higher than a Splitter
            break;
        }
#endif

        // window->InvalidateBestSize();
        allWindows.push_back(window);

        LOG_WIDGET_NAME("Found window: ", window)
    }

#if 0
    if (topCollapsible)
    {
        LOG_WIDGET_NAME("topCollapsible: ", topCollapsible)

        wxWindow *parent = topCollapsible->GetParent();

        LOG_WIDGET_NAME("parent: ", parent)

        auto sizer = parent->GetParent()->GetSizer();

        if (sizer)
        {
            auto newSize = sizer->ComputeFittingClientSize(parent);
            parent->SetMinSize(newSize);
        }

        if (dynamic_cast<detail::StaticBoxInternal *>(parent))
        {
            // We must call SetMinSize on the wxpex::StaticBox that owns the
            // wxStaticBox.
            assert(dynamic_cast<StaticBox *>(parent->GetParent()));
            auto grandParent = parent->GetParent();

            LOG_WIDGET_NAME("grandParent: ", grandParent)
            auto grandParentSizer = grandParent->GetSizer();
            
            if (sizer)
            {
                auto newSize = sizer->ComputeFittingClientSize(grandParent);
                grandParent->SetMinSize(newSize);
            }
        }
    }
#endif

    auto topSize = wxpex::ToSize<int>(top->GetSize());

    // We expect to have encountered at least one pointer to ourselves in our
    // traversal up the family tree.
    assert(topExpandable);

    auto endOfExpandable = std::find(
        std::begin(allWindows),
        std::end(allWindows),
        topExpandable);

    // This window must have been added to allWindows.
    assert(endOfExpandable != std::end(allWindows));

    ++endOfExpandable;

    for (auto it = std::begin(allWindows); it != endOfExpandable; ++it)
    {
        this->FixExpandableSize_(*it);
    }

    for (auto it = endOfExpandable; it != std::end(allWindows); ++it)
    {
        this->FixContainerSize_(*it);
    }

    top->Layout();
#if 0
    auto postLayoutSize = wxpex::ToSize<int>(top->GetSize());

    // Do not make the top window smaller
    topSize.width = std::max(postLayoutSize.width, topSize.width);
    topSize.height = std::max(postLayoutSize.height, topSize.height);

    if (topSize != postLayoutSize)
    {
        top->SetSize(wxpex::ToWxSize(topSize));
    }
    else
    {
        top->SendSizeEvent();
    }
#endif
}


void Expandable::FixExpandableSize_(wxWindow *window)
{
    LOG_WIDGET_NAME("FixExpandableSize_: ", window)

    if (auto *scrolled = dynamic_cast<Scrolled *>(window))
    {
        window->Layout();
        scrolled->FitInside();

        return;
    }

    if (auto *splitter = dynamic_cast<Splitter *>(window))
    {
        if (auto *window1 = splitter->GetWindow1())
        {
            window1->Layout();
        }

        if (auto *window2 = splitter->GetWindow2())
        {
            window2->Layout();
        }

        // splitter->UpdateSize();
        return;
    }

    window->Layout();
}


void Expandable::FixContainerSize_(wxWindow *window)
{
    LOG_WIDGET_NAME("FixContainerSize_: ", window)

    if (auto *scrolled = dynamic_cast<Scrolled *>(window))
    {
        window->Layout();
        scrolled->FitInside();

        return;
    }

    if (auto *splitter = dynamic_cast<Splitter *>(window))
    {
        if (auto *window1 = splitter->GetWindow1())
        {
            window1->Layout();
        }

        if (auto *window2 = splitter->GetWindow2())
        {
            window2->Layout();
        }

        // splitter->UpdateSize();
        return;
    }

    window->Layout();
}


void Expandable::ReportWindowSize(wxWindow *window, size_t depth)
{
    std::cout << std::string(depth * 4, ' ') << depth << std::endl;

    auto bestSize = window->GetBestSize();
    auto size = window->GetSize();

    std::cout << std::string(depth * 4, ' ')
        << "best size: " << wxpex::ToSize<int>(bestSize) << std::endl;

    std::cout << std::string(depth * 4, ' ')
        << "size: " << wxpex::ToSize<int>(size) << std::endl;
}


} // end namespace wxpex
