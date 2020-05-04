cmake_minimum_required(VERSION 3.7)
project(OpenClPitch CXX)

find_package(OpenCL REQUIRED)

add_executable(
  clpitch
  OpenClPitch/main.cpp
  OpenClPitch/PeakDetectMachine.cpp
  #OpenClPitch/PeakDetectMachineCommon.h
  #OpenClPitch/PeakDetectMachine.h
  #OpenClPitch/StopWatch.h
  OpenClPitch/util.hpp
  OpenClPitch/device_picker.hpp
  #OpenClPitch/error_code.h
  )

target_compile_options(clpitch
  PUBLIC -mfp16-format=ieee
  )
target_link_libraries(clpitch OpenCL::OpenCL)

configure_file(OpenClPitch/pitdet.cl pitdet.cl COPYONLY)
