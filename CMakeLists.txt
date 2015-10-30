cmake_minimum_required(VERSION 2.8)
project(axcpt_cpp CXX)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics") # clang flavor color
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics") # clang flavor color
elseif(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always") # gcc flavor color
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always") # gcc flavor color
endif() 

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

add_subdirectory(external/ConfigFile)
add_subdirectory(external/armadillo)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(EnableCXX11)

find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
	${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating documentation with Doxygen" VERBATIM
	)
endif(DOXYGEN_FOUND)


set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/catch CACHE INTERNAL "Path to include folder for Catch")
set(CONFIGFILE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ConfigFile CACHE INTERNAL "Path to include folder for ConfigFile")
set(ARMADILLO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external/armadillo/include)

FILE(GLOB Test_targets tests/*_test.cpp)

include_directories(${ARMADILLO_INCLUDE_DIRS} ${COMMON_INCLUDES} ${CATCH_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(fatal_error_test tests/fatal_error_test.cpp tests/catch_main.cpp)

add_executable(rng_test tests/rng_test.cpp tests/catch_main.cpp rng.cpp)
target_link_libraries(rng_test armadillo ${TRNG_LIBRARY})

FILE(COPY tests/test_config.cfg DESTINATION ${CMAKE_BINARY_DIR})

add_executable(config_test tests/config_test.cpp tests/catch_main.cpp config.cpp)
target_link_libraries(config_test armadillo ConfigFile)

add_executable(arch_test tests/architecture_test.cpp tests/catch_main.cpp config.cpp architecture.cpp rng.cpp utils.cpp)
target_link_libraries(arch_test ${TRNG_LIBRARIES} ConfigFile)

add_executable(utils_test tests/utils_test.cpp tests/catch_main.cpp utils.cpp)

add_executable(belief_test tests/belief_test.cpp tests/catch_main.cpp belief.cpp config.cpp rng.cpp)
target_link_libraries(belief_test armadillo ConfigFile)

add_executable(recorder_test tests/recorder_test.cpp tests/catch_main.cpp recorder.cpp rng.cpp)
target_link_libraries(recorder_test armadillo ConfigFile)

add_executable(task_test tests/task_test.cpp tests/catch_main.cpp task.cpp config.cpp belief.cpp rng.cpp recorder.cpp architecture.cpp utils.cpp)
target_link_libraries(task_test armadillo ConfigFile)

add_executable(experiment_test tests/experiment_test.cpp tests/catch_main.cpp experiment.cpp config.cpp recorder.cpp task.cpp architecture.cpp rng.cpp belief.cpp utils.cpp)
target_link_libraries(experiment_test armadillo ConfigFile)

add_executable(catch_main tests/catch_main.cpp ${Test_targets} architecture.cpp rng.cpp utils.cpp config.cpp belief.cpp recorder.cpp task.cpp experiment.cpp)
target_link_libraries(catch_main armadillo ConfigFile)

add_library(cddm SHARED architecture.cpp rng.cpp utils.cpp config.cpp belief.cpp recorder.cpp task.cpp experiment.cpp)
target_link_libraries(cddm armadillo ConfigFile)

add_executable(axcpt_batch examples/AX-CPT/axcpt_batch_runner.cpp examples/AX-CPT/axcpt.cpp)
target_link_libraries(axcpt_batch cddm)

add_executable(axcpt_trace examples/AX-CPT/axcpt_trace_runner.cpp examples/AX-CPT/axcpt.cpp)
target_link_libraries(axcpt_trace cddm)

add_executable(axcpt_event examples/AX-CPT/axcpt_event_runner.cpp examples/AX-CPT/axcpt.cpp)
target_link_libraries(axcpt_event cddm)

add_executable(flanker_batch examples/Flanker/flanker_batch_runner.cpp examples/Flanker/flanker.cpp)
target_link_libraries(flanker_batch cddm)

add_executable(flanker_trace examples/Flanker/flanker_trace_runner.cpp examples/Flanker/flanker.cpp)
target_link_libraries(flanker_trace cddm)

add_executable(flanker_event examples/Flanker/flanker_event_runner.cpp examples/Flanker/flanker.cpp)
target_link_libraries(flanker_event cddm)

add_custom_target(examples)
add_dependencies(examples axcpt_trace axcpt_batch flanker_trace flanker_batch)
