if(NOT TARGET NuWro::All)

  if(NOT COMMAND EnsureVarOrEnvSet)

  # If ${OUT_VARNAME} is defined and not empty, this is a no-op
  # If ${OUT_VARNAME} is undefiend or empty then first $ENV{${VARNAME}} 
  # and then ${${VARNAME}} are checked, if either are defined and non-empty, 
  # ${OUT_VARNAME} is set equal to them in the parent scope.
  function(EnsureVarOrEnvSet OUT_VARNAME VARNAME)
    set(options UNSET_IS_FATAL)
    cmake_parse_arguments(OPTS 
                        "${options}" 
                        "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN})

    if(NOT DEFINED ${OUT_VARNAME} OR "${${OUT_VARNAME}}x" STREQUAL "x")
      if(DEFINED ENV{${VARNAME}} AND NOT "$ENV{${VARNAME}}x" STREQUAL "x")
        set(${OUT_VARNAME} $ENV{${VARNAME}} PARENT_SCOPE)
        message(DEBUG "[EnsureVarOrEnvSet] Variable \"${OUT_VARNAME}\" set to the value of environment variable \"${VARNAME}\"=\"$ENV{${VARNAME}}\"")
        return()
      endif()
      if(DEFINED ${VARNAME} AND NOT "${${VARNAME}}x" STREQUAL "x")
        set(${OUT_VARNAME} ${${VARNAME}} PARENT_SCOPE)
        message(DEBUG "[EnsureVarOrEnvSet] Variable \"${OUT_VARNAME}\" set to the value of CMake variable \"${VARNAME}\"=\"${${VARNAME}}\"")
        return()
      endif()
    else()
      message(DEBUG "[EnsureVarOrEnvSet] Variable \"${OUT_VARNAME}\" already set to \"${${OUT_VARNAME}}\"")
      return()
    endif()

    if(OPTS_UNSET_IS_FATAL)
      message(FATAL_ERROR "${OUT_VARNAME} undefined, either configure with -D${VARNAME}=<value> or set ${VARNAME} into the environment")
    else()
      message(DEBUG "[EnsureVarOrEnvSet] Variable \"${OUT_VARNAME}\" is not set as \"${VARNAME}\" is not set in CMake or in the environment.")
      set(${OUT_VARNAME} ${OUT_VARNAME}-NOTFOUND PARENT_SCOPE)
    endif()

  endfunction()
  endif()

  EnsureVarOrEnvSet(NUWRO NUWRO)

  if("${NUWRO}" STREQUAL "NUWRO-NOTFOUND")
    message(STATUS "NUWRO environment variable is not defined, assuming no NuWro build")
    SET(NuWro_FOUND FALSE)
    return()
  endif()

  find_package(ROOT 6 REQUIRED)

  include(FindPackageHandleStandardArgs)

  find_path(NuWro_INC_DIR
    NAMES dis/dis_cc.h
    PATHS ${NUWRO}/src)

  find_path(NuWro_LIB_DIR
    NAMES event1.so
    PATHS ${NUWRO}/bin)

  find_package_handle_standard_args(NuWro
    REQUIRED_VARS 
      NUWRO 
      NuWro_INC_DIR 
      NuWro_LIB_DIR
  )

  if(NuWro_FOUND)

    message(STATUS "NuWro found: ${NUWRO}")
    message(STATUS "       NuWro_INC_DIR: ${NuWro_INC_DIR}")
    message(STATUS "       NuWro_LIB_DIR: ${NuWro_LIB_DIR}")

    if(NOT TARGET NuWro::event1)
      add_library(NuWro::event1 UNKNOWN IMPORTED)
      set_target_properties(NuWro::event1 PROPERTIES
        IMPORTED_NO_SONAME ON
        IMPORTED_LOCATION ${NUWRO}/bin/event1.so
        INTERFACE_LINK_LIBRARIES "ROOT::Tree;ROOT::RIO"
        )
    endif()

    if(NOT TARGET NuWro::All)
      add_library(NuWro::All INTERFACE IMPORTED)
      set_target_properties(NuWro::All PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${NuWro_INC_DIR}"
          INTERFACE_COMPILE_OPTIONS "-DNuWro_ENABLED"
          INTERFACE_LINK_LIBRARIES NuWro::event1
      )
    endif()

  endif()

endif()