if (MSVC)
    set(windowed WIN32)
elseif (APPLE)
    set(windowed MACOSX_BUNDLE)
endif ()

add_library(console_app INTERFACE)
target_compile_definitions(console_app INTERFACE _CONSOLE_APP)

target_link_libraries(
    console_app
    INTERFACE
    project_warnings
    project_options
    wxpex)

add_library(windowed_app INTERFACE)

target_link_libraries(
    windowed_app
    INTERFACE
    project_warnings
    project_options
    wxpex)

add_executable(sandbox sandbox.cpp)

target_link_libraries(
    sandbox
    PRIVATE
    console_app)

add_executable(knob_demo knob_demo.cpp)

target_link_libraries(
    knob_demo
    PRIVATE
    console_app)


add_executable(view_demo view_demo.cpp)

target_link_libraries(
    view_demo
    PRIVATE
    console_app)


add_executable(scroll_demo scroll_demo.cpp)

target_link_libraries(
    scroll_demo
    PRIVATE
    console_app)


add_executable(splitter_demo splitter_demo.cpp)

target_link_libraries(
    splitter_demo
    PRIVATE
    console_app)


add_executable(check_box_demo ${windowed} check_box_demo.cpp)

target_link_libraries(
    check_box_demo
    PRIVATE
    windowed_app)

add_executable(converter_demo converter_demo.cpp)

target_link_libraries(
    converter_demo
    PRIVATE
    console_app)


add_executable(field_demo field_demo.cpp)

target_link_libraries(
    field_demo
    PRIVATE
    console_app)


add_executable(file_field_demo file_field_demo.cpp)

target_link_libraries(
    file_field_demo
    PRIVATE
    console_app)


add_executable(async_demo async_demo.cpp)

target_link_libraries(
    async_demo
    PRIVATE
    console_app)


add_executable(gauge_demo gauge_demo.cpp)

target_link_libraries(
    gauge_demo
    PRIVATE
    console_app)


add_executable(bitset_demo bitset_demo.cpp)

target_link_libraries(
    bitset_demo
    PRIVATE
    console_app)


add_executable(radio_box_demo radio_box_demo.cpp)

target_link_libraries(
    radio_box_demo
    PRIVATE
    console_app)


add_executable(combo_box_demo combo_box_demo.cpp)

target_link_libraries(
    combo_box_demo
    PRIVATE
    console_app)


add_executable(combo_box_muxer_demo combo_box_muxer_demo.cpp)

target_link_libraries(
    combo_box_muxer_demo
    PRIVATE
    console_app)


add_executable(slider_demo slider_demo.cpp)

target_link_libraries(
    slider_demo
    PRIVATE
    console_app)


add_executable(spin_control_demo spin_control_demo.cpp)

target_link_libraries(
    spin_control_demo
    PRIVATE
    console_app)


add_executable(shortcut_demo shortcut_demo.cpp)

target_link_libraries(
    shortcut_demo
    PRIVATE
    console_app)


add_executable(settings_demo settings_demo.cpp)

target_link_libraries(
    settings_demo
    PRIVATE
    console_app)


add_executable(collapsible_demo collapsible_demo.cpp)

target_link_libraries(
    collapsible_demo
    PRIVATE
    console_app)


add_executable(color_demo color_demo.cpp)

target_link_libraries(
    color_demo
    PRIVATE
    console_app)


add_executable(transparency_demo transparency_demo.cpp)

target_link_libraries(
    transparency_demo
    PRIVATE
    console_app)


add_executable(json_viewer json_viewer.cpp)

target_link_libraries(
    json_viewer
    PRIVATE
    console_app)
