#include "wxpex/modifier.h"
#include <map>
#include <jive/strings.h>


namespace wxpex
{


Modifier::Modifier()
    :
    value_{}
{

};

Modifier::Modifier(wxKeyModifier value)
    :
    value_(value)
{

}

Modifier::Modifier(int value)
    :
    value_(static_cast<wxKeyModifier>(value))
{

}

void Modifier::Add(int value)
{
    this->value_ = static_cast<wxKeyModifier>(this->value_ | value);
}

void Modifier::Remove(int value)
{
    this->value_ = static_cast<wxKeyModifier>(this->value_ & ~value);
}

bool Modifier::IsAlt() const
{
    return (this->value_ & wxMOD_ALT);
}

bool Modifier::IsControl() const
{
    return (this->value_ & wxMOD_CONTROL);
}

bool Modifier::IsShift() const
{
    return (this->value_ & wxMOD_SHIFT);
}

bool Modifier::IsMeta() const
{
    return (this->value_ & wxMOD_META);
}

bool Modifier::IsWin() const
{
    return (this->value_ & wxMOD_WIN);
}

bool Modifier::IsRawControl() const
{
    return (this->value_ & wxMOD_RAW_CONTROL);
}

bool Modifier::IsCmd() const
{
    return (this->value_ & wxMOD_CMD);
}

bool Modifier::IsNone() const
{
    return this->value_ == wxMOD_NONE;
}

std::string Modifier::ToString() const
{
    static const std::map<wxKeyModifier, std::string> modifierStrings
    {
        {wxMOD_NONE, ""},
        {wxMOD_ALT, "ALT"},
        {wxMOD_CONTROL, "CONTROL"},
        {wxMOD_SHIFT, "SHIFT"},
        {wxMOD_META, "SHIFT"},
        {wxMOD_RAW_CONTROL, "RAW_CONTROL"}
    };

    static const std::vector<wxKeyModifier> modifierOrder
    {
        wxMOD_CONTROL,
        wxMOD_SHIFT,
        wxMOD_ALT
#ifdef __APPLE__
        , wxMOD_RAW_CONTROL
#endif
        , wxMOD_META
    };

    std::vector<std::string> modifiers;

    for (auto modifier: modifierOrder)
    {
        if (this->value_ & modifier)
        {
            modifiers.push_back(modifierStrings.at(modifier));
        }
    }

    return jive::strings::Join(modifiers.begin(), modifiers.end(), '+');
}


std::ostream & operator<<(std::ostream &output, const Modifier &modifier)
{
    return output << modifier.ToString();
}


} // end namespace wxpex
