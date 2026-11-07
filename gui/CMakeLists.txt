cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

include_directories(
    ${LIBUSB_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(legionaura-gui
    main.cpp
    MainWindow.cpp
    ScreenSync.cpp
    ScreenSync.h
    ScreenColors.h
    MainWindow.h
    MainWindow.ui
    KeyboardPreview.cpp
    KeyboardPreview.h
    accent/AccentProvider.h
    accent/AccentManager.h
    accent/AccentManager.cpp
)

target_link_libraries(legionaura-gui
    Qt6::Widgets
    Qt6::Multimedia
    Threads::Threads
    ${LIBUSB_LIBRARIES}
    legionaura_lib
)

install(TARGETS legionaura-gui
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
