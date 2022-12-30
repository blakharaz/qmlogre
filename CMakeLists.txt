cmake_minimum_required(VERSION 3.16)
project(ogreqml LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Widgets QuickWidgets)
find_package(OGRE REQUIRED COMPONENTS Bites)

qt_add_executable(ogreqml
    main.cpp
    ogrecontext.cpp ogrecontext.h
    uirenderer.cpp uirenderer.h
)

set_target_properties(ogreqml PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE

    # Prevent name clash with build subdirectory on case-insensitive file systems
    OUTPUT_NAME ogreqmlapp
)

target_link_libraries(ogreqml PUBLIC
    Qt::Core  Qt::Gui Qt::Qml Qt::Quick Qt::Widgets Qt::QuickWidgets
    OgreMain OgreBites OgreBitesQt
)

qt_add_qml_module(ogreqml
    URI ogreqml
    VERSION 1.0
    QML_FILES main.qml
    RESOURCE_PREFIX /ogreqml
    NO_RESOURCE_TARGET_PATH
)
