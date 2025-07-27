file(GLOB sources *.cc)
foreach(_src ${sources})
  get_filename_component(bin ${_src} NAME_WE)
  add_executable(${bin} "${_src}")
  target_link_libraries(${bin} DRS4lib)
  add_test(NAME ${bin} COMMAND ${bin})
endforeach()

