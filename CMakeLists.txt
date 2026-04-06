cmake_minimum_required(VERSION 3.16)
project(can_frame_visualizer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(appcan_frame_visualizer
    main.cpp
    include/CANBackend.hpp
    src/CANFrame.cpp
    src/CRC15.cpp
    src/BitStuffing.cpp
    src/CANBackend.cpp
)

qt_add_qml_module(appcan_frame_visualizer
    URI can_frame_visualizer
    QML_FILES
        Main.qml
)

target_include_directories(appcan_frame_visualizer PRIVATE include)

set_target_properties(appcan_frame_visualizer PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appcan_frame_visualizer
    PRIVATE Qt6::Quick
)

install(TARGETS appcan_frame_visualizer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)