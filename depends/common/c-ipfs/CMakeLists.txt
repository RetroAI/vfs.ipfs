project(c-ipfs)

cmake_minimum_required(VERSION 2.6)

include(ExternalProject)

ExternalProject_Add(${PROJECT_NAME}
  SOURCE_DIR ${CMAKE_SOURCE_DIR}
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
  BUILD_IN_SOURCE 1
)
