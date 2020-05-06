cmake_minimum_required(VERSION 3.7)
project(OpenClPitch CXX)

find_package(OpenCL REQUIRED)
find_package(OpenAL REQUIRED)

add_library(sndcap STATIC
  SoundCapture/SoundCapture/CaptureDevice.cpp
  SoundCapture/SoundCapture/RunnableService.cpp
  SoundCapture/SoundCapture/SoundCapture.cpp)

add_executable(
  clpitch
  OpenClPitch/main.cpp
  OpenClPitch/PeakDetectMachine.cpp
  OpenClPitch/util.hpp
  OpenClPitch/device_picker.hpp
  )

target_compile_options(clpitch
  PUBLIC -mfp16-format=ieee
  )

target_link_libraries(clpitch OpenCL::OpenCL sndcap openal)

configure_file(OpenClPitch/pitdet.cl pitdet.cl COPYONLY)
