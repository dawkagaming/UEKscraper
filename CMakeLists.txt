cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

project(uekscraper
    VERSION 0.2
    #SPDX_LICENSE "GPL-3.0+"
    DESCRIPTION "UEKscraper is a tool to scrap Cracow's University of Economics time schedule"
    HOMEPAGE_URL "https://github.com/dawkagaming/UEKscraper"
    LANGUAGES CXX
)

add_compile_definitions(
    QT_DEPRECATED_WARNINGS
    QT_MESSAGELOGCONTEXT
)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Network
    HttpServer
)

qt_standard_project_setup()

set(SOURCES
    main.cpp
    parser.cpp
    server.cpp
)

set(HEADERS
   	parser.hpp
  	server.hpp
)

qt_add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)


target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::HttpServer
)

include(GNUInstallDirs)

if(LINUX)
    install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
