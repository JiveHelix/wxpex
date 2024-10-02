/**
  * @file pex_view.h
  *
  * @brief A read-only view of a pex.Value control node.
  *
  * @author Jive Helix (jivehelix@gmail.com)
  * @date 06 Aug 2020
  * @copyright Jive Helix
  * Licensed under the MIT license. See LICENSE file.
**/

#pragma once

#include <type_traits>
#include <string>

#include <pex/value.h>
#include <pex/argument.h>
#include <wxpex/converter.h>

#include "wxpex/wxshim.h"


namespace wxpex
{


template
<
    typename Control,
    typename Convert = pex::Converter<typename Control::Type>
>
class View: public wxStaticText
{
public:
    static constexpr auto observerName = "wxpex::View";

    using Base = wxStaticText;
    using Type = typename Control::Type;

    View(
        wxWindow *parent,
        Control value,
        long style = 0)
        :
        Base(
            parent,
            wxID_ANY,
            Convert::ToString(value.Get()),
            wxDefaultPosition,
            wxDefaultSize,
            style),
        value_(this, value)
    {
        PEX_LOG("Connect");
        this->value_.Connect(&View::OnValueChanged_);
    }

private:
    void OnValueChanged_(pex::Argument<Type> value)
    {
        this->SetLabel(Convert::ToString(value));
    }

    using Value_ = pex::Terminus<View, Control>;
    Value_ value_;
};


template
<
    typename Value,
    typename Convert = pex::Converter<typename Value::Type>
>
View<Value, Convert> * MakeView(wxWindow *parent, Value value, long style = 0)
{
    return new View<Value, Convert>(parent, value, style);
}


template<int precision, typename Control>
auto CreateView(wxWindow *parent, Control control)
{
    using Result = View<Control, PrecisionConverter<Control, precision>>;

    return new Result(parent, control);
}


} // namespace wxpex
