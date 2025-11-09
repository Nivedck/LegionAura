add_library(legionaura_lib
    legionaura.cpp
    legionaura.h
)

target_include_directories(legionaura_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(legionaura_lib PUBLIC PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")


find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

target_link_libraries(legionaura_lib PUBLIC ${LIBUSB_LIBRARIES})
