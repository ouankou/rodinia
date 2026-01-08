include(CMakeParseArguments)

function(rodinia_set_output target backend)
  if(NOT backend)
    return()
  endif()
  set(out_dir "${RODINIA_OUTPUT_ROOT}/${backend}")
  set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${out_dir}")
endfunction()

function(rodinia_add_executable target)
  set(options)
  set(oneValueArgs BACKEND OUTPUT_NAME)
  set(multiValueArgs SOURCES INCLUDES DEFINES LIBS COMPILE_OPTIONS LINK_OPTIONS LINK_DIRECTORIES)
  cmake_parse_arguments(RA "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT RA_SOURCES)
    message(FATAL_ERROR "rodinia_add_executable(${target}) missing SOURCES")
  endif()

  add_executable(${target} ${RA_SOURCES})

  if(RA_OUTPUT_NAME)
    set_target_properties(${target} PROPERTIES OUTPUT_NAME "${RA_OUTPUT_NAME}")
  endif()

  if(RA_INCLUDES)
    target_include_directories(${target} PRIVATE ${RA_INCLUDES})
  endif()

  if(RA_DEFINES)
    target_compile_definitions(${target} PRIVATE ${RA_DEFINES})
  endif()

  if(RA_COMPILE_OPTIONS)
    target_compile_options(${target} PRIVATE ${RA_COMPILE_OPTIONS})
  endif()

  if(RA_LINK_OPTIONS)
    target_link_options(${target} PRIVATE ${RA_LINK_OPTIONS})
  endif()

  if(RA_LINK_DIRECTORIES)
    target_link_directories(${target} PRIVATE ${RA_LINK_DIRECTORIES})
  endif()

  if(RA_LIBS)
    target_link_libraries(${target} PRIVATE ${RA_LIBS})
  endif()

  rodinia_set_output(${target} "${RA_BACKEND}")
endfunction()

function(rodinia_add_cuda_executable target)
  rodinia_add_executable(${target} ${ARGN})
  set_target_properties(${target} PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
endfunction()
