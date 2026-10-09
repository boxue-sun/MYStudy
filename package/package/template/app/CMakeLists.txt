CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
PROJECT(example_app)

SET(CMAKE_C_STANDARD 11)
SET(CMAKE_CXX_STANDARD 11)

SET(CMAKE_C_FLAGS "-fPIC -g -DPIC -Wall -Werror=return-type -Wl,-z,defs")
SET(CMAKE_CXX_FLAGS "-fPIC -g -DPIC -Wall -Werror=return-type -Wl,-z,defs")

### output path
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)

### install path
SET(CMAKE_INSTALL_PREFIX "$ENV{HOME}/${CMAKE_PROJECT_NAME}")
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

INCLUDE_DIRECTORIES(
  "/opt/airos/sdk/airos/include"
  "/usr/local/include"
)
LINK_DIRECTORIES(
  "/opt/airos/sdk/airos/lib"
  "/usr/local/lib"
)

ADD_LIBRARY(example_app SHARED
  example_app.cc
  )
TARGET_LINK_LIBRARIES(example_app
  app_interface
  airos_app_pb
  protobuf
  glog
  )