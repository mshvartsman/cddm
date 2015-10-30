##!!! CURRENTLY BROKEN+DEPRECATED! ALL TESTS ARE COMPILED BASED ON CMAKELISTS IN /lib
##!!! Reason: we currently dependt on things in ../external and there is no good way 
## to relativize the path for out-of-source goodness

cmake_minimum_required(VERSION 2.8)
project(cddmTests CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always") # gcc flavor color
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always") # gcc flavor color

add_subdirectory(external/ConfigFile)
include_directories(external/ConfigFile)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(EnableCXX11)

find_package(Armadillo REQUIRED)

set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/catch CACHE INTERNAL "Path to include folder for Catch")

set(CONFIGFILE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ConfigFile CACHE INTERNAL "Path to include folder for ConfigFile")

FILE(GLOB Test_targets *_test.cpp)

include_directories(${ARMADILLO_INCLUDE_DIRS} ${COMMON_INCLUDES} ${CATCH_INCLUDE_DIR} ${CONFIGFILE_INCLUDE_DIR} ${TRNG_INCLUDE_DIR} ${EZOPTIONPARSER_INCLUDE_DIR})

add_executable(fatal_error_test fatal_error_test.cpp catch_main.cpp)

add_executable(rng_test rng_test.cpp catch_main.cpp ../src/rng.cpp)
target_link_libraries(rng_test ${ARMADILLO_LIBRARIES} ${TRNG_LIBRARY})

FILE(COPY test_config.cfg DESTINATION ${CMAKE_BINARY_DIR})

add_executable(config_test config_test.cpp catch_main.cpp ../src/config.cpp)
target_link_libraries(config_test ${ARMADILLO_LIBRARIES} ConfigFile)

add_executable(arch_test architecture_test.cpp catch_main.cpp ../src/config.cpp ../src/architecture.cpp ../src/rng.cpp ../src/utils.cpp)
target_link_libraries(arch_test ${TRNG_LIBRARIES} ConfigFile)

add_executable(utils_test utils_test.cpp catch_main.cpp ../src/utils.cpp)

add_executable(belief_test belief_test.cpp catch_main.cpp ../src/belief.cpp ../src/config.cpp ../src/rng.cpp)
target_link_libraries(belief_test ${ARMADILLO_LIBRARIES} ConfigFile)

add_executable(recorder_test recorder_test.cpp catch_main.cpp ../src/recorder.cpp)
target_link_libraries(recorder_test ${ARMADILLO_LIBRARIES} ConfigFile)

add_executable(task_test task_test.cpp catch_main.cpp ../src/task.cpp ../src/config.cpp ../src/belief.cpp ../src/rng.cpp ../src/recorder.cpp ../src/architecture.cpp ../src/utils.cpp)
target_link_libraries(task_test ${ARMADILLO_LIBRARIES} ConfigFile)

add_executable(experiment_test experiment_test.cpp catch_main.cpp ../src/experiment.cpp ../src/config.cpp ../src/recorder.cpp ../src/task.cpp ../src/architecture.cpp ../src/rng.cpp ../src/belief.cpp ../src/utils.cpp)
target_link_libraries(experiment_test ${ARMADILLO_LIBRARIES} ConfigFile)

add_executable(catch_main catch_main.cpp ${Test_targets} ../src/architecture.cpp ../src/rng.cpp ../src/utils.cpp ../src/config.cpp ../src/belief.cpp ../src/recorder.cpp ../src/task.cpp ../src/experiment.cpp)
target_link_libraries(catch_main ${ARMADILLO_LIBRARIES} ConfigFile)