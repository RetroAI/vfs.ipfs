project(lmdb)

cmake_minimum_required(VERSION 2.6)

include(ExternalProject)

ExternalProject_Add(${PROJECT_NAME}
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/libraries/liblmdb
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
  TEST_BEFORE_INSTALL 1
  BUILD_IN_SOURCE 1
)
