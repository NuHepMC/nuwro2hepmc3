add_executable(nuwro2hepmc3 nuwro2hepmc3.cxx)

target_link_libraries(nuwro2hepmc3 PRIVATE nuwroconv)

if(ZLIB_FOUND)
  target_link_libraries(nuwro2hepmc3 PRIVATE ZLIB::ZLIB)
  target_compile_definitions(nuwro2hepmc3 PRIVATE 
    HEPMC3_USE_COMPRESSION
    HEPMC3_Z_SUPPORT)
endif()


install(TARGETS nuwro2hepmc3 EXPORT nuwro2hepmc3-targets)