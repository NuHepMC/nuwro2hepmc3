cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

find_program(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)

project(nuwroconv VERSION 0.9)

set(CMAKE_CXX_STANDARD 17)

#Changes default install path to be a subdirectory of the build dir.
#Can set build dir at configure time with -DCMAKE_INSTALL_PREFIX=/install/path
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}/"
           CACHE PATH "default install path" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE RELWITHDEBINFO)
elseif(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RELWITHDEBINFO)
endif()

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules)

#### RPATH handling #####

set(CMAKE_MACOSX_RPATH 1)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
ENDIF("${isSystemDir}" STREQUAL "-1")

######################### Dependencies

find_package(ROOT 6 REQUIRED)

include(CPM)
CPMFindPackage(
    NAME NuHepMC_CPPUtils
    GIT_REPOSITORY "https://github.com/NuHepMC/cpputils.git"
    GIT_TAG main
    OPTIONS "BUILTIN_HEPMC3 ON"
)

find_package(NuWro QUIET)
SET(nuwroconv_EXTERNAL_NuWro TRUE)
if(NOT NuWro_FOUND)
  SET(nuwroconv_EXTERNAL_NuWro FALSE)

  CPMAddPackage(
      NAME NuWro
      VERSION 21.09.02
      GIT_REPOSITORY "https://github.com/NuWro/nuwro.git"
      GIT_TAG nuwro_21.09.2
      DOWNLOAD_ONLY ON
  )

  ROOT_GENERATE_DICTIONARY(NuWroEvent
              ${NuWro_SOURCE_DIR}/src/event1.h
              LINKDEF ${NuWro_SOURCE_DIR}/src/event1LinkDef.h
              OPTIONS -p)

  add_library(nuwro_event1 SHARED NuWroEvent.cxx ${NuWro_SOURCE_DIR}/src/event1.cc)

  target_include_directories(nuwro_event1 PUBLIC 
    $<BUILD_INTERFACE:${NuWro_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>)

  target_link_libraries(nuwro_event1 PUBLIC ROOT::Tree ROOT::RIO)

  FILE(GLOB NUWROHEADERS "${NuWro_SOURCE_DIR}/src/*.h")
  set_target_properties(nuwro_event1 PROPERTIES PUBLIC_HEADER "${NUWROHEADERS}")

  install(TARGETS nuwro_event1
      EXPORT nuwroconv-targets
      LIBRARY DESTINATION lib 
      ARCHIVE DESTINATION lib 
      PUBLIC_HEADER DESTINATION include)

  install(FILES 
      ${CMAKE_CURRENT_BINARY_DIR}/libNuWroEvent.rootmap 
      ${CMAKE_CURRENT_BINARY_DIR}/libNuWroEvent_rdict.pcm 
    DESTINATION lib)

  add_library(NuWro::All ALIAS nuwro_event1)

endif()

add_subdirectory(src)
add_subdirectory(app)

install(EXPORT nuwroconv-targets
  FILE nuwroconvTargets.cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/nuwroconv
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/nuwroconvConfig.cmake.in 
  ${CMAKE_BINARY_DIR}/nuwroconvConfig.cmake
  INSTALL_DESTINATION 
    /this/is/ignored/for/some/reason/thanks/kitware
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(
  ${CMAKE_BINARY_DIR}/nuwroconvConfigVersion.cmake
  VERSION ${nuwroconv_VERSION}
  COMPATIBILITY AnyNewerVersion)

install(FILES 
    ${CMAKE_BINARY_DIR}/nuwroconvConfig.cmake 
    ${CMAKE_BINARY_DIR}/nuwroconvConfigVersion.cmake 
    ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules/FindNuWro.cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/nuwroconv)