cmake_minimum_required(VERSION 3.14)

project(pivoine LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

option(USE_SYSTEM_LIBLCF "Use system install of liblcf" OFF)
if(USE_SYSTEM_LIBLCF)
        find_package(liblcf REQUIRED)
else(USE_SYSTEM_LIBLCF)
        function(add_liblcf)
                set(BUILD_SHARED_LIBS OFF)
                set(LIBLCF_ENABLE_INSTALL OFF)
                add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/liblcf)
        endfunction()
        add_liblcf()
endif(USE_SYSTEM_LIBLCF)

add_executable(pivoine
  third_party/dbstring.h
  src/main.cpp
  src/projectwindow.cpp
  src/projectwindow.h
  src/projectwindow.ui
  src/track.h
  src/record_player.h
  src/book.h
  src/book_data.h
  src/cover.h
  src/theme.h
  src/text.h
)
target_link_libraries(pivoine PRIVATE Qt6::Widgets)
target_link_libraries(pivoine PRIVATE lcf)

include(GNUInstallDirs)
install(TARGETS pivoine
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(pivoine)
