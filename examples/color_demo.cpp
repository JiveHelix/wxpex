#include <jive/scope_flag.h>
#include <tau/color.h>
#include <pex/value.h>
#include <pex/endpoint.h>

#include <wxpex/wxshim.h>
#include <wxpex/color_picker.h>



class ExampleApp: public wxApp
{
public:
    static constexpr auto observerName = "ExampleApp";

    bool OnInit() override;

private:
    void OnHsvColor_(const wxpex::Hsv &hsv)
    {
        if (this->muted_)
        {
            return;
        }

        auto mute = jive::ScopeFlag(this->muted_);
        this->rgbColor_.Set(tau::HsvToRgb<uint8_t>(hsv));
    }

    void OnRgbColor_(const wxpex::Rgb &rgb)
    {
        if (this->muted_)
        {
            return;
        }

        auto mute = jive::ScopeFlag(this->muted_);
        this->hsvColor_.Set(tau::RgbToHsv<double>(rgb));
    }

    wxpex::HsvModel hsvColor_ = wxpex::HsvModel{{{0, 1, 0.5}}};

    wxpex::RgbModel rgbColor_ =
        wxpex::RgbModel{tau::HsvToRgb<uint8_t>(hsvColor_.Get())};

    wxpex::HsvControl hsvControl_;
    wxpex::RgbControl rgbControl_;

    using HsvConnect = pex::MakeConnector<ExampleApp, wxpex::HsvControl>;

    std::unique_ptr<HsvConnect> hsvConnect_;

    using RgbConnect = pex::MakeConnector<ExampleApp, wxpex::RgbControl>;

    std::unique_ptr<RgbConnect> rgbConnect_;

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
    this->hsvControl_ = wxpex::HsvControl(this->hsvColor_);

    this->hsvConnect_ = std::make_unique<HsvConnect>(
        this,
        this->hsvControl_,
        &ExampleApp::OnHsvColor_);

    this->rgbControl_ = wxpex::RgbControl(this->rgbColor_);

    this->rgbConnect_ = std::make_unique<RgbConnect>(
        this,
        this->rgbControl_,
        &ExampleApp::OnRgbColor_);

    ExampleFrame *exampleFrame =
        new ExampleFrame(this->hsvControl_, this->rgbControl_);

    exampleFrame->Show();

    return true;
}


ExampleFrame::ExampleFrame(wxpex::HsvControl hsv, wxpex::RgbControl rgb)
    :
    wxFrame(nullptr, wxID_ANY, "Color Demo")
{
    auto hsvPicker = new wxpex::HsvPicker(this, "HSV", hsv);
    auto rgbPicker = new wxpex::RgbPicker(this, "RGB", rgb);
    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    sizer->Add(hsvPicker, 0, wxEXPAND | wxALL, 10);
    sizer->Add(rgbPicker, 0, wxEXPAND | wxBOTTOM | wxLEFT | wxRIGHT, 10);

    this->SetSizerAndFit(sizer.release());
}
