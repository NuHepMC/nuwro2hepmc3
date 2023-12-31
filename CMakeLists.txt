cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

find_program(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)

project(nuwro2hepmc3 VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)

find_package(ROOT 6 REQUIRED)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

include(CPM)

CPMAddPackage(
    NAME NuHepMC_CPPUtils
    VERSION 0.9.0
    GIT_REPOSITORY "https://github.com/NuHepMC/cpputils.git"
    GIT_TAG main
    OPTIONS "BUILTIN_HEPMC3 ON"
            "NuHepMC_BUILD_STATIC_LIBS ON"
)

CPMAddPackage(
    NAME NuWro
    VERSION 21.09.02
    GIT_REPOSITORY "https://github.com/NuWro/nuwro.git"
    GIT_TAG nuwro_21.09.2
    DOWNLOAD_ONLY ON
)

add_subdirectory(src)
add_subdirectory(app)