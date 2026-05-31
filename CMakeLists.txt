cmake_minimum_required(VERSION 3.16)
project(TeyvatTatics VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

set(CORE_SOURCES
    core/CharacterBase.cpp
    core/KeQing.cpp
    core/TestCharacter.cpp
    core/Weapon.cpp
    core/Artifact.cpp
    core/Team.cpp
    core/Board.cpp
    core/ElementSystem.cpp
    core/Shop.cpp
    core/AIController.cpp
    core/GameEngine.cpp
    core/SaveManager.cpp
)

set(CORE_HEADERS
    core/GameTypes.h
    core/CharacterBase.h
    core/KeQing.h
    core/TestCharacter.h
    core/Weapon.h
    core/Artifact.h
    core/Team.h
    core/Board.h
    core/ElementSystem.h
    core/Shop.h
    core/AIController.h
    core/GameEngine.h
    core/SaveManager.h
)

set(UI_SOURCES
    ui/MainWindow.cpp
    ui/BoardWidget.cpp
    ui/PieceItem.cpp
    ui/ShopWidget.cpp
    ui/StorageBar.cpp
    ui/InfoPanel.cpp
    ui/BattleLogWidget.cpp
    ui/BackpackWidget.cpp
    ui/MergePanel.cpp
)

set(UI_HEADERS
    ui/MainWindow.h
    ui/BoardWidget.h
    ui/PieceItem.h
    ui/ShopWidget.h
    ui/StorageBar.h
    ui/InfoPanel.h
    ui/DragDropMimeData.h
    ui/BattleLogWidget.h
    ui/BackpackWidget.h
    ui/MergePanel.h
)

set(RESOURCES
    resources/resources.qrc
)

add_executable(${PROJECT_NAME}
    main.cpp
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${UI_SOURCES}
    ${UI_HEADERS}
    ${RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/ui
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)
