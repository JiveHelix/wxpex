if (MSVC)
    set(windowed WIN32)
elseif (APPLE)
    set(windowed MACOSX_BUNDLE)
endif ()


add_executable(sandbox sandbox.cpp)

target_link_libraries(
    sandbox
    PRIVATE
    project_warnings
    project_options
    wxpex)


add_executable(view_demo ${windowed} view_demo.cpp)

target_link_libraries(
    view_demo
    PRIVATE
    project_warnings
    project_options
    wxpex)


add_executable(check_box_demo ${windowed} check_box_demo.cpp)

target_link_libraries(
    check_box_demo
    PRIVATE
    project_warnings
    project_options
    wxpex)


add_executable(field_demo field_demo.cpp)

target_link_libraries(
    field_demo
    PRIVATE
    project_warnings
    project_options
    wxpex)


add_executable(file_field_demo file_field_demo.cpp)

target_link_libraries(
    file_field_demo
    PRIVATE
    project_warnings
    project_options
    wxpex)


add_executable(async_demo ${windowed} async_demo.cpp)

target_link_libraries(
    async_demo
    PRIVATE
    project_warnings
    project_options
    wxpex)


add_executable(gauge_demo ${windowed} gauge_demo.cpp)

target_link_libraries(
    gauge_demo
    PRIVATE
    project_warnings
    project_options
    wxpex)


add_executable(bitset_demo ${windowed} bitset_demo.cpp)

target_link_libraries(
    bitset_demo
    PRIVATE
    project_warnings
    project_options
    wxpex)


add_executable(radio_box_demo ${windowed} radio_box_demo.cpp)

target_link_libraries(
    radio_box_demo
    PRIVATE
    project_warnings
    project_options
    wxpex)


add_executable(combo_box_demo ${windowed} combo_box_demo.cpp)

target_link_libraries(
    combo_box_demo
    PRIVATE
    project_warnings
    project_options
    wxpex)


add_executable(slider_demo ${windowed} slider_demo.cpp)

target_link_libraries(
    slider_demo
    PRIVATE
    project_warnings
    project_options
    wxpex)


# Left as a console app for demonstration.
add_executable(spin_control_demo spin_control_demo.cpp)

target_link_libraries(
    spin_control_demo
    PRIVATE
    project_warnings
    project_options
    wxpex)


add_executable(shortcut_demo shortcut_demo.cpp)

target_link_libraries(
    shortcut_demo
    PRIVATE
    project_warnings
    project_options
    wxpex)


add_executable(settings_demo settings_demo.cpp)

target_link_libraries(
    settings_demo
    PRIVATE
    project_warnings
    project_options
    wxpex)


add_executable(color_demo color_demo.cpp)

target_link_libraries(
    color_demo
    PRIVATE
    project_warnings
    project_options
    wxpex)


add_executable(transparency_demo transparency_demo.cpp)

target_link_libraries(
    transparency_demo
    PRIVATE
    project_warnings
    project_options
    wxpex)


# add_executable(drawing_demo drawing_demo.cpp)

# target_link_libraries(
#    drawing_demo
#    PRIVATE
#    project_warnings
#    project_options
#    wxpex)
