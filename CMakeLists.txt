cmake_minimum_required(VERSION 3.16)

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

qt_add_executable(pivoine
  MANUAL_FINALIZATION
  third_party/dbstring.h
  src/font.h
  src/main.cpp
  src/reservation_list.h
  src/text.h
  src/data/book_data.cpp
  src/data/book_data.h
  src/data/snack.cpp
  src/data/snack.h
  src/data/track.cpp
  src/data/track.h
  src/generator/book.h
  src/generator/cover.h
  src/generator/giant_vm.h
  src/generator/record_player.h
  src/generator/theme.h
  src/ui/project_window.cpp
  src/ui/project_window.h
  src/ui/project_window.ui
  src/ui/validate_dialog.cpp
  src/ui/validate_dialog.h
  src/ui/validate_dialog.ui
  LICENSE
  pivoine.qrc
)
target_link_libraries(pivoine PRIVATE Qt6::Widgets)
target_link_libraries(pivoine PRIVATE lcf)

include(GNUInstallDirs)
install(TARGETS pivoine
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(pivoine)
