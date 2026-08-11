cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
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
    MainWindow.h
    MainWindow.ui
    KeyboardPreview.cpp
    KeyboardPreview.h
    accent/AccentProvider.h
    accent/AccentManager.h
    accent/AccentManager.cpp
    accent/providers/NoctaliaV5Provider.h
    accent/providers/NoctaliaV5Provider.cpp
    accent/providers/NoctaliaV4Provider.h
    accent/providers/NoctaliaV4Provider.cpp
    accent/providers/MatugenProvider.h
    accent/providers/MatugenProvider.cpp
    accent/providers/Pywal16Provider.h
    accent/providers/Pywal16Provider.cpp
    accent/providers/PywalProvider.h
    accent/providers/PywalProvider.cpp
    accent/providers/WallustProvider.h
    accent/providers/WallustProvider.cpp
    accent/providers/WpgtkProvider.h
    accent/providers/WpgtkProvider.cpp
    accent/providers/KdeProvider.h
    accent/providers/KdeProvider.cpp
    accent/providers/GnomeProvider.h
    accent/providers/GnomeProvider.cpp
)

target_link_libraries(legionaura-gui
    Qt6::Widgets
    ${LIBUSB_LIBRARIES}
    legionaura_lib
)

install(TARGETS legionaura-gui
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

