#include "wxpex/shortcut.h"


namespace wxpex
{


std::string Key::GetString() const
{
    if (this->keyStringByKeyCode_.count(this->code_))
    {
        return this->keyStringByKeyCode_.at(this->code_);
    }

    if (this->code_ > 127)
    {
        throw std::runtime_error("Unsupported key code.");
    }

    return std::string(1, static_cast<char>(this->code_));
}

int Key::GetInt() const { return this->code_; }

const std::map<int, std::string> Key::keyStringByKeyCode_
{
    {WXK_DELETE, "DELETE"},
    {WXK_BACK, "BACK"},
    {WXK_INSERT, "INSERT"},
    {WXK_RETURN, "RETURN"},
    {WXK_PAGEUP, "PGUP"},
    {WXK_PAGEDOWN, "PGDN"},
    {WXK_LEFT, "LEFT"},
    {WXK_RIGHT, "RIGHT"},
    {WXK_UP, "UP"},
    {WXK_DOWN, "DOWN"},
    {WXK_HOME, "HOME"},
    {WXK_END, "END"},
    {WXK_SPACE, "SPACE"},
    {WXK_TAB, "TAB"},
    {WXK_ESCAPE, "ESCAPE"}
};


std::string GetModifierString(int modifierBitfield)
{
    static const std::map<wxAcceleratorEntryFlags, std::string>
    modifierStringByWxAccel
    {
        {wxACCEL_NORMAL, ""},
        {wxACCEL_SHIFT, "SHIFT"},
        {wxACCEL_CTRL, "CTRL"},
        {wxACCEL_ALT, "ALT"},
        {wxACCEL_RAW_CTRL, "RAWCTRL"}
    };

    static const std::vector<wxAcceleratorEntryFlags> modifierOrder
    {
        wxACCEL_CTRL,
        wxACCEL_SHIFT,
        wxACCEL_ALT
#ifdef __APPLE__
        , wxACCEL_RAW_CTRL
#endif
    };

    std::vector<std::string> modifiers;

    for (auto modifier: modifierOrder)
    {
        if (modifierBitfield & modifier)
        {
            modifiers.push_back(modifierStringByWxAccel.at(modifier));
        }
    }

    return jive::strings::Join(modifiers.begin(), modifiers.end(), '+');
}


void Shortcut::AddToMenu(wxMenu *menu)
{
    menu->Append(
        this->menuItem_ = new wxMenuItem(
            menu,
            this->id_,
            this->GetMenuItemLabel_(),
            this->longDescription_));
}

wxAcceleratorEntry Shortcut::GetAcceleratorEntry() const
{
    return wxAcceleratorEntry(
        this->modifier_,
        this->key_.GetInt(),
        this->id_,
        this->menuItem_);
}

int Shortcut::GetModifier() const { return this->modifier_; }

int Shortcut::GetKeyAsInt() const { return this->key_.GetInt(); }

int Shortcut::GetId() const { return this->id_; }

void Shortcut::OnEventMenu()
{
    this->signal_.Trigger();
}

wxString Shortcut::GetMenuItemLabel_() const
{
    auto modifier = GetModifierString(this->modifier_);

    std::string result(this->description_);

    if (!modifier.empty())
    {
        jive::strings::Concatenate(
            &result,
            '\t',
            modifier,
            '+',
            this->key_.GetString());
    }
    else
    {
        jive::strings::Concatenate(
            &result,
            '\t',
            this->key_.GetString());
    }

    return wxString(result);
}


ShortcutsBase::ShortcutsBase(
    const UnclosedWindow &window,
    const ShortcutGroups &groups)
    :
    window_(window),
    hasBindings_(false),
    groups_(groups)
{
    this->BindAll_();
}


ShortcutsBase::~ShortcutsBase()
{
    this->UnbindAll_();
}


void ShortcutsBase::EnsureIsShown_()
{
    auto wxWindow = this->window_.Get();

    if (!wxWindow->IsShown())
    {
        wxWindow->Show(true);
    }
}


void ShortcutsBase::BindAll_()
{
    auto window = this->window_.Get();

    if (!window || window->IsBeingDeleted())
    {
        std::cout << "Warning: ShortcutBase::BindAll_() without wxWindow"
            << std::endl;

        return;
    }

    window->Bind(
        wxEVT_CLOSE_WINDOW,
        &ShortcutsBase::OnWindowClose_,
        this);

    for (auto & shortcutGroup: this->groups_)
    {
        this->BindShortcuts_(window, shortcutGroup.shortcuts);
    }

    this->hasBindings_ = true;
}


void ShortcutsBase::UnbindAll_()
{
    auto window = this->window_.Get();

    if (!window || !this->hasBindings_)
    {
        return;
    }

    if (window && !window->IsBeingDeleted())
    {
        window->Unbind(
            wxEVT_CLOSE_WINDOW,
            &ShortcutsBase::OnWindowClose_,
            this);

        PEX_LOG("Unbind shortcuts");

        for (auto & shortcutGroup: this->groups_)
        {
            this->UnbindShortcuts_(window, shortcutGroup.shortcuts);
        }
    }

    this->hasBindings_ = false;
}


ShortcutsBase::ShortcutsBase(ShortcutsBase &&other)
    :
    window_(std::move(other.window_)),
    hasBindings_(other.hasBindings_),
    groups_(other.groups_)
{
    this->UnbindAll_();
    this->BindAll_();
}


ShortcutsBase & ShortcutsBase::operator=(ShortcutsBase &&other)
{
    other.UnbindAll_();
    this->UnbindAll_();
    this->window_ = std::move(other.window_);
    this->groups_ = std::move(other.groups_);
    this->BindAll_();

    return *this;
}


void ShortcutsBase::BindShortcuts_(wxWindow *window, Shortcuts &shortcuts)
{
    assert(window != nullptr);

    if (window && !window->IsBeingDeleted())
    {
        for (auto &shortcut: shortcuts)
        {
            window->Bind(
                wxEVT_MENU,
                ShortcutFunctor(shortcut),
                shortcut.GetId());
        }
    }
}

void ShortcutsBase::UnbindShortcuts_(wxWindow *window, Shortcuts &shortcuts)
{
    assert(window != nullptr);
    assert(!window->IsBeingDeleted());

    for (auto &shortcut: shortcuts)
    {
        window->Unbind(
            wxEVT_MENU,
            ShortcutFunctor(shortcut),
            shortcut.GetId());
    }
}


void ShortcutsBase::OnWindowClose_(wxCloseEvent &event)
{
    this->UnbindAll_();
    this->window_.Clear();
    event.Skip();
}


MenuShortcuts::MenuShortcuts(
    const UnclosedWindow &window,
    const ShortcutGroups &groups)
    :
    ShortcutsBase(window, groups),
    menuBar_(std::make_unique<wxMenuBar>())
{
    // Discovered the hard way that wx GTK will segfault on exit if we call
    // SetMenuBar on a top-level window before it is shown.
    // Because it doesn't make sense to create a menu bar for a hidden window,
    // this window will be shown.
    this->EnsureIsShown_();

    for (auto & shortcutGroup: this->groups_)
    {
        auto menu = std::make_unique<wxMenu>();
        this->AddToMenu(menu.get(), shortcutGroup.shortcuts);

        this->menuBar_->Append(
            menu.release(),
            wxString(std::forward<std::string>(shortcutGroup.name)));
    }
}


wxMenuBar * MenuShortcuts::GetMenuBar()
{
    return this->menuBar_.release();
}


void MenuShortcuts::AddToMenu(
    wxMenu *menu,
    Shortcuts &shortcuts)
{
    for (auto &shortcut: shortcuts)
    {
        shortcut.AddToMenu(menu);
    }
}


AcceleratorShortcuts::AcceleratorShortcuts(
    const UnclosedWindow &window,
    const ShortcutGroups &groups)
    :
    ShortcutsBase(std::move(window), groups),
    acceleratorTable_()
{
    std::vector<std::vector<wxAcceleratorEntry>> entries;
    size_t entryCount = 0;

    for (auto & shortcutGroup: this->groups_)
    {
        auto shortcutEntries =
            CreateAcceleratorEntries(shortcutGroup.shortcuts);

        entryCount += shortcutEntries.size();
        entries.push_back(shortcutEntries);
    }

    std::vector<wxAcceleratorEntry> allEntries;
    allEntries.reserve(entryCount);

    for (auto &it: entries)
    {
        allEntries.insert(allEntries.end(), it.begin(), it.end());
    }

    assert(allEntries.size() == entryCount);

    this->acceleratorTable_ = wxAcceleratorTable(
        static_cast<int>(entryCount),
        allEntries.data());
}


const wxAcceleratorTable & AcceleratorShortcuts::GetAcceleratorTable() const
{
    return this->acceleratorTable_;
}


std::vector<wxAcceleratorEntry>
AcceleratorShortcuts::CreateAcceleratorEntries(const Shortcuts &shortcuts)
{
    std::vector<wxAcceleratorEntry> entries;
    entries.reserve(shortcuts.size());

    for (auto &shortcut: shortcuts)
    {
        entries.push_back(shortcut.GetAcceleratorEntry());
    }

    return entries;
}


ShortcutWindow::ShortcutWindow()
    :
    closeRequested_(false),
    acceleratorShortcuts_(),
    eventHandler_()
{

}


ShortcutWindow::ShortcutWindow(
    wxWindow *window,
    const wxpex::ShortcutGroups &group)
    :
    UnclosedWindow(window),
    acceleratorShortcuts_(
        std::make_unique<AcceleratorShortcuts>(
            UnclosedWindow(window),
            group)),
    eventHandler_(new wxEvtHandler())
{
    window->SetAcceleratorTable(
        this->acceleratorShortcuts_->GetAcceleratorTable());

    this->eventHandler_->Bind(
        wxEVT_THREAD,
        &ShortcutWindow::OnCloseEvent_,
        this);
}


ShortcutWindow::ShortcutWindow(ShortcutWindow &&other)
    :
    UnclosedWindow(other),
    closeRequested_(false),
    acceleratorShortcuts_(std::move(other.acceleratorShortcuts_)),
    eventHandler_(std::move(other.eventHandler_))
{
    this->eventHandler_->Bind(
        wxEVT_THREAD,
        &ShortcutWindow::OnCloseEvent_,
        this);
}


ShortcutWindow & ShortcutWindow::operator=(ShortcutWindow &&other)
{
    this->UnclosedWindow::operator=(other);

    if (this->eventHandler_)
    {
        this->eventHandler_->Unbind(
            wxEVT_THREAD,
            &ShortcutWindow::OnCloseEvent_,
            this);
    }

    if (other.eventHandler_)
    {
        other.eventHandler_->Unbind(
            wxEVT_THREAD,
            &ShortcutWindow::OnCloseEvent_,
            &other);
    }

    this->acceleratorShortcuts_ = std::move(other.acceleratorShortcuts_);
    this->eventHandler_ = std::move(other.eventHandler_);

    if (this->eventHandler_)
    {
        this->eventHandler_->Bind(
            wxEVT_THREAD,
            &ShortcutWindow::OnCloseEvent_,
            this);
    }

    return *this;
}


void ShortcutWindow::Close()
{
    if (this->closeRequested_)
    {
        return;
    }

    if (!this->eventHandler_)
    {
        return;
    }

    this->eventHandler_->QueueEvent(new wxThreadEvent());

    this->closeRequested_ = true;
}


void ShortcutWindow::OnCloseEvent_(wxThreadEvent &)
{
    this->acceleratorShortcuts_.reset();
    this->UnclosedWindow::Close();
}


} // end namespace wxpex
