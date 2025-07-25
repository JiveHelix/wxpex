#pragma once


#include <pex/terminus.h>
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

    using Quit = pex::Terminus<App, pex::model::Signal>;

    bool OnInit() override
    {
        REGISTER_PEX_NAME(this, "App");
        this->brain_ = std::make_unique<Brain>();
        auto userControls = this->brain_->GetUserControls();
        this->quit_.Assign(this, Quit(this, userControls.quit));
        this->quit_.Connect(&App<Brain>::OnQuit_);
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
