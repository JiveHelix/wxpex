#pragma once

#include <map>
#include <functional>
#include <memory>

#include <jive/strings.h>
#include <jive/formatter.h>
#include <jive/for_each.h>

#include <pex/signal.h>
#include <pex/detail/log.h>

#include "wxpex/wxshim.h"
#include "wxpex/window.h"


namespace wxpex
{


class Key
{
public:
    template<typename T>
    Key(T keyCode)
        :
        code_(static_cast<int>(keyCode))
    {
        static_assert(
            std::is_same_v<T, char>
            || std::is_same_v<T, wxKeyCode>,
            "A key code must be a char or a wxKeyCode enum value.");
    }

    std::string GetString() const;

    int GetInt() const;

private:
    int code_;
    static const std::map<int, std::string> keyStringByKeyCode_;
};


/**
Returns a string describing all modifiers in the bitfield.

modifierBitfield may be bitwise combination of modifiers.

For Mac builds, wxACCEL_CTRL and wxACCEL_CMD map to the command key,
wxACCEL_RAW_CTRL is the modifier for the 'control' key.

For other builds, wxACCEL_RAW_CTRL maps to the 'ctrl' key.
**/
std::string GetModifierString(int modifierBitfield);


class Shortcut
{
public:
    // We are not observing this signal, so the Observer can be void.
    using SignalType = pex::control::Signal<void>;

    template<typename KeyCode>
    Shortcut(
        SignalType signal,
        int modifier,
        KeyCode keyCode,
        std::string_view description,
        std::string_view longDescription,
        int id = wxID_NONE)
        :
        signal_(signal),
        id_((id == wxID_NONE) ? wxWindow::NewControlId(): id),
        modifier_(modifier),
        key_(keyCode),
        description_(description),
        longDescription_(longDescription),
        menuItem_(NULL)
    {

    }

    void AddToMenu(wxMenu *menu);

    wxAcceleratorEntry GetAcceleratorEntry() const;

    int GetModifier() const;

    int GetKeyAsInt() const;
    
    int GetId() const;

    void OnEventMenu();

private: 
    wxString GetMenuItemLabel_() const;

    SignalType signal_;
    int id_;
    int modifier_;
    Key key_;
    std::string description_;
    std::string longDescription_;
    wxMenuItem *menuItem_;
};


class ShortcutFunctor
{
public:
    ShortcutFunctor(const Shortcut &shortcut): shortcut_(shortcut) {}

    void operator()(wxCommandEvent &)
    {
        this->shortcut_.OnEventMenu();
    }

private:
    Shortcut shortcut_;
};


using Shortcuts = std::vector<Shortcut>;

struct ShortcutGroup
{
    std::string name;
    Shortcuts shortcuts;
};

using ShortcutGroups = std::vector<ShortcutGroup>;


class ShortcutsBase
{
public:
    ShortcutsBase(
        Window &&window,
        const ShortcutGroups &groups);

    ~ShortcutsBase();

    ShortcutsBase(const ShortcutsBase &) = delete;
    ShortcutsBase & operator=(const ShortcutsBase &) = delete;

    ShortcutsBase(ShortcutsBase &&);
    ShortcutsBase & operator=(ShortcutsBase &&);

private:
    void BindAll_();

    void UnbindAll_();

    void UnbindShortcuts_(wxWindow *window, Shortcuts &shortcuts);

    void BindShortcuts_(wxWindow *window, Shortcuts &shortcuts);

    void OnWindowClose_(wxCloseEvent &event);

private:
    Window window_;
    bool hasBindings_;

protected:
    ShortcutGroups groups_;
};


class MenuShortcuts: public ShortcutsBase
{
public:
    MenuShortcuts(
        Window &&window,
        const ShortcutGroups &groups);

    wxMenuBar * GetMenuBar();

    static void AddToMenu(wxMenu *menu, Shortcuts &shortcuts);

private:
    std::unique_ptr<wxMenuBar> menuBar_;
};


class AcceleratorShortcuts: public ShortcutsBase
{
public:
    AcceleratorShortcuts(
        Window &&window,
        const ShortcutGroups &groups);

    const wxAcceleratorTable & GetAcceleratorTable() const;

    static std::vector<wxAcceleratorEntry>
    CreateAcceleratorEntries(const Shortcuts &shortcuts);

private:
    wxAcceleratorTable acceleratorTable_;
};


class ShortcutWindow: public Window
{
public:
    ShortcutWindow() = default;

    ShortcutWindow(wxWindow *, const ShortcutGroups &);

private:
    std::unique_ptr<wxpex::AcceleratorShortcuts> acceleratorShortcuts_;
};


} // namespace wxpex
