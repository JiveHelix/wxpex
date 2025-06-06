#pragma once


#include <wx/sizer.h>
#include <type_traits>


namespace wxpex
{


namespace detail
{


template<typename T>
concept CanAddToSizer =
    std::convertible_to<T, wxWindow *>
    || std::convertible_to<T, wxSizer *>
    || std::convertible_to<T, wxSizerItem *>;


template<typename T>
concept HasRelease = requires(T t)
{
    {t.release()} -> CanAddToSizer;
};


template<typename T>
auto Release(T &&value)
{
    if constexpr (HasRelease<std::remove_cvref_t<T>>)
    {
        return value.release();
    }
    else
    {
        return value;
    }
};


template<typename Tuple, size_t... I>
void DoLayoutItems(
    wxBoxSizer *sizer,
    int proportion,
    int flags,
    int spacingFlag,
    int spacing,
    Tuple items,
    std::index_sequence<I...>)
{
    static constexpr size_t count = sizeof...(I);
    static_assert(count >= 1);

    (sizer->Add(
        Release(std::get<I>(items)),
        proportion,
        I < (count - 1) ? (flags | spacingFlag) : flags,
        I < (count - 1) ? spacing : 0), ...);
}


} // end namespace detail


struct ItemOptions
{
    int orient;
    int flags;
    int spacing;
    int proportion;

    ItemOptions & Orient(int orient_)
    {
        this->orient = orient_;
        return *this;
    }

    ItemOptions & Flags(int flags_)
    {
        this->flags = flags_;
        return *this;
    }

    ItemOptions & Spacing(int spacing_)
    {
        this->spacing = spacing_;
        return *this;
    }

    ItemOptions & Proportion(int proportion_)
    {
        this->proportion = proportion_;
        return *this;
    }
};


inline constexpr auto horizontalItems =
    ItemOptions{wxHORIZONTAL, wxEXPAND, 5, 0};

inline constexpr auto verticalItems =
    ItemOptions{wxVERTICAL, wxEXPAND, 5, 0};


inline ItemOptions HorizontalItems()
{
    return horizontalItems;
}

inline ItemOptions VerticalItems()
{
    return verticalItems;
}


template<typename ...Items>
std::unique_ptr<wxSizer> LayoutItems(
    const ItemOptions &options,
    Items &&...items)
{
    auto sizer = std::make_unique<wxBoxSizer>(options.orient);
    int spacingFlag;

    if (options.orient == wxHORIZONTAL)
    {
        spacingFlag = wxRIGHT;
    }
    else
    {
        spacingFlag = wxBOTTOM;
    }

    detail::DoLayoutItems(
        sizer.get(),
        options.proportion,
        options.flags,
        spacingFlag,
        options.spacing,
        std::forward_as_tuple(std::forward<Items>(items)...),
        std::make_index_sequence<sizeof...(Items)>());

    return sizer;
}


} // end namespace wxpex
