cmake_minimum_required(VERSION 2.8)

include_directories( ../basecode ../utility ../kinetics)
include_directories( ../external/muparserx/src )

set( SYNAPSE_SRCS
    GraupnerBrunel2012CaPlasticitySynHandler.cpp
    RollingMatrix.cpp
    SeqSynHandler.cpp
    SimpleSynHandler.cpp
    STDPSynapse.cpp
    STDPSynHandler.cpp
    Synapse.cpp
    SynHandlerBase.cpp
    testSynapse.cpp
    )

add_library(synapse ${SYNAPSE_SRCS} )
