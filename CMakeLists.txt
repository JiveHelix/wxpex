cmake_minimum_required(VERSION 3.15)

project(wxpex CXX)

include(${CMAKE_CURRENT_LIST_DIR}/cmake_includes/setup_project.cmake)
setup_project()


add_subdirectory(wxpex)


if (${PROJECT_IS_TOP_LEVEL})
    option(
        BUILD_EXAMPLES
        "Build the example targets"
        ${ENABLES_DEFAULT})
endif ()


if (${PROJECT_IS_TOP_LEVEL})

    if (ENABLE_TESTING)
        add_subdirectory(test)
    endif()

    if (${BUILD_EXAMPLES})
        add_subdirectory(examples)
    endif ()

endif ()
