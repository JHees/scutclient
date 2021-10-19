cmake_minimum_required(VERSION 2.4)
project(scutclient)
include(GNUInstallDirs)
# ADD_DEFINITIONS(-Os -Wall)
set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)

file(GLOB SRC_FILE ./src/*)
list(FILTER SRC_FILE INCLUDE REGEX "[\\w]*.(cpp|c)$")
list(FILTER SRC_FILE EXCLUDE REGEX "test.(cpp|c)")
list(FILTER SRC_FILE EXCLUDE REGEX "main.(cpp|c)")
file(GLOB HEADER_FILE ./src/*)
list(FILTER HEADER_FILE INCLUDE REGEX "[\\w]*.(hpp|h)$")
add_executable(DrcomScutClient ${SRC_FILE} src/main.cpp)


include_directories(wirless 
    ./wireless
    ${CURL_INCLUDE_DIRS} 
)
add_executable(wireless wireless/main.cpp)
target_link_libraries(wireless 
    ${CURL_LIBRARIES}
)


install(TARGETS DrcomScutClient 
    CONFIGURATIONS Release
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES DrcomScutClient.service DrcomScutClient.timer
    CONFIGURATIONS Release
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system
)
# uninstall target
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()