add_library(wxpex)

if (${fPIC})
    set_property(TARGET wxpex PROPERTY POSITION_INDEPENDENT_CODE ON)
endif ()

# Projects that include this project must #include "wxpex/<header-name>"
target_include_directories(wxpex PUBLIC ${PROJECT_SOURCE_DIR})

target_link_libraries(
    wxpex
    PUBLIC
    CONAN_PKG::jive
    CONAN_PKG::fields
    CONAN_PKG::tau
    CONAN_PKG::pex
    CONAN_PKG::wxwidgets)

target_sources(
    wxpex
    PRIVATE
    app.h
    array_string.h
    bitset_check_boxes.h
    border_sizer.h
    button.h
    check_box.h
    collapsible.h
    color.h
    color_picker.h
    combo_box.h
    converter.h
    cursor.h
    directory_field.h
    field.h
    file_field.h
    gauge.h
    graphics.h
    ignores.h
    indent_sizer.h
    knob.h
    labeled_widget.h
    layout_top_level.h
    modifier.h
    point.h
    radio_box.h
    region.h
    scrolled.h
    shape.h
    shortcut.h
    size.h
    slider.h
    spin_control.h
    splitter.h
    splitter.cpp
    static_box.h
    style.h
    tile.h
    view.h
    window.h
    wxshim.h
    wx_ostream.h
    wx_select.h
    border_sizer.cpp
    collapsible.cpp
    gauge.cpp
    indent_sizer.cpp
    layout_top_level.cpp
    modifier.cpp
    scrolled.cpp
    shortcut.cpp
    static_box.cpp
    tile.cpp)


install(TARGETS wxpex DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/wxpex
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
