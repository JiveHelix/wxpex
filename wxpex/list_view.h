#pragma once


#include <mutex>
#include <condition_variable>
#include <pex/signal.h>
#include <pex/endpoint.h>
#include <pex/list_observer.h>
#include <pex/ordered_list.h>
#include <wxpex/async.h>
#include <wxpex/expandable.h>
#include <wxpex/freezer.h>
#include <wxpex/scrolled.h>
#include <wxpex/border_sizer.h>
#include <wxpex/widget_names.h>


namespace wxpex
{


template<typename T>
concept IsId =
    std::is_convertible_v<ssize_t, decltype(std::declval<T>().Get())>;


template<typename T>
concept HasVirtualGetId = requires(T t)
{
    { t.GetVirtual()->GetId() } -> IsId;
};


template<typename T>
concept HasIdMember = requires (T t)
{
    { t.id } -> IsId;
};


template<typename T>
concept HasId =
    HasIdMember<T> || HasVirtualGetId<T>;


#if 0
template<typename ListMaker>
concept ListHasIdMember =
    HasIdMember<pex::ListControlItem<ListMaker>>
    && HasIdMember<pex::ListModelItem<ListMaker>>;


template<typename ListMaker>
concept ListHasVirtualGetId =
    HasVirtualGetId<pex::ListControlItem<ListMaker>>
    && HasVirtualGetId<pex::ListModelItem<ListMaker>>;


template<typename ListMaker>
concept ListHasId =
    ListHasIdMember<ListMaker> || ListHasVirtualGetId<ListMaker>;
#endif


template<typename Control>
ssize_t GetId(const Control &control)
{
    static_assert(HasId<Control>);

    if constexpr (HasVirtualGetId<Control>)
    {
        return control.GetVirtual()->GetId();
    }
    else
    {
        return control.id.Get();
    }
}


template<typename ListControl>
class ListView: public wxPanel, public wxpex::Expandable
{
public:
    static constexpr auto observerName = "ListView";

    using ListObserver = pex::ListObserver<ListView, ListControl>;
    using Reorder = pex::control::Signal<>;
    using ListItem = typename ListControl::ListItem;

    ListView(
        wxWindow *parent,
        ListControl control,
        std::optional<Reorder> reorder = {})
        :
        wxPanel(parent, wxID_ANY),
        wxpex::Expandable(this),
        listControl_(control),

        listObserver_(
            this,
            control,
            &ListView::OnCountWillChange_,
            &ListView::OnCount_),

        isDestroyed_(false),
        pendingCreate_(false),
        mutex_(),
        condition_(),
        viewCount_(0),
        viewsById_(),
        sizer_(),
        onReorder_(),

        destroyInterface_(std::bind(&ListView::DestroyInterface_, this)),
        createInterface_(std::bind(&ListView::CreateInterface_, this)),

        destroyAndCreateInterface_(
            std::bind(&ListView::DestroyAndCreateInterface_, this)),

        flags_(wxEXPAND | wxBOTTOM),
        spacing_(3)
    {
        REGISTER_WIDGET_NAME(this, "ListView");

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        this->sizer_ = sizer.get();
        this->SetSizer(sizer.release());

        if (reorder)
        {
            this->onReorder_ =
                ReorderEndpoint(this, *reorder, &ListView::OnReorder_);
        }
    }

    void SetFlags(int flags)
    {
        this->flags_ = flags;
        this->OnReorder_();
    }

    void SetSpacing(int spacing)
    {
        this->spacing_ = spacing;
        this->OnReorder_();
    }

protected:
    virtual wxWindow * CreateView_(ListItem &itemControl, size_t index) = 0;

    void Initialize_()
    {
        assert(this->sizer_->IsEmpty());
        std::lock_guard lock(this->mutex_);

        if (this->pendingCreate_)
        {
            return;
        }

        this->pendingCreate_ = true;
        this->createInterface_();
    }

    void OnReorder_()
    {
        {
            std::lock_guard lock(this->mutex_);

            if (this->pendingCreate_)
            {
                return;
            }

            this->pendingCreate_ = true;
        }

        if (!wxIsMainThread())
        {
            throw std::logic_error(
                "Changes to the GUI must be made on the main thread.");
        }

        if constexpr (!HasId<ListItem>)
        {
            // Destroy and recreate the interface later.
            // This could destroy the notifier that called this function.
            // By doing it later on the wx event loop we ensure that we are no
            // longer operating within the pex notification loop.
            this->destroyAndCreateInterface_();
        }
        else
        {
            // Nothing is destroyed, only reordered.
            std::lock_guard lock(this->mutex_);

            for (auto it: this->viewsById_)
            {
                this->sizer_->Detach(it.second);
            }

            size_t count = this->listControl_.count.Get();

            assert(this->sizer_->IsEmpty());

            for (size_t i = 0; i < count; ++i)
            {
                auto &it = this->listControl_[i];
                auto id = ::wxpex::GetId(it);

                this->sizer_->Add(
                    this->viewsById_.at(id),
                    0,
                    this->flags_,
                    this->spacing_);
            }

            this->Layout();

            this->pendingCreate_ = false;
        }
    }

    void DestroyInterface_()
    {
        std::lock_guard lock(this->mutex_);

        if (this->IsBeingDeleted())
        {
            return;
        }

        if constexpr (HasId<ListItem>)
        {
            for (auto it: this->viewsById_)
            {
                this->sizer_->Detach(it.second);
            }

            this->viewsById_.clear();

            this->DestroyChildren();
        }
        else
        {
            this->sizer_->Clear(true);
        }

        this->viewCount_ = 0;
        this->isDestroyed_ = true;
        this->condition_.notify_one();
    }

    void CreateViews_()
    {
        std::lock_guard lock(this->mutex_);

        size_t count = this->listControl_.count.Get();

        assert(this->sizer_->IsEmpty());

        for (size_t i = 0; i < count; ++i)
        {
            auto &it = this->listControl_[i];
            auto view = this->CreateView_(it, i);

            if constexpr (HasId<ListItem>)
            {
                auto id = ::wxpex::GetId(it);
                this->viewsById_[id] = view;
            }

            this->sizer_->Add(view, 0, this->flags_, this->spacing_);
        }

        this->viewCount_ = count;
    }

    void CreateInterface_()
    {
        // wxpex::Freezer freeze(this);
        this->CreateViews_();
        this->FixLayout();

        std::lock_guard lock(this->mutex_);
        this->pendingCreate_ = false;
    }

    void OnCountWillChange_()
    {
        if (wxIsMainThread())
        {
            // Process the destroyInterface request now.
            this->DestroyInterface_();

            return;
        }

        // else this is not the main application thread.
        // Queue the deletion for later and wait until it has completed.

        std::unique_lock lock(this->mutex_);
        this->isDestroyed_ = false;

        this->destroyInterface_();

        // Wait for the views to be destroyed.
        this->condition_.wait(
            lock,
            [this]() -> bool
            {
                return this->isDestroyed_;
            });
    }

    void OnCount_(size_t count)
    {
        size_t viewCount;

        {
            std::lock_guard lock(this->mutex_);

            if (this->pendingCreate_)
            {
                return;
            }

            this->pendingCreate_ = true;

            viewCount = this->viewCount_;
        }

        if ((viewCount > 0) && (viewCount != count))
        {
            std::cerr << "Warning: ListView did not receive countWillChange."
                << std::endl;

            this->OnCountWillChange_();
        }

        {
            std::lock_guard lock(this->mutex_);
            viewCount = this->viewCount_;
        }

        if (viewCount != count)
        {
            assert(this->sizer_->IsEmpty());

            if (wxIsMainThread())
            {
                // Call CreateInterface_ synchronously.
                this->CreateInterface_();
            }
            else
            {
                this->createInterface_();
            }
        }
    }

private:
    void DestroyAndCreateInterface_()
    {
        // OnCountWillChange waits until destroy is complete.
        this->OnCountWillChange_();
        assert(this->sizer_->IsEmpty());
        this->createInterface_();
    }

protected:
    ListControl listControl_;
    ListObserver listObserver_;
    bool isDestroyed_;
    bool pendingCreate_;
    std::mutex mutex_;
    std::condition_variable condition_;
    size_t viewCount_;
    std::map<ssize_t, wxWindow *> viewsById_;
    wxBoxSizer *sizer_;

private:
    using ReorderEndpoint = pex::Endpoint<ListView, Reorder>;

    ReorderEndpoint onReorder_;

    wxpex::CallAfter destroyInterface_;
    wxpex::CallAfter createInterface_;
    wxpex::CallAfter destroyAndCreateInterface_;

    int flags_;
    int spacing_;
};


} // end namespace wxpex
