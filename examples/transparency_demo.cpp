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
    void OnHsvaColor_(const wxpex::Hsva &hsv)
    {
        if (this->muted_)
        {
            return;
        }

        auto mute = jive::ScopeFlag(this->muted_);
        this->rgbaColor_.Set(tau::HsvToRgb<uint8_t>(hsv));
    }

    void OnRgbaColor_(const wxpex::Rgba &rgb)
    {
        if (this->muted_)
        {
            return;
        }

        auto mute = jive::ScopeFlag(this->muted_);
        this->hsvaColor_.Set(tau::RgbToHsv<double>(rgb));
    }

    wxpex::HsvaModel hsvaColor_ = wxpex::HsvaModel{{{0, 1, 0.5, 1.0}}};

    wxpex::RgbaModel rgbaColor_ =
        wxpex::RgbaModel{tau::HsvToRgb<uint8_t>(hsvaColor_.Get())};

    wxpex::HsvaControl hsvaControl_;
    wxpex::RgbaControl rgbaControl_;

    using HsvaConnect = pex::MakeConnector<ExampleApp, wxpex::HsvaControl>;

    std::unique_ptr<HsvaConnect> hsvaConnect_;

    using RgbaConnect = pex::MakeConnector<ExampleApp, wxpex::RgbaControl>;

    std::unique_ptr<RgbaConnect> rgbaConnect_;


    bool muted_ = false;
};


class ExampleFrame: public wxFrame
{
public:
    ExampleFrame(wxpex::HsvaControl hsv, wxpex::RgbaControl rgb);
};


// Creates the main function for us, and initializes the app's run loop.
wxshimIMPLEMENT_APP_CONSOLE(ExampleApp)


bool ExampleApp::OnInit()
{
    this->hsvaControl_ = wxpex::HsvaControl(this->hsvaColor_);

    this->hsvaConnect_ = std::make_unique<HsvaConnect>(
        this,
        this->hsvaControl_,
        &ExampleApp::OnHsvaColor_);

    this->rgbaControl_ = wxpex::RgbaControl(this->rgbaColor_);

    this->rgbaConnect_ = std::make_unique<RgbaConnect>(
        this,
        this->rgbaControl_,
        &ExampleApp::OnRgbaColor_);

    ExampleFrame *exampleFrame =
        new ExampleFrame(this->hsvaControl_, this->rgbaControl_);

    exampleFrame->Show();

    return true;
}


ExampleFrame::ExampleFrame(wxpex::HsvaControl hsv, wxpex::RgbaControl rgb)
    :
    wxFrame(nullptr, wxID_ANY, "Color Demo")
{
    auto hsvPicker = new wxpex::HsvaPicker(this, "HSV", hsv);
    auto rgbPicker = new wxpex::RgbaPicker(this, "RGB", rgb);
    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);

    sizer->Add(hsvPicker, 1, wxEXPAND | wxALL, 10);
    sizer->Add(rgbPicker, 1, wxEXPAND | wxALL, 10);

    this->SetSizerAndFit(sizer.release());
}
