#include <tau/color.h>
#include <pex/value.h>

#include "wxpex/wxshim.h"
#include "wxpex/color.h"



class ExampleApp: public wxApp
{
public:
    bool OnInit() override;

private:
    void OnColor_(const tau::Hsv<float> &color)
    {
        std::cout << "Color:\n" << color << std::endl;
    }

    wxpex::HsvModel color_ = wxpex::HsvModel{{{0, 1, 1}}};

    using ColorTerminus = pex::Terminus<ExampleApp, wxpex::HsvModel>;
    ColorTerminus colorTerminus_;
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(wxpex::HsvControl control);
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(ExampleApp)


bool ExampleApp::OnInit()
{
    this->colorTerminus_.Assign(this, ColorTerminus(this, this->color_));

    PEX_LOG("color_.Connect");
    this->colorTerminus_.Connect(&ExampleApp::OnColor_);

    PEX_LOG("ExampleFrame");
    ExampleFrame *exampleFrame =
        new ExampleFrame(wxpex::HsvControl(this->color_));

    exampleFrame->Show();

    return true;
}


ExampleFrame::ExampleFrame(wxpex::HsvControl control)
    :
    wxFrame(nullptr, wxID_ANY, "Color Demo")
{
    PEX_LOG("\n\n ********* new HsvPicker ************* \n\n");
    auto colorPicker = new wxpex::HsvPicker(this, control);
    auto sizer = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    
    sizer->Add(colorPicker, 1, wxEXPAND | wxALL, 10);

    this->SetSizerAndFit(sizer.release());
}
