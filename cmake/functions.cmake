# ——— Helper function to add & register tests —————————————————————————
function(ppc_add_test test_name test_src USE_FLAG)
  if(${USE_FLAG})
    add_executable(${test_name} "${PROJECT_SOURCE_DIR}/${test_src}")
    enable_testing()
    add_test(NAME ${test_name} COMMAND ${test_name})
    install(TARGETS ${test_name} RUNTIME DESTINATION bin)
  endif()
endfunction()

# Function to configure tests
function(add_tests test_flag exec_target subdir)
  if(${test_flag})
    # Gather all source files under tests/<subdir>
    file(GLOB_RECURSE src_files "${TEST_DIR}/${subdir}/*.cpp"
         "${TEST_DIR}/${subdir}/*.cxx" "${TEST_DIR}/${subdir}/*.cc")
    target_sources(${exec_target} PRIVATE ${src_files})
    list(APPEND TEST_EXECUTABLES ${exec_target})
    set(TEST_EXECUTABLES
        "${TEST_EXECUTABLES}"
        PARENT_SCOPE)
  endif()
endfunction()

# ============================================================================
# Function: setup_implementation - NAME:       implementation sub‐directory name
# (e.g. “mpi”) - PROJ_NAME:  project base name - BASE_DIR:   root source
# directory - TESTS:      list of test executables to link against
# ============================================================================
function(setup_implementation)
  # parse named args: NAME, PROJ_NAME, BASE_DIR; multi‐value: TESTS
  cmake_parse_arguments(SETUP "" # no plain options
                        "NAME;PROJ_NAME;BASE_DIR" "TESTS" ${ARGN})

  # skip if impl dir doesn't exist
  set(IMP_DIR "${SETUP_BASE_DIR}/${SETUP_NAME}")
  if(NOT EXISTS "${IMP_DIR}")
    return()
  endif()
  message(STATUS "  -- ${SETUP_NAME}")

  # collect sources
  file(GLOB_RECURSE CPP_SOURCES "${IMP_DIR}/src/*.cpp")
  file(GLOB_RECURSE ALL_SOURCES "${IMP_DIR}/include/*.h"
       "${IMP_DIR}/include/*.hpp" "${IMP_DIR}/src/*.cpp")

  # create library (STATIC if .cpp exist, otherwise INTERFACE)
  set(LIB_NAME "${SETUP_PROJ_NAME}_${SETUP_NAME}")
  if(CPP_SOURCES)
    add_library(${LIB_NAME} STATIC ${ALL_SOURCES})
  else()
    add_library(${LIB_NAME} INTERFACE ${ALL_SOURCES})
  endif()

  # link core module
  target_link_libraries(${LIB_NAME} PUBLIC core_module_lib)

  # and link into each enabled test executable
  foreach(test_exec ${SETUP_TESTS})
    target_link_libraries(${test_exec} PUBLIC ${LIB_NAME})
  endforeach()
endfunction()

# Function to configure each subproject
function(ppc_configure_subproject SUBDIR)
  # Module-specific compile-time definitions
  add_compile_definitions(
    PPC_SETTINGS_${SUBDIR}="${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR}/settings.json"
    PPC_ID_${SUBDIR}="${SUBDIR}")

  # Switch project context to the subproject
  project(${SUBDIR})

  # Directory with tests and list of test executables (populated by
  # setup_implementation)
  set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR}/tests")
  set(TEST_EXECUTABLES "")

  # Register functional and performance test runners
  add_tests(USE_FUNC_TESTS ${FUNC_TEST_EXEC} functional)
  add_tests(USE_PERF_TESTS ${PERF_TEST_EXEC} performance)

  message(STATUS "${SUBDIR}")

  # List of implementations to configure
  foreach(IMPL IN LISTS PPC_IMPLEMENTATIONS)
    setup_implementation(
      NAME
      ${IMPL}
      PROJ_NAME
      ${SUBDIR}
      TESTS
      "${TEST_EXECUTABLES}"
      BASE_DIR
      "${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR}")
  endforeach()
endfunction()

# Function to configure single subproject as static library
function(ppc_configure_single_subproject SUBDIR)
  if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR}"
     OR SUBDIR STREQUAL "common")
    return()
  endif()

  message(STATUS "  ${SUBDIR}")

  # Create individual library for this task
  set(TASK_LIB_NAME "ppc_task_${SUBDIR}")
  set(TASK_HAS_SOURCES FALSE)
  set(ALL_TASK_SOURCES)
  set(ALL_TASK_INCLUDE_DIRS)

  # Module-specific compile-time definitions
  add_compile_definitions(
    PPC_SETTINGS_${SUBDIR}="${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR}/settings.json"
    PPC_ID_${SUBDIR}="${SUBDIR}")

  # Switch project context to the subproject
  project(${SUBDIR})

  # Register functional and performance test runners
  set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR}/tests")
  set(TEST_EXECUTABLES "")
  add_tests(USE_FUNC_TESTS ${FUNC_TEST_EXEC} functional)
  add_tests(USE_PERF_TESTS ${PERF_TEST_EXEC} performance)

  # Collect test sources
  if(EXISTS "${TEST_DIR}/functional")
    file(GLOB_RECURSE FUNC_TEST_SOURCES "${TEST_DIR}/functional/*.cpp")
    if(FUNC_TEST_SOURCES)
      list(APPEND ALL_TASK_SOURCES ${FUNC_TEST_SOURCES})
      set(TASK_HAS_SOURCES TRUE)
    endif()
  endif()

  if(EXISTS "${TEST_DIR}/performance")
    file(GLOB_RECURSE PERF_TEST_SOURCES "${TEST_DIR}/performance/*.cpp")
    if(PERF_TEST_SOURCES)
      list(APPEND ALL_TASK_SOURCES ${PERF_TEST_SOURCES})
      set(TASK_HAS_SOURCES TRUE)
    endif()
  endif()

  # Collect implementation sources
  foreach(IMPL IN LISTS PPC_IMPLEMENTATIONS)
    set(IMP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR}/${IMPL}")
    if(EXISTS "${IMP_DIR}")
      file(GLOB_RECURSE IMPL_SOURCES "${IMP_DIR}/src/*.cpp")
      if(IMPL_SOURCES)
        list(APPEND ALL_TASK_SOURCES ${IMPL_SOURCES})
        list(APPEND ALL_TASK_INCLUDE_DIRS "${IMP_DIR}/include")
        set(TASK_HAS_SOURCES TRUE)
      endif()
    endif()
  endforeach()

  # Create library only if we have sources
  if(TASK_HAS_SOURCES)
    add_library(${TASK_LIB_NAME} STATIC ${ALL_TASK_SOURCES})

    # Set include directories
    if(ALL_TASK_INCLUDE_DIRS)
      list(REMOVE_DUPLICATES ALL_TASK_INCLUDE_DIRS)
      target_include_directories(${TASK_LIB_NAME}
                                 PUBLIC ${ALL_TASK_INCLUDE_DIRS})
    endif()

    # Set task-specific definitions
    target_compile_definitions(
      ${TASK_LIB_NAME}
      PRIVATE
        PPC_SETTINGS_${SUBDIR}="${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR}/settings.json"
        PPC_ID_${SUBDIR}="${SUBDIR}")

    # Link core module
    target_link_libraries(${TASK_LIB_NAME} PUBLIC core_module_lib)

    # Link to main executables
    if(USE_FUNC_TESTS)
      target_link_libraries(${FUNC_TEST_EXEC} PUBLIC ${TASK_LIB_NAME})
    endif()
    if(USE_PERF_TESTS)
      target_link_libraries(${PERF_TEST_EXEC} PUBLIC ${TASK_LIB_NAME})
    endif()
  endif()
endfunction()
