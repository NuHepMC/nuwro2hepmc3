add_executable(nuwro2hepmc3 nuwro2hepmc3.cxx)

target_link_libraries(nuwro2hepmc3 PRIVATE nuwroconv)

set_target_properties(nuwro2hepmc3 PROPERTIES 
  INSTALL_RPATH "\${ORIGIN}/../lib")

install(TARGETS nuwro2hepmc3 EXPORT nuwro2hepmc3-targets)