#include <tau/color.h>
#include <pex/value.h>

#include "wxpex/wxshim.h"
#include "wxpex/color.h"



class ExampleApp: public wxApp
{
public:
    bool OnInit() override;

private:
    void OnHsvColor_(const wxpex::Hsv &hsv)
    {
        if (this->muted_)
        {
            return;
        }

        this->muted_ = true;
        this->rgbColor_.Set(tau::HsvToRgb<uint8_t>(hsv));
        this->muted_ = false;
    }

    void OnRgbColor_(const wxpex::Rgb &rgb)
    {
        if (this->muted_)
        {
            return;
        }

        this->muted_ = true;
        this->hsvColor_.Set(tau::RgbToHsv<float>(rgb));
        this->muted_ = false;
    }

    wxpex::HsvModel hsvColor_ = wxpex::HsvModel{{{0, 1, 1}}};

    wxpex::RgbModel rgbColor_ =
        wxpex::RgbModel{tau::HsvToRgb<uint8_t>(hsvColor_.Get())};

    wxpex::HsvTerminus<ExampleApp> hsvTerminus_;
    wxpex::RgbTerminus<ExampleApp> rgbTerminus_;
    bool muted_ = false;
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(wxpex::HsvControl hsv, wxpex::RgbControl rgb);
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(ExampleApp)


bool ExampleApp::OnInit()
{
    this->hsvTerminus_.Assign(
        this,
        wxpex::HsvTerminus<ExampleApp>(this, this->hsvColor_));

    this->hsvTerminus_.Connect(&ExampleApp::OnHsvColor_);

    this->rgbTerminus_.Assign(
        this,
        wxpex::RgbTerminus<ExampleApp>(this, this->rgbColor_));

    this->rgbTerminus_.Connect(&ExampleApp::OnRgbColor_);

    ExampleFrame *exampleFrame =
        new ExampleFrame(
            wxpex::HsvControl(this->hsvColor_),
            wxpex::RgbControl(this->rgbColor_));

    exampleFrame->Show();

    return true;
}


ExampleFrame::ExampleFrame(wxpex::HsvControl hsv, wxpex::RgbControl rgb)
    :
    wxFrame(nullptr, wxID_ANY, "Color Demo")
{
    auto hsvPicker = new wxpex::HsvPicker(this, hsv);
    auto rgbPicker = new wxpex::RgbPicker(this, rgb);
    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    sizer->Add(hsvPicker, 1, wxEXPAND | wxALL, 10);
    sizer->Add(rgbPicker, 1, wxEXPAND | wxALL, 10);

    this->SetSizerAndFit(sizer.release());
}
