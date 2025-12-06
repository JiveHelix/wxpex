
#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include <pex/interface.h>
#include <fields/core.h>
#include <fields/json_color.h>
#include <wxpex/console_viewer.h>
#include <wxpex/window.h>
#include <wxpex/button.h>
#include <wxpex/file_field.h>
#include <wxpex/app.h>
#include <wxpex/wxshim_app.h>


template<typename T>
struct UserFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::fileName, "fileName"),
        fields::Field(&T::console, "console"),
        fields::Field(&T::openJson, "openJson"),
        fields::Field(&T::quit, "quit"));
};


template<template<typename> typename T>
struct UserTemplate
{
    T<std::string> fileName;
    T<wxpex::ConsoleViewerGroup> console;
    T<pex::MakeSignal> openJson;
    T<pex::MakeSignal> quit;

    static constexpr auto fields = UserFields<UserTemplate>::fields;
};


using UserGroup = pex::Group<UserFields, UserTemplate>;
using UserControls = typename UserGroup::DefaultControl;
using UserModel = typename UserGroup::Model;


class JsonViewer: public wxFrame
{
public:
    JsonViewer(UserControls userControls)
        :
        wxFrame(nullptr, wxID_ANY, "Json Viewer")
    {
        auto openButton =
            new wxpex::Button(this, "Open Json", userControls.openJson);

        auto consoleViewer =
            new wxpex::ConsoleViewer(this, userControls.console);

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

        sizer->Add(openButton, 0, wxBOTTOM, 5);
        sizer->Add(consoleViewer, 1, wxEXPAND);

        this->SetSizerAndFit(sizer.release());
    }
};


#if 0
template<typename CharT, typename Traits = std::char_traits<CharT>>
class BasicBufferReplace
{
public:
    using StreamBuffer = std::basic_streambuf<CharT, Traits>;
    using OutputStream = std::basic_ostream<CharT, Traits>;

    BasicBufferReplace(
        OutputStream &outputStream,
        StreamBuffer &replacementBuffer)
        :
        outputStream_(outputStream),
        originalBuffer_(outputStream.rdbuf())
    {
        this->outputStream_.rdbuf(&replacementBuffer);
    }

    ~BasicBufferReplace()
    {
        this->outputStream_.rdbuf(this->originalBuffer_);
    }

private:
    OutputStream &outputStream_;
    StreamBuffer *originalBuffer_;
};
#endif


class ConsoleAppBrain
{
public:
    // using BufferReplace = BasicBufferReplace<char>;

    ConsoleAppBrain()
        :
        userModel_{},
        userControls_(this->userModel_),
        applicationFrame_(),

        openEndpoint_(
            this,
            this->userControls_.openJson,
            &ConsoleAppBrain::OnOpenJson_)
    {

    }

    UserControls GetUserControls()
    {
        return this->userControls_;
    }

    void CreateFrame()
    {
        this->applicationFrame_ =
            wxpex::UnclosedFrame(new JsonViewer(this->userControls_));

        this->applicationFrame_.Get()->Show();
    }

    void Shutdown()
    {
        this->applicationFrame_.Close();
    }

    void OnOpenJson_()
    {
        auto result = wxpex::ChoosePath(
            this->userModel_.fileName.Get(),
            wxpex::FileDialogOptions{}
                .Message("Choose a json file")
                .Wildcard(".json"));

        if (!result)
        {
            return;
        }

        std::ifstream input(*result);

        if (!input)
        {
            this->userModel_.console.message.Set(
                fmt::format("Unable to open file: {}", *result));

            return;
        }

        std::stringstream buffer;
        buffer << input.rdbuf();

        this->userModel_.fileName.Set(*result);
        this->userModel_.console.clear.Trigger();

        this->userModel_.console.message.Set(
            fields::GetColorizedJson(
                nlohmann::ordered_json::parse(buffer.str())));
    }

private:
    UserModel userModel_;
    UserControls userControls_;
    wxpex::UnclosedFrame applicationFrame_;
    /*
    std::unique_ptr<TeeBuffer> coutTee_;
    std::unique_ptr<TeeBuffer> cerrTee_;
    std::unique_ptr<BufferReplace> coutReplace_;
    std::unique_ptr<BufferReplace> cerrReplace_;
    */
    using OpenEndpoint =
        pex::Endpoint<ConsoleAppBrain, decltype(UserControls::openJson)>;

    OpenEndpoint openEndpoint_;
};


wxshimAPP(wxpex::App<ConsoleAppBrain>);
