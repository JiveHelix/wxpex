/**
  * @file shortcut_demo.cpp
  *
  * @brief Demonstrates the usage of pex::Shortcut and wx menus.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 23 Aug 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#include <iostream>
#include <string>

#include <fields/fields.h>
#include <fields/assign.h>

#include <pex/interface.h>
#include <pex/group.h>

#include "wxpex/wxshim.h"
#include "wxpex/window.h"
#include "wxpex/shortcut.h"
#include "wxpex/view.h"


template<typename T>
struct ApplicationFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::sayWhatsUp, "sayWhatsUp"),
        fields::Field(&T::sayHello, "sayHello"),
        fields::Field(&T::sayFortyTwo, "sayFortyTwo"),
        fields::Field(&T::frobnicate, "frobnicate"),
        fields::Field(&T::quit, "quit"),
        fields::Field(&T::message, "message"));
};

template<template<typename> typename T>
struct ApplicationTemplate
{
    T<pex::MakeSignal> sayWhatsUp;
    T<pex::MakeSignal> sayHello;
    T<pex::MakeSignal> sayFortyTwo;
    T<pex::MakeSignal> frobnicate;
    T<pex::MakeSignal> quit;
    T<std::string> message;
};


using ApplicationGroup = pex::Group<ApplicationFields, ApplicationTemplate>;


struct ApplicationModel: public ApplicationGroup::Model
{
    static constexpr auto observerName = "ApplicationModel";

    ApplicationModel()
    {
        this->message.Set("<message>");

        PEX_LOG("Connect");
        this->sayHello.Connect(
            this,
            &ApplicationModel::OnSayHello_);

        PEX_LOG("Connect");
        this->sayWhatsUp.Connect(
            this,
            &ApplicationModel::OnSayWhatsUp_);

        PEX_LOG("Connect");
        this->sayFortyTwo.Connect(
            this,
            &ApplicationModel::OnSayFortyTwo_);

        PEX_LOG("Connect");
        this->frobnicate.Connect(
            this,
            &ApplicationModel::OnFrobnicate_);
    }

    ~ApplicationModel()
    {
        PEX_LOG("Disconnect");
        this->sayHello.Disconnect(this);

        PEX_LOG("Disconnect");
        this->sayWhatsUp.Disconnect(this);

        PEX_LOG("Disconnect");
        this->sayFortyTwo.Disconnect(this);

        PEX_LOG("Disconnect");
        this->frobnicate.Disconnect(this);
    }

    static void OnSayHello_(void * context)
    {
        auto self = static_cast<ApplicationModel *>(context);
        self->message.Set("Hello");
    }

    static void OnSayWhatsUp_(void * context)
    {
        auto self = static_cast<ApplicationModel *>(context);
        self->message.Set("What's up?");
    }

    static void OnSayFortyTwo_(void * context)
    {
        auto self = static_cast<ApplicationModel *>(context);
        self->message.Set("forty-two");
    }

    static void OnFrobnicate_(void * context)
    {
        auto self = static_cast<ApplicationModel *>(context);
        self->message.Set("Frobnicating...");
    }
};


using ApplicationControl = typename ApplicationGroup::Control;


class ExampleApp: public wxApp
{
    using Quit = pex::Terminus<ExampleApp, pex::model::Signal>;

public:
    static constexpr auto observerName = "ExampleApp";

    ExampleApp()
        :
        applicationModel_{},
        quit_(this, this->applicationModel_.quit)
    {
        this->quit_.Connect(&ExampleApp::OnQuit_);
    }

    bool OnInit() override;

private:
    void OnQuit_()
    {
        this->anotherFrame_.Close();
        this->exampleFrame_.Close();
    }

private:
    ApplicationModel applicationModel_;
    Quit quit_;

    wxpex::Window exampleFrame_;
    wxpex::Window anotherFrame_;
};


class AnotherFrame: public wxFrame
{
public:
    AnotherFrame(const wxpex::ShortcutGroups & shortcuts)
        :
        wxFrame(nullptr, wxID_ANY, "A frame with no menus"),
        acceleratorShortcuts_(wxpex::UnclosedWindow(this), shortcuts)
    {
        this->SetAcceleratorTable(
            this->acceleratorShortcuts_.GetAcceleratorTable());

        new wxTextCtrl(
            this,
            wxID_ANY,
            "This frame has no menus, but shortcuts should still work when "
            "it has focus.",
            wxDefaultPosition,
            wxDefaultSize,
            wxTE_MULTILINE | wxTE_READONLY);
    }

private:
    wxpex::AcceleratorShortcuts acceleratorShortcuts_;
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(
        ApplicationControl applicationControl,
        wxpex::ShortcutGroups & shortcuts
        )
        :
        wxFrame(nullptr, wxID_ANY, "wxpex::Shortcut Demo"),
        menuShortcuts_(wxpex::UnclosedWindow(this), shortcuts)
    {
        this->SetMenuBar(this->menuShortcuts_.GetMenuBar());

        auto message = new wxStaticText(
            this,
            wxID_ANY,
            "Use the shortcut keys and the menu items.");

        auto view =
            new wxpex::View(this, applicationControl.message);

        auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        topSizer->Add(message, 0, wxALL, 10);
        topSizer->Add(view, 0, wxALL, 10);
        this->SetSizer(topSizer.release());
    }

private:
    wxpex::MenuShortcuts menuShortcuts_;
};


wxshimIMPLEMENT_APP_CONSOLE(ExampleApp)


bool ExampleApp::OnInit()
{
    auto applicationControl = ApplicationControl(this->applicationModel_);

    using namespace std::literals;

    auto shortcutsByMenu = wxpex::ShortcutGroups(
        {
            {
                "File", wxpex::Shortcuts(
                    {
                        wxpex::Shortcut(
                            applicationControl.sayFortyTwo,
                            wxACCEL_CMD,
                            'Z',
                            "42",
                            "Say '42'"),

                        wxpex::Shortcut(
                            applicationControl.frobnicate,
                            wxACCEL_CMD,
                            'F',
                            "Frobnicate",
                            "Do some frobnicating"),

                        wxpex::Shortcut(
                            applicationControl.quit,
                            wxACCEL_CMD,
                            'Q',
                            "Quit",
                            "Quit the application",
                            wxID_EXIT)
                    })
            },
            {
                "Other", wxpex::Shortcuts(
                    {
                        wxpex::Shortcut(
                            applicationControl.sayWhatsUp,
                            wxACCEL_CMD,
                            'W',
                            "What's up?",
                            "Say 'What's up?'"),

                        wxpex::Shortcut(
                            applicationControl.sayHello,
                            wxACCEL_ALT | wxACCEL_SHIFT,
                            'H',
                            "Hello",
                            "Say 'Hello'")
                    })
            }
        });

    auto exampleFrame = new ExampleFrame(applicationControl, shortcutsByMenu);
    this->exampleFrame_ = wxpex::Window(exampleFrame);

    auto anotherFrame = new AnotherFrame(shortcutsByMenu);
    this->anotherFrame_ = wxpex::Window(anotherFrame);

    exampleFrame->Show();
    anotherFrame->Show();

    exampleFrame->Raise();

    auto position = exampleFrame->GetScreenRect().GetTopRight();
    anotherFrame->SetPosition(position);

    return true;
}
