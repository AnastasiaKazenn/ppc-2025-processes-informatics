project(parallel_programming_course LANGUAGES C CXX)

message(STATUS "Student's tasks")

# Test runner executables
set(FUNC_TEST_EXEC ppc_func_tests)
set(PERF_TEST_EXEC ppc_perf_tests)

# ——— Include helper scripts ——————————————————————————————————————
include(${CMAKE_SOURCE_DIR}/cmake/functions.cmake)

# ——— Initialize test executables —————————————————————————————————————
ppc_add_test(${FUNC_TEST_EXEC} common/runners/functional.cpp USE_FUNC_TESTS)
ppc_add_test(${PERF_TEST_EXEC} common/runners/performance.cpp USE_PERF_TESTS)

# ——— List of implementations ————————————————————————————————————————
set(PPC_IMPLEMENTATIONS "all;mpi;omp;seq;stl;tbb" CACHE STRING "Implementations to build (semicolon-separated)")

# ——— Configure each subproject —————————————————————————————————————
file(
  GLOB subdirs
  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/*")
# Filter out non-task directories
set(task_dirs)
foreach(sub IN LISTS subdirs)
  if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${sub}" AND NOT sub STREQUAL "common")
    list(APPEND task_dirs ${sub})
  endif()
endforeach()

# Use single-task libraries for all platforms
message(STATUS "Using single-task library system (${CMAKE_CXX_COMPILER_ID})")

# Configure each task as separate library
foreach(sub IN LISTS task_dirs)
  ppc_configure_single_subproject(${sub})
endforeach()