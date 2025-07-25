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
concept IsIndex = std::is_same_v<T, size_t>;


template<typename T>
concept HasGetStorageIndex = requires(T t)
{
    { t.GetStorageIndex(std::declval<size_t>()) } -> IsIndex;
};


template<typename T>
concept HasGetOrderedIndex = requires(T t)
{
    { t.GetOrderedIndex(std::declval<size_t>()) } -> IsIndex;
};


template<typename Control>
size_t GetStorageIndex(const Control &control, size_t orderedIndex)
{
    if constexpr (HasGetStorageIndex<Control>)
    {
        // This list provides a mapping between the ordering index and the
        // storage index.
        return control.GetStorageIndex(orderedIndex);
    }
    else
    {
        // The ordering index is the same as the storage index.
        return orderedIndex;
    }
}


template<typename Control>
size_t GetOrderedIndex(const Control &control, size_t storageIndex)
{
    if constexpr (HasGetOrderedIndex<Control>)
    {
        // This list provides a mapping between the ordering index and the
        // storage index.
        return control.GetOrderedIndex(storageIndex);
    }
    else
    {
        // The ordering index is the same as the storage index.
        return storageIndex;
    }
}


class ModificationGuard
{
public:
    ModificationGuard(
        std::condition_variable &condition,
        bool &flag)
        :
        condition_(condition),
        flag_(flag)
    {
        assert(this->flag_);
    }

    ~ModificationGuard()
    {
        this->flag_ = false;
        this->condition_.notify_one();
    }

private:
    std::condition_variable & condition_;
    bool & flag_;
};


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
            USE_REGISTER_PEX_NAME(this, "ListView"),
            control,
            &ListView::OnMemberAdded_,
            &ListView::OnMemberWillRemove_,
            &ListView::OnMemberRemoved_),

        mutex_(),
        pendingModification_(false),
        pendingRemoval_(false),
        condition_(),
        removalCondition_(),
        creationIndex_(),
        removalIndex_(),
        viewCount_(0),
        views_(),
        sizer_(),
        onReorder_(),

        createInterface_(std::bind(&ListView::CreateInterface_, this)),
        createSingleView_(std::bind(&ListView::CreateSingleView_, this)),
        destroySingleView_(std::bind(&ListView::DestroySingleView_, this)),

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
        std::unique_lock lock(this->mutex_);

        this->condition_.wait(
            lock,
            [this]() -> bool
            {
                return !this->pendingModification_;
            });

        this->pendingModification_ = true;
        lock.unlock();

        this->CreateInterface_();
    }

    void OnReorder_()
    {
        if (!wxIsMainThread())
        {
            throw std::logic_error(
                "Changes to the GUI must be made on the main thread.");
        }

        std::unique_lock lock(this->mutex_);

        this->condition_.wait(
            lock,
            [this]() -> bool
            {
                return !this->pendingModification_;
            });

        this->pendingModification_ = true;

        // Nothing is destroyed, only reordered.
        for (auto it: this->views_)
        {
            this->sizer_->Detach(it);
        }

        size_t count = this->listControl_.count.Get();

        assert(this->sizer_->IsEmpty());

        for (size_t i = 0; i < count; ++i)
        {
            auto storageIndex = ::wxpex::GetStorageIndex(this->listControl_, i);

            this->sizer_->Add(
                this->views_.at(storageIndex),
                0,
                this->flags_,
                this->spacing_);
        }

        this->Layout();

        this->pendingModification_ = false;
        this->condition_.notify_one();
    }

    void DestroySingleView_()
    {
        std::lock_guard lock(this->mutex_);

        ModificationGuard guardModification(
            this->condition_,
            this->pendingModification_);

        ModificationGuard guardRemoval(
            this->removalCondition_,
            this->pendingRemoval_);

        if (this->IsBeingDeleted())
        {
            return;
        }

        if (!this->removalIndex_.has_value())
        {
            return;
        }

        // Consume the next removal index.
        size_t removalIndex = *this->removalIndex_;

        auto view = this->views_.at(removalIndex);
        jive::SafeErase(this->views_, removalIndex);

        this->sizer_->Detach(view);
        view->Destroy();

        assert(this->viewCount_ > 0);
        this->viewCount_ = this->viewCount_ - 1;
        this->removalIndex_ = std::nullopt;;
    }

    void CreateSingleView_()
    {
        std::lock_guard lock(this->mutex_);

        ModificationGuard guard(this->condition_, this->pendingModification_);

        if (!this->creationIndex_.has_value())
        {
            return;
        }

        // Get the next view creation index.
        size_t nextIndex = *this->creationIndex_;

        auto orderedIndex = GetOrderedIndex(this->listControl_, nextIndex);
        auto &it = this->listControl_.at(orderedIndex);
        auto view = this->CreateView_(it, orderedIndex);

        REGISTER_PEX_NAME(view, "List view");

        this->views_.insert(
            jive::SafeInsertIterator(this->views_, nextIndex),
            view);

        this->sizer_->Insert(
            orderedIndex,
            view,
            0,
            this->flags_,
            this->spacing_);

        this->FixLayout();

        this->viewCount_ += 1;
        this->creationIndex_ = std::nullopt;
    }

    void CreateInterface_()
    {
        std::lock_guard lock(this->mutex_);

        ModificationGuard guard(this->condition_, this->pendingModification_);

        size_t count = this->listControl_.count.Get();

        assert(this->sizer_->IsEmpty());

        this->views_.resize(count);

        for (size_t i = 0; i < count; ++i)
        {
            auto &it = this->listControl_[i];
            auto view = this->CreateView_(it, i);
            auto storageIndex = ::wxpex::GetStorageIndex(this->listControl_, i);
            this->views_[storageIndex] = view;
            this->sizer_->Add(view, 0, this->flags_, this->spacing_);
        }

        this->viewCount_ = count;

        this->FixLayout();
    }

    void OnMemberAdded_(const std::optional<size_t> &index)
    {
        if (!index)
        {
            return;
        }

        {
            std::unique_lock lock(this->mutex_);

            this->condition_.wait(
                lock,
                [this]() -> bool
                {
                    return !this->pendingModification_;
                });

            this->pendingModification_ = true;
            this->creationIndex_ = index;
        }

        if (wxIsMainThread())
        {
            // Call CreateSingleView_ synchronously.
            this->CreateSingleView_();
        }
        else
        {
            // Allow the view to be created on the main thread.
            this->createSingleView_();
        }
    }

    void OnMemberWillRemove_(const std::optional<size_t> &index)
    {
        if (!index)
        {
            return;
        }

        {
            std::unique_lock lock(this->mutex_);

            this->condition_.wait(
                lock,
                [this]() -> bool
                {
                    return !this->pendingModification_;
                });

            this->pendingModification_ = true;
            this->pendingRemoval_ = true;
            this->removalIndex_ = index;
        }

        if (wxIsMainThread())
        {
            // Call DestroySingleView_ synchronously.
            this->DestroySingleView_();
        }
        else
        {
            std::unique_lock lock(this->mutex_);

            // All the view to be destroyed on the main thread.
            this->destroySingleView_();

            // Wait for the view to be destroyed.
            this->removalCondition_.wait(
                lock,
                [this]() -> bool
                {
                    return !this->pendingRemoval_;
                });
        }
    }

    void OnMemberRemoved_(const std::optional<size_t> &)
    {
        this->Layout();
    }

protected:
    ListControl listControl_;
    ListObserver listObserver_;
    std::mutex mutex_;
    bool pendingModification_;
    bool pendingRemoval_;
    std::condition_variable condition_;
    std::condition_variable removalCondition_;
    std::optional<size_t> creationIndex_;
    std::optional<size_t> removalIndex_;
    size_t viewCount_;
    std::vector<wxWindow *> views_;
    wxBoxSizer *sizer_;

private:
    using ReorderEndpoint = pex::Endpoint<ListView, Reorder>;

    ReorderEndpoint onReorder_;
    wxpex::CallAfter createInterface_;
    wxpex::CallAfter createSingleView_;
    wxpex::CallAfter destroySingleView_;

    int flags_;
    int spacing_;
};


} // end namespace wxpex
