#pragma once


#include <wxpex/wxshim.h>
#include <string>


namespace wxpex
{


/**
Returns a string describing all modifiers in the bitfield.

modifierBitfield may be bitwise combination of modifiers.

For Mac builds, wxACCEL_CTRL and wxACCEL_CMD map to the command key,
wxACCEL_RAW_CTRL is the modifier for the 'control' key.

For other builds, wxACCEL_RAW_CTRL maps to the 'ctrl' key.
**/
// std::string GetModifierString(int modifierBitfield);



class Modifier
{
public:
    Modifier();

    Modifier(wxKeyModifier value);

    Modifier(int value);

    void Add(int value);

    void Remove(int value);

    bool IsNone() const;

    bool IsAlt() const;

    bool IsControl() const;

    bool IsShift() const;

    bool IsMeta() const;

    bool IsWin() const;

    bool IsRawControl() const;

    bool IsCmd() const;

    std::string ToString() const;

private:
    wxKeyModifier value_;
};


std::ostream & operator<<(std::ostream &, const Modifier &);


} // end namespace wxpex
