cmake_minimum_required(VERSION 3.12)

find_package(Eigen3 REQUIRED)
find_package(Boost REQUIRED)

set(KSOLVE_SRCS
    KinSparseMatrix.cpp
    ZombiePool.cpp
    ZombieFunction.cpp
    ZombieBufPool.cpp
    ZombieReac.cpp
    ZombieEnz.cpp
    ZombieMMenz.cpp
    VoxelPoolsBase.cpp
    VoxelPools.cpp
    GssaVoxelPools.cpp
    RateTerm.cpp
    FuncTerm.cpp
    Stoich.cpp
    Ksolve.cpp
    Gsolve.cpp
    ZombiePoolInterface.cpp
    SteadyState.cpp
    testKsolve.cpp
    )

add_library( ksolve ${KSOLVE_SRCS} )
target_link_libraries(ksolve PUBLIC Boost::headers Eigen3::Eigen3)
