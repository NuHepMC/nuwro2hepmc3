add_library(nuwroconv SHARED nuwroconv.cxx)

target_link_libraries(nuwroconv PUBLIC NuHepMC::CPPUtils NuWro::All)
target_include_directories(nuwroconv PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(nuwroconv PRIVATE NUWROCONV_DEBUG)
endif()

target_compile_definitions(nuwroconv PRIVATE 
  NUWRO_VERSION_STR="UNKNOWN-VERSION"
  PROJECT_VERSION_STR="${PROJECT_VERSION}")

set_target_properties(nuwroconv PROPERTIES 
  PUBLIC_HEADER "nuwroconv.h")

install(TARGETS nuwroconv
    EXPORT nuwroconv-targets
    LIBRARY DESTINATION lib 
    ARCHIVE DESTINATION lib 
    PUBLIC_HEADER DESTINATION include)