CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
PROJECT(example_perception_camera)

SET(CMAKE_C_STANDARD 11)
SET(CMAKE_CXX_STANDARD 11)

SET(CMAKE_C_FLAGS "-fPIC -g -DPIC -Wall -Werror=return-type -Wl,-z,defs")
SET(CMAKE_CXX_FLAGS "-fPIC -g -DPIC -Wall -Werror=return-type -Wl,-z,defs")
SET(CUDA_NVCC_FLAGS "-std=c++11 -O3 -Xcompiler -fPIC --expt-relaxed-constexpr")

### output path
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)

### install path
SET(CMAKE_INSTALL_PREFIX "$ENV{HOME}/${CMAKE_PROJECT_NAME}")
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

FIND_PACKAGE(CUDA REQUIRED)

INCLUDE_DIRECTORIES(
  "/opt/airos/sdk/airos/include"
  "/usr/local/include"
  ${CUDA_TOOLKIT_ROOT_DIR}/include
)

LINK_DIRECTORIES(
  "/opt/airos/sdk/airos/lib"
  "/usr/local/lib"
  ${CUDA_TOOLKIT_ROOT_DIR}/lib64/stubs
  ${CUDA_TOOLKIT_ROOT_DIR}/lib64
)

ADD_LIBRARY(example_perception_camera SHARED
  example_perception_camera.cc
  )
TARGET_LINK_LIBRARIES(example_perception_camera
  protobuf
  airos_base
  airos_service_pb
  glog
  cuda
  -ldl
  )