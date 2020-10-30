add_executable(window_menu_tests ${CMAKE_CURRENT_SOURCE_DIR}/window_menu_tests.cpp)
target_include_directories(
  window_menu_tests
  PUBLIC ${CMAKE_SOURCE_DIR}/src/guiapi/include
         ${CMAKE_SOURCE_DIR}/src
         ${CMAKE_SOURCE_DIR}/src/common
         ${CMAKE_SOURCE_DIR}/src/lang
         ${CMAKE_SOURCE_DIR}/src/gui
         ${CMAKE_SOURCE_DIR}/src/gui/dialogs
         ${CMAKE_SOURCE_DIR}/src/gui/wizard
  )

add_catch_test(window_menu_tests)
