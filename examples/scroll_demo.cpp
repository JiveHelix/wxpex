#include <iostream>
#include <string>

#include <wxpex/scrolled.h>
#include <wxpex/splitter.h>
#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wx/colour.h>


class Widget: public wxpex::Collapsible
{
public:
    Widget(
        wxWindow *parent,
        const std::string &name,
        const wxColor &color,
        wxpex::Collapsible::StateControl stateControl)
        :
        wxpex::Collapsible(parent, name, stateControl, wxBORDER_SIMPLE)
    {
        auto coloredSquare = new wxPanel(this->GetPanel());
        coloredSquare->SetBackgroundColour(color);
        coloredSquare->SetMinSize(wxSize(400, 400));

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        sizer->Add(coloredSquare, 0, wxALL, 3);

        this->ConfigureTopSizer(std::move(sizer));
    }
};


template<typename T>
struct ColorsStateFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::blue, "blue"),
        fields::Field(&T::cyan, "cyan"),
        fields::Field(&T::green, "green"),
        fields::Field(&T::yellow, "yellow"),
        fields::Field(&T::red, "red"));
};


template<template<typename> typename T>
struct StateTemplate
{
    T<bool> blue;
    T<bool> cyan;
    T<bool> green;
    T<bool> yellow;
    T<bool> red;

    static constexpr auto fields = ColorsStateFields<StateTemplate>::fields;
};


using ColorsStateGroup = pex::Group<ColorsStateFields, StateTemplate>;
using ColorsStateModel = typename ColorsStateGroup::Model;
using ColorsStateControl = typename ColorsStateGroup::Control;


class Colors: public wxpex::Collapsible
{
public:
    Colors(
        wxWindow *parent,
        const std::string &name,
        ColorsStateControl state)
        :
        wxpex::Collapsible(parent, name, wxBORDER_SIMPLE)
    {
        auto blue =
            new Widget(this->GetPanel(), "Blue", *wxBLUE, state.blue);

        auto cyan =
            new Widget(this->GetPanel(), "Cyan", *wxCYAN, state.cyan);

        auto green =
            new Widget(this->GetPanel(), "Green", *wxGREEN, state.green);

        auto yellow =
            new Widget(this->GetPanel(), "Yellow", *wxYELLOW, state.yellow);

        auto red =
            new Widget(this->GetPanel(), "Red", *wxRED, state.red);

        auto sizer = wxpex::LayoutItems(
            wxpex::verticalItems,
            blue,
            cyan,
            green,
            yellow,
            red);

        this->ConfigureTopSizer(std::move(sizer));
    }
};


class ColorSets: public wxpex::Scrolled
{
public:
    ColorSets(wxWindow *parent, ColorsStateControl state)
        :
        wxpex::Scrolled(parent)
    {
        auto set1 = new Colors(this, "Set 1", state);
        auto set2 = new Colors(this, "Set 2", state);
        auto set3 = new Colors(this, "Set 3", state);

        auto sizer = wxpex::LayoutItems(
            wxpex::verticalItems,
            set1,
            set2,
            set3);

        this->ConfigureTopSizer(
            wxpex::verticalScrolled,
            std::move(sizer));
    }
};



class LeftControls: public wxPanel
{
public:
    LeftControls(wxWindow *parent, ColorsStateControl state)
        :
        wxPanel(parent)
    {
        auto topPanel = new wxPanel(this);
        topPanel->SetMinSize(wxSize(400, 100));

        auto colorSets = new ColorSets(this, state);

        auto bottomPanel = new wxPanel(this);
        bottomPanel->SetMinSize(wxSize(400, 100));

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

        sizer->Add(topPanel, 0, wxBOTTOM | wxEXPAND, 5);
        sizer->Add(colorSets, 1, wxBOTTOM | wxEXPAND, 5);
        sizer->Add(bottomPanel, 0);

        auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        topSizer->Add(sizer.get(), 1, wxEXPAND | wxALL, 5);
        sizer.release();
        this->SetSizerAndFit(topSizer.release());
    }
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(ColorsStateControl state)
        :
        wxFrame(nullptr, wxID_ANY, "Scroll Demo")
    {
        auto splitter = new wxpex::Splitter(this);

        auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        topSizer->Add(splitter, 1, wxEXPAND);
        auto localCopy = topSizer.get();
        this->SetSizer(topSizer.release());

        auto controls = new LeftControls(splitter, state);

        auto rightPanel = new wxPanel(splitter);
        rightPanel->SetMinSize(wxSize(600, 600));

        splitter->SplitVerticallyLeft(
            controls,
            rightPanel);

        localCopy->SetSizeHints(this);
    }
};


class ExampleApp: public wxApp
{
public:
    bool OnInit() override
    {
        ExampleFrame *exampleFrame =
            new ExampleFrame(ColorsStateControl(this->colorsStateModel_));

        exampleFrame->Show();

        return true;
    }

    ColorsStateModel colorsStateModel_;
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(ExampleApp)
