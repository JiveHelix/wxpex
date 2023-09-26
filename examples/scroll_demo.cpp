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
        const wxColor &color)
        :
        wxpex::Collapsible(parent, name, wxBORDER_SIMPLE)
    {
        auto coloredSquare = new wxPanel(this->GetPanel());
        coloredSquare->SetBackgroundColour(color);
        coloredSquare->SetMinSize(wxSize(400, 400));

        auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        sizer->Add(coloredSquare, 0, wxALL, 3);

        this->ConfigureTopSizer(std::move(sizer));
    }
};


class Colors: public wxpex::Collapsible
{
public:
    Colors(wxWindow *parent, const std::string &name)
        :
        wxpex::Collapsible(parent, name, wxBORDER_SIMPLE)
    {
        auto blue = new Widget(this->GetPanel(), "Blue", *wxBLUE);
        auto cyan = new Widget(this->GetPanel(), "Cyan", *wxCYAN);
        auto green = new Widget(this->GetPanel(), "Green", *wxGREEN);
        auto yellow = new Widget(this->GetPanel(), "Yellow", *wxYELLOW);
        auto red = new Widget(this->GetPanel(), "Red", *wxRED);

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
    ColorSets(wxWindow *parent)
        :
        wxpex::Scrolled(parent)
    {
        auto set1 = new Colors(this, "Set 1");
        auto set2 = new Colors(this, "Set 2");
        auto set3 = new Colors(this, "Set 3");

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
    LeftControls(wxWindow *parent)
        :
        wxPanel(parent)
    {
        auto topPanel = new wxPanel(this);
        topPanel->SetMinSize(wxSize(400, 100));

        auto colorSets = new ColorSets(this);

        auto bottomPanel = new wxPanel(this);
        bottomPanel->SetMinSize(wxSize(400, 400));

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
    ExampleFrame()
        :
        wxFrame(nullptr, wxID_ANY, "Settings Demo")
    {
        auto splitter = new wxpex::Splitter(this);

        auto topSizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
        topSizer->Add(splitter, 1, wxEXPAND);
        auto localCopy = topSizer.get();
        this->SetSizer(topSizer.release());

        auto controls = new LeftControls(splitter);

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
            new ExampleFrame();

        exampleFrame->Show();

        return true;
    }
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(ExampleApp)
