cmake_minimum_required(VERSION 3.16.3)
project(DLB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(eRPC/src)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
#find_package(Torch REQUIRED)

FIND_PACKAGE( Boost 1.7.1 COMPONENTS program_options log REQUIRED )
find_package(GSL)
include_directories(${GSL_INCLUDE_DIR})

find_package(ZLIB)
add_definitions(-DBOOST_LOG_DYN_LINK)

add_compile_definitions(ERPC_INFINIBAND)

if(DEFINE_DLB)
    add_definitions(-DDLB)
endif(DEFINE_DLB)

if (DEFINE_POWD)
    add_definitions(-DPOWD)
endif (DEFINE_POWD)

if(DEFINE_JBT)
    add_definitions(-DJBT)
endif(DEFINE_JBT)

if(DEFINE_JSQ)
    add_definitions(-DJSQ)
endif(DEFINE_JSQ)

if(DEFINE_FLUCTUATE_LOAD)
    add_definitions(-DFLUCTUATE_LOAD)
endif(DEFINE_FLUCTUATE_LOAD)

if(DEFINE_CHANGE_SPEED)
    add_definitions(-DCHANGE_SPEED)
endif(DEFINE_CHANGE_SPEED)

set(server_srcs src/core/server.cpp
        src/RL/AC_AVX2/AC.cpp src/RL/AC_AVX2/memory.cpp src/RL/AC_AVX2/uniformInit.cpp)

add_library(server "${server_srcs}")
target_link_directories(server PUBLIC eRPC/build)

target_link_libraries(server PUBLIC boost_program_options erpc pthread numa dl ibverbs
         glog gflags pthread double-conversion iberty dl fmt GSL::gsl GSL::gslcblas)

set(driver_srcs src/core/driver.cpp)

add_library(driver "${driver_srcs}")
target_link_directories(driver PUBLIC eRPC/build)

target_link_libraries(driver PUBLIC boost_program_options erpc pthread numa dl ibverbs
         glog gflags pthread double-conversion iberty dl fmt GSL::gsl GSL::gslcblas)



add_executable(SyntheticDataDrivers src/tests/synthetic_distributions/SyntheticDataDrivers.cpp "${driver_srcs}")
add_executable(SyntheticDataServers src/tests/synthetic_distributions/SyntheticDataServers.cpp "${server_srcs}")

target_link_libraries(SyntheticDataDrivers PUBLIC boost_program_options boost_log_setup
        erpc pthread numa dl ibverbs ${Boost_LIBRARIES}
         glog gflags pthread double-conversion iberty dl fmt GSL::gsl GSL::gslcblas)
target_link_libraries(SyntheticDataServers PUBLIC boost_program_options boost_log_setup
        erpc pthread numa dl ibverbs ${Boost_LIBRARIES}
         glog gflags pthread double-conversion iberty dl fmt GSL::gsl GSL::gslcblas)

target_link_directories(SyntheticDataDrivers PUBLIC eRPC/build)
target_link_directories(SyntheticDataServers PUBLIC eRPC/build)

