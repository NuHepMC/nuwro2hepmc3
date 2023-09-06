ROOT_GENERATE_DICTIONARY(NuWroEvent
            ${NuWro_SOURCE_DIR}/src/event1.h
            LINKDEF ${NuWro_SOURCE_DIR}/src/event1LinkDef.h
            OPTIONS -p)

add_library(NuWroEvent_lib SHARED NuWroEvent.cxx ${NuWro_SOURCE_DIR}/src/event1.cc)

target_link_libraries(NuWroEvent_lib PRIVATE ROOT::RIO)
target_include_directories(NuWroEvent_lib PRIVATE 
  $<BUILD_INTERFACE:${NuWro_SOURCE_DIR}/src>)

#If we want this to work properly we do need to ship some nuwro headers...
set_target_properties(NuWroEvent_lib PROPERTIES 
  LIBRARY_OUTPUT_NAME "NuWroEvent")

add_library(nuwroconv SHARED nuwroconv.cxx)

target_link_libraries(nuwroconv PUBLIC NuWroEvent_lib NuHepMC::CPPUtils ROOT::Tree ROOT::RIO)
target_include_directories(nuwroconv PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${NuWro_SOURCE_DIR}/src>)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(nuwroconv PRIVATE NUWROCONV_DEBUG)
endif()

target_compile_definitions(nuwroconv PRIVATE 
  NUWRO_VERSION_STR="UNKNOWN-VERSION"
  PROJECT_VERSION_STR="${PROJECT_VERSION}")

set_target_properties(nuwroconv PROPERTIES 
  PUBLIC_HEADER nuwroconv.h)


install(FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/libNuWroEvent.rootmap 
    ${CMAKE_CURRENT_BINARY_DIR}/libNuWroEvent_rdict.pcm 
  DESTINATION lib)

install(TARGETS nuwroconv NuWroEvent_lib
    EXPORT nuwroconv-targets
    LIBRARY DESTINATION lib 
    ARCHIVE DESTINATION lib 
    PUBLIC_HEADER DESTINATION include)