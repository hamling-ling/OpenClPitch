cmake_minimum_required(VERSION 3.7)
project(mikutuner CXX)

find_package(OpenCL REQUIRED)
find_package(OpenAL REQUIRED)

add_library(sndcap STATIC
  SoundCapture/SoundCapture/CaptureDevice.cpp
  SoundCapture/SoundCapture/RunnableService.cpp
  SoundCapture/SoundCapture/SoundCapture.cpp)

add_library(
  pitdet STATIC
  PitchDetector/PitchDetector/PitchDetector.cpp
  PitchDetector/PitchDetector/impl/PitchDetectorImpl.cpp
  PitchDetector/PitchDetector/impl/PitchDetectorNullImpl.cpp
  PitchDetector/PitchDetector/climpl/PitchDetectorClImpl.cpp
  PitchDetector/PeakDetectMachine/PeakDetectMachine.cpp
  )

configure_file(PitchDetector/PitchDetector/climpl/pitdet.cl pitdet.cl COPYONLY)

add_executable(
  mikutuner
  App/main.cpp
  )

target_compile_options(pitdet
  PUBLIC -mfp16-format=ieee
  )

target_link_libraries(mikutuner pitdet OpenCL::OpenCL sndcap openal pthread)
