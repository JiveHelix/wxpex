/**
  * @file file_field_demo.cpp
  *
  * @brief Demonstrates the use of wxpex::FileField.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 23 Feb 2023
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/


#include <iostream>
#include <string>
#include <fstream>
#include <fields/fields.h>
#include <pex/group.h>
#include <wxpex/wxshim.h>
#include <wxpex/file_field.h>
#include <wxpex/button.h>
#include <wxpex/labeled_widget.h>


template<typename T>
struct ApplicationFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::source, "source"),
        fields::Field(&T::target, "target"),
        fields::Field(&T::message, "message"),
        fields::Field(&T::copy, "copy"));

    static constexpr auto fieldsTypeName = "FileFieldDemo";
};


template<template<typename> typename T>
struct ApplicationTemplate
{
    T<std::string> source;
    T<std::string> target;
    T<std::string> message;
    T<pex::MakeSignal> copy;
};


using ApplicationGroup = pex::Group<ApplicationFields, ApplicationTemplate>;

using Model = typename ApplicationGroup::Model;

using Control = typename ApplicationGroup::Control<void>;


class ExampleApp: public wxApp
{
public:
    static constexpr auto observerName = "ExampleApp";

    ExampleApp()
        :
        model_{},
        message_(this, this->model_.message),
        copy_(this, this->model_.copy)
    {
        this->message_.Connect(&ExampleApp::OnMessage_);
        this->copy_.Connect(&ExampleApp::OnCopy_);
    }

    bool OnInit() override;

private:
    void OnMessage_(const std::string &message)
    {
        wxMessageDialog(nullptr, message).ShowModal();
    }

    void OnCopy_()
    {
        auto source = this->model_.source.Get();
        auto target = this->model_.target.Get();

        if (source.empty())
        {
            this->message_.Set("Please specify a source.");
            return;
        }

        if (target.empty())
        {
            this->message_.Set("Please specify a target.");
            return;
        }

        std::ifstream sourceStream(source, std::ios::binary);

        if (!sourceStream)
        {
            this->message_.Set("FAIL: Unable to open for reading: " + source);
            return;
        }

        std::ofstream targetStream(target, std::ios::binary);

        if (!targetStream)
        {
            this->message_.Set("FAIL: Unable to open for writing: " + target);
            return;
        }

        targetStream << sourceStream.rdbuf();

        targetStream.flush();

        if (!targetStream)
        {
            this->message_.Set("FAIL: Copy failed to " + target);
            return;
        }

        this->message_.Set("SUCCESS: Copied " + source + " to " + target);
    }

private:
    Model model_;
    pex::Terminus<ExampleApp, decltype(Model::message)> message_;
    pex::Terminus<ExampleApp, decltype(Model::copy)> copy_;
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(Control control);
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(ExampleApp)


bool ExampleApp::OnInit()
{
    ExampleFrame *exampleFrame =
        new ExampleFrame(Control(this->model_));

    exampleFrame->Show();

    return true;
}


ExampleFrame::ExampleFrame(Control control)
    :
    wxFrame(nullptr, wxID_ANY, "wxpex::Field Demo")
{
    using namespace wxpex;

    FileDialogOptions options{};
    options.style = wxFD_OPEN;

    auto source = LabeledWidget(
        this,
        "Source",
        new FileField(this, control.source, options));

    options.style = wxFD_SAVE;

    auto target = LabeledWidget(
        this,
        "Target",
        new FileField(this, control.target, options));

    auto copy = new Button(this, "Copy", control.copy);

    auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    auto sizer = LayoutLabeled(
        LayoutOptions{},
        source,
        target);

    topSizer->Add(sizer.release(), 1, wxALL | wxEXPAND, 10);
    topSizer->Add(copy, 0, wxBOTTOM, 10);

    this->SetSizerAndFit(topSizer.release());
}
