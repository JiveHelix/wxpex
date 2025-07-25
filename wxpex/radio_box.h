/**
  * @file radio_box.h
  *
  * @brief A wxRadioBox connected to a pex::Value control node.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 11 Aug 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#pragma once

#include <vector>
#include <algorithm>

#include <pex/value.h>
#include <pex/converter.h>
#include <pex/find_index.h>

#include "wxpex/wxshim.h"
#include "wxpex/array_string.h"
#include "wxpex/wx_select.h"


namespace wxpex
{


template
<   typename Control,
    typename Convert = pex::Converter<typename Control::Value::Type>
>
class RadioBox: public wxRadioBox
{
public:
    static constexpr auto observerName = "wxpex::RadioBox";

    static_assert(
        !Control::choicesMayChange,
        "RadioBox choices cannot change after creation");

    using Type = typename Control::Type;
    using Selection = typename Control::Selection;
    using WxAdapter = WxSelect<typename Control::Type, Convert>;

    RadioBox(
        wxWindow *parent,
        Control select,
        const std::string &label = "",
        long style = wxRA_SPECIFY_ROWS)
        :
        wxRadioBox(
            parent,
            wxID_ANY,
            label,
            wxDefaultPosition,
            wxDefaultSize,
            WxAdapter::GetChoicesAsStrings(select.choices.Get()),
            0,
            style),

        selection_(
            USE_REGISTER_PEX_NAME(this, "wxpex::RadioBox"),
            select.selection)
    {
        assert(
            this->selection_.Get() <= std::numeric_limits<int>::max());

        this->SetSelection(static_cast<int>(this->selection_.Get()));

        this->selection_.Connect(&RadioBox::OnSelection_);

        this->Bind(wxEVT_RADIOBOX, &RadioBox::OnRadioBox_, this);
    }

    void OnSelection_(size_t index)
    {
        assert(index <= std::numeric_limits<int>::max());
        this->SetSelection(static_cast<int>(index));
    }

    void OnRadioBox_(wxCommandEvent &event)
    {
        auto index = event.GetSelection();

        if (index < 0)
        {
            return;
        }

        this->selection_.Set(static_cast<size_t>(index));
    }

private:
    pex::Terminus<RadioBox, Selection> selection_;
};


} // namespace wxpex
