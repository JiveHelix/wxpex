#include "wxpex/widget_names.h"
#include <map>


namespace wxpex
{


static std::map<wxWindow *, std::string> nameByPointer_;



void RegisterWidgetName(wxWindow *window, const std::string &name)
{
    nameByPointer_[window] = name;
}


std::string GetWidgetName(wxWindow *window)
{
    if (nameByPointer_.count(window) == 1)
    {
        return nameByPointer_[window];
    }

    return "None";
}


std::vector<std::string> GetAncestry(wxWindow *window)
{
    std::vector<std::string> result;

    result.push_back(GetWidgetName(window));

    while ((window = window->GetParent()))
    {
        result.push_back(GetWidgetName(window));
    }

    return result;
}


std::ostream & PrintAncestry(
    std::ostream &output,
    const std::vector<std::string> &ancestry)
{
    auto it = std::rbegin(ancestry);

    size_t depth = 0;

    while (it != std::rend(ancestry))
    {
        output << std::string(depth * 2, ' ') << *it << '\n';
        ++it;
        ++depth;
    }

    return output;
}


} // end namespace wxpex
