#include <catch2/catch.hpp>

#include <wxpex/async.h>
#include <wxpex/async_range.h>
#include <wx/app.h>
#include <wx/init.h>


class TestApp: public wxAppConsole
{
public:
    bool OnInit() override { return true; }
};

wxIMPLEMENT_APP_NO_MAIN(TestApp);

class WxTestFixture
{
    wxInitializer wx_;

public:
    WxTestFixture()
        :
        wx_{}
    {
        REQUIRE(this->wx_.IsOk());
        REQUIRE(wxAppConsole::GetInstance()->CallOnInit());
    }

    ~WxTestFixture()
    {
        wxAppConsole::GetInstance()->OnExit();
    }

    void ProcessPendingEvents()
    {
        wxAppConsole::GetInstance()->ProcessPendingEvents();
    }
};


TEST_CASE("Range filter is applied to both threads", "[async]")
{
    using Range = wxpex::DefaultAsyncRange<int>;
    using Model = pex::ModelTailor<Range>;
    using RangeControl = pex::ControlTailor<Range>;
    using ValueControl = typename RangeControl::Value;

    WxTestFixture testFixture;
    Model model;
    RangeControl control(model);
    ValueControl workerControl(model.value.GetWorkerControl());

    REQUIRE(model.value.Get() == 0);
    REQUIRE(control.value.Get() == 0);

    model.SetLimits(-10, 10);
    workerControl.Set(13);

    // The control cannot update until the wx run loop has processed the queued
    // event.
    REQUIRE(control.value.Get() == 0);

    testFixture.ProcessPendingEvents();

    REQUIRE(control.value.Get() == 10);

    // Increase limits to prove that the filter on the worker node is also
    // updated.
    model.SetLimits(-20, 20);
    workerControl.Set(13);

    // The control cannot update until the wx run loop has processed the queued
    // event.
    REQUIRE(control.value.Get() == 10);

    testFixture.ProcessPendingEvents();

    REQUIRE(control.value.Get() == 13);
}
