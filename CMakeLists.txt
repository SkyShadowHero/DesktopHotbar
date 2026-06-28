cmake_minimum_required(VERSION 3.16)
project(DesktopHotbar VERSION 0.0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(DesktopHotbar
    resources.qrc
    src/main.cpp
    src/ConfigManager.h src/ConfigManager.cpp
    src/SlotLabel.h src/SlotLabel.cpp
    src/SlotSettingsDialog.h src/SlotSettingsDialog.cpp
    src/GeneralSettingsDialog.h src/GeneralSettingsDialog.cpp
    src/HotbarWindow.h src/HotbarWindow.cpp
)

target_link_libraries(DesktopHotbar PRIVATE Qt6::Widgets)

# 安装规则
install(TARGETS DesktopHotbar RUNTIME DESTINATION bin)
install(FILES icon.png DESTINATION share/icons/hicolor/256x256/apps RENAME mcdesktophotbar.png)
