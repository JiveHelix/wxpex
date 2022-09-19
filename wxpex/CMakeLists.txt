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
    shortcut.cpp
    layout_top_level.cpp
    tile.cpp
    gauge.cpp)


install(TARGETS wxpex DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/wxpex
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
