add_executable(
  window_menu_tests
  ${CMAKE_CURRENT_SOURCE_DIR}/window_menu_tests.cpp ${CMAKE_SOURCE_DIR}/src/lang/translator.cpp
  ${CMAKE_SOURCE_DIR}/src/guiapi/src/window_menu.cpp
  )
target_include_directories(
  window_menu_tests
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../stub/ ${CMAKE_SOURCE_DIR}/src/guiapi/include
         ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/common ${CMAKE_SOURCE_DIR}/src/lang
         ${CMAKE_SOURCE_DIR}/src/gui
  )

add_catch_test(window_menu_tests)
