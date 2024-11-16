cmake_minimum_required(VERSION 3.30)
project(DRS4lib)

find_package(ROOT REQUIRED)

file(GLOB sources src/*.cc)
set(executables listGoodEvents maketree merge)
set(executables_sources)
foreach(_exec ${executables})
  file(GLOB src src/${_exec}.cc)
  list(REMOVE_ITEM sources ${src})
  list(APPEND ${executables_sources} ${src})
endforeach()

add_library(DRS4lib SHARED ${sources})
target_link_libraries(DRS4lib PUBLIC ROOT::Core)

foreach(_exec_src ${executables_sources})
  get_filename_component(exec_bin ${_exec_src} NAME_WE)
  add_executable(${_exec_src} ${exec_bin})
  target_link_libraries(${exec_bin} DRS4lib)
  install(TARGETS ${exec_bin} DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()
