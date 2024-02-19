ROOT_GENERATE_DICTIONARY(NuWroEvent
            ${NuWro_SOURCE_DIR}/src/event1.h
            LINKDEF ${NuWro_SOURCE_DIR}/src/event1LinkDef.h
            OPTIONS -p)

add_library(nuwroconv SHARED nuwroconv.cxx NuWroEvent.cxx ${NuWro_SOURCE_DIR}/src/event1.cc)

FILE(GLOB NUWROHEADERS "${NuWro_SOURCE_DIR}/src/*.h")

target_link_libraries(nuwroconv PUBLIC NuHepMC::CPPUtils ROOT::Tree ROOT::RIO)
target_include_directories(nuwroconv PUBLIC 
  $<BUILD_INTERFACE:${NuWro_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(nuwroconv PRIVATE NUWROCONV_DEBUG)
endif()

target_compile_definitions(nuwroconv PRIVATE 
  NUWRO_VERSION_STR="UNKNOWN-VERSION"
  PROJECT_VERSION_STR="${PROJECT_VERSION}")

set_target_properties(nuwroconv PROPERTIES 
  PUBLIC_HEADER "nuwroconv.h;${NUWROHEADERS}")

install(FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/libNuWroEvent.rootmap 
    ${CMAKE_CURRENT_BINARY_DIR}/libNuWroEvent_rdict.pcm 
  DESTINATION lib)

install(TARGETS nuwroconv
    EXPORT nuwroconv-targets
    LIBRARY DESTINATION lib 
    ARCHIVE DESTINATION lib 
    PUBLIC_HEADER DESTINATION include)