#pragma once


#include <pex/endpoint.h>
#include <pex/signal.h>
#include <wxpex/async.h>
#include <wxpex/wxshim.h>
#include <wxpex/window.h>


namespace wxpex
{


template<typename Brain>
class App : public wxApp
{
public:
    static constexpr auto observerName = "App";

    using Quit = pex::Endpoint<App, pex::control::DefaultSignal>;

    bool OnInit() override
    {
        PEX_NAME("App");
        this->brain_ = std::make_unique<Brain>();
        auto userControls = this->brain_->GetUserControls();
        this->quit_ = Quit(this, userControls.quit, &App<Brain>::OnQuit_);
        this->brain_->CreateFrame();

        this->doQuit_ = std::make_unique<wxpex::CallAfter>(
            [this]() -> void
            {
                this->brain_->Shutdown();
            });

        return true;
    }

private:
    void OnQuit_()
    {
        if (this->doQuit_)
        {
            (*this->doQuit_)();
        }
    }

    std::unique_ptr<Brain> brain_;
    Quit quit_;
    std::unique_ptr<wxpex::CallAfter> doQuit_;
};


} // end namespace wxpex
