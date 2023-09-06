add_executable(nuwro2hepmc3 nuwro2hepmc3.cxx)

target_link_libraries(nuwro2hepmc3 PRIVATE nuwroconv)

install(TARGETS nuwro2hepmc3 EXPORT nuwro2hepmc3-targets)