add_library(wxpex_catch_main STATIC catch_main.cpp)
target_link_libraries(wxpex_catch_main PUBLIC CONAN_PKG::catch2)
target_link_libraries(wxpex_catch_main PRIVATE project_options)

add_executable(
    wxpex_tests
    graphics_tests.cpp)

target_link_libraries(
    wxpex_tests
    PRIVATE
    project_warnings
    project_options
    wxpex_catch_main
    wxpex)
