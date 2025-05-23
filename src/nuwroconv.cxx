#include "nuwroconv.h"

#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"
#include "HepMC3/Print.h"

#include "NuHepMC/Constants.hxx"
#include "NuHepMC/UnitsUtils.hxx"
#include "NuHepMC/WriterUtils.hxx"

#include "params_all.h"

#include <utility>

namespace nuwroconv {

int GetEC1Channel(flags const &flag) {
  // primary vertex flags
  //  bool qel;        ///< (quasi) elastic       (qel == dyn/2==0)
  //  bool res;        ///< resontant             (res == dyn/2==1)
  //  bool dis;        ///< deep inelastic        (dis == dyn/2==2)
  //  bool coh;        ///< coherent              (coh == dyn/2==3)
  //  bool mec;        ///< meson exhange current (mec == dyn/2==4)
  //  bool hyp;        ///< hyperon production
  //  bool lep;        ///< neutrino-lepton

  // bool nc;         ///< neutral current       (nc == dyn%2)
  // bool cc;         ///< charged current       (cc == !nc)

  // bool anty;       ///< true if antineutrino (anty==in[0].pdg<0)

  // bool res_delta;  ///< true if RES pion comes from Delta

  // int anti = flag.anty ? -1 : 1;
  int nc = flag.nc ? 50 : 0;

  if (flag.qel) {
    return 200 + nc;
  } else if (flag.res) {
    return (flag.res_delta ? 400 : 500) + nc;
  } else if (flag.dis) {
    return 600 + nc;
  } else if (flag.coh) {
    return 100 + nc;
  } else if (flag.mec) {
    return 300 + nc;
  } else if (flag.hyp) {
    return 201 + nc;
  } else if (flag.lep) {
    return 700 + nc;
  }
  return 0;
}

void AddParamInfo(std::shared_ptr<HepMC3::GenRunInfo> gri, params const &par) {
  std::stringstream ss;
#define PARAM(type, name, default_value)                                       \
  ss.str("");                                                                  \
  write(par.name, ss);                                                         \
  NuHepMC::add_attribute(                                                      \
      gri, std::string("NuHepMC.Provenance.NuWro.") + #name, ss.str());
  PARAMS_ALL()
#undef PARAM
  NuHepMC::add_attribute(gri, "NuHepMC.Provenance.NuWro.path_to_data",
                         par.path_to_data);
}

std::shared_ptr<HepMC3::GenRunInfo>
BuildRunInfo(int nevents, double flux_averaged_total_cross_section,
             params const &par) {

  // G.R.1 Valid GenRunInfo
  auto run_info = std::make_shared<HepMC3::GenRunInfo>();

  // G.R.2 NuHepMC Version
  NuHepMC::GR2::WriteVersion(run_info);

  // G.R.3 Generator Identification
  run_info->tools().emplace_back(HepMC3::GenRunInfo::ToolInfo{
      "NuWro", NUWRO_VERSION_STR,
      "https://doi.org/10.1016/j.nuclphysbps.2012.09.136"});
  run_info->tools().emplace_back(HepMC3::GenRunInfo::ToolInfo{
      "nuwro2hepmc3", PROJECT_VERSION_STR, "github.com/NuHepMC/nuwro2hepmc3"});

  // G.R.4 Process Metadata
  // E.C.1
  NuHepMC::StatusCodeDescriptors ChannelNameIndexModeMapping = {
      {100, {"CC coh", "coherent"}},
      {150, {"NC coh", "coherent"}},

      {200, {"CC qel", "(quasi) elastic"}},
      {250, {"NC qel", "(quasi) elastic"}},

      {201, {"CC hyp", "hyperon production"}},
      {251, {"NC hyp", "hyperon production"}},

      {300, {"CC mec", "meson exhange current"}},
      {350, {"NC mec", "meson exhange current"}},

      {400, {"CC res", "delta resonant"}},
      {450, {"NC res", "delta resonant"}},

      {500, {"CC non-delta SPP", "non-delta single pion"}},
      {550, {"NC non-delta SPP", "non-delta single pion"}},

      {600, {"CC dis", "deep inelastic"}},
      {650, {"NC dis", "deep inelastic"}},

      {700, {"CC lep", "neutrino-lepton"}},
      {750, {"NC lep", "neutrino-lepton"}},
  };
  NuHepMC::GR8::WriteProcessIDDefinitions(run_info,
                                          ChannelNameIndexModeMapping);

  NuHepMC::StatusCodeDescriptors VertexStatuses = {
      {NuHepMC::VertexStatus::Primary,
       {"PrimaryVertex", "The neutrino hard-scatter vertex"}},
      {NuHepMC::VertexStatus::FSISummary,
       {"FSIVertex", "A single vertex representing the cascade"}},
      {NuHepMC::VertexStatus::NucleonSeparation,
       {"NucleonSeparationVertex",
        "Impulse approximation vertex that represents the separation of the "
        "single target nucleon from the target nucleus ground state."}},
  };

  NuHepMC::GR9::WriteVertexStatusIDDefinitions(run_info, VertexStatuses);

  NuHepMC::StatusCodeDescriptors ParticleStatuses = {
      {NuHepMC::ParticleStatus::UndecayedPhysical,
       {"UndecayedPhysicalParticle",
        "Physical final state particles produced by this simulation"}},
      {NuHepMC::ParticleStatus::DecayedPhysical,
       {"DecayedPhysical", "Particle was decayed by the simulation"}},
      {NuHepMC::ParticleStatus::DocumentationLine,
       {"DocumentationLine",
        "Documentation line, not considered a real particle"}},
      {NuHepMC::ParticleStatus::IncomingBeam,
       {"IncomingBeamParticle", "Incoming beam particle"}},
      {NuHepMC::ParticleStatus::Target,
       {"TargetParticle", "The target particle in the hard scatter"}},
      {NuHepMC::ParticleStatus::StruckNucleon,
       {"StruckNucleon", "The nucleon involved in the hard scatter"}},
  };
  NuHepMC::GR10::WriteParticleStatusIDDefinitions(run_info, ParticleStatuses);

  // G.R.7 Event Weights
  NuHepMC::GR7::SetWeightNames(run_info, {
                                             "CV",
                                         });

  // G.R.4 Signalling Followed Conventions
  NuHepMC::GR4::SetConventions(run_info, {
                                             "G.C.2",
                                             "E.C.1",
                                         });

  NuHepMC::GR6::SetCrossSectionUnits(run_info, "pb", "PerNucleon");

  // G.C.2 Flux-averaged Total Cross Section
  NuHepMC::GC2::SetFluxAveragedTotalXSec(run_info,
                                         flux_averaged_total_cross_section *
                                             NuHepMC::CrossSection::Units::cm2);

  AddParamInfo(run_info, par);
  return run_info;
}

std::shared_ptr<HepMC3::GenEvent>
ToGenEvent(event &ev, std::shared_ptr<HepMC3::GenRunInfo> gri) {

#ifdef NUWROCONV_DEBUG
  std::cout << ">>>>>>>>>>>>>>>" << std::endl;
#endif

  auto evt =
      std::make_shared<HepMC3::GenEvent>(HepMC3::Units::MEV, HepMC3::Units::CM);
  evt->set_run_info(gri);

  HepMC3::GenVertexPtr IAVertex =
      std::make_shared<HepMC3::GenVertex>(HepMC3::FourVector{});
  IAVertex->set_status(NuHepMC::VertexStatus::NucleonSeparation);

  int nuclear_PDG = 1000000000 + ev.par.nucleus_p * 10000 +
                    (ev.par.nucleus_p + ev.par.nucleus_n) * 10;

  HepMC3::GenParticlePtr target_nucleus = std::make_shared<HepMC3::GenParticle>(
      HepMC3::FourVector{0, 0, 0, 0}, nuclear_PDG,
      NuHepMC::ParticleStatus::Target);

#ifdef NUWROCONV_DEBUG
  std::cout << "        kTargetParticle: " << nuclear_PDG << std::endl;
#endif

  IAVertex->add_particle_in(target_nucleus);

  int res_nuclear_PDG = 1000000000 + ev.pr * 10000 + (ev.pr + ev.nr) * 10;

  if (ev.pr < 0) {
    res_nuclear_PDG = nuclear_PDG;
  }

  HepMC3::GenParticlePtr residual_nucleus_internal =
      std::make_shared<HepMC3::GenParticle>(
          HepMC3::FourVector{0, 0, 0, 0},
          NuHepMC::ParticleNumber::NuclearRemnant,
          NuHepMC::ParticleStatus::DocumentationLine);

  HepMC3::GenParticlePtr residual_nucleus_external =
      std::make_shared<HepMC3::GenParticle>(
          HepMC3::FourVector{0, 0, 0, 0},
          NuHepMC::ParticleNumber::NuclearRemnant,
          NuHepMC::ParticleStatus::UndecayedPhysical);

#ifdef NUWROCONV_DEBUG
  std::cout << "      kDocumentationLine: "
            << NuHepMC::ParticleNumber::NuclearRemnant << std::endl;
#endif

  IAVertex->add_particle_out(residual_nucleus_internal);

  // E.R.5
  HepMC3::GenVertexPtr primvertex =
      std::make_shared<HepMC3::GenVertex>(HepMC3::FourVector{});
  primvertex->set_status(NuHepMC::VertexStatus::Primary);

  HepMC3::GenVertexPtr fsivertex =
      std::make_shared<HepMC3::GenVertex>(HepMC3::FourVector{});
  fsivertex->set_status(NuHepMC::VertexStatus::FSISummary);
  fsivertex->add_particle_in(residual_nucleus_internal);
  fsivertex->add_particle_out(residual_nucleus_external);

  for (auto &p : ev.in) {

    if ((std::abs(p.pdg) > 10) && (std::abs(p.pdg) < 17)) { // incoming neutrino

      HepMC3::GenParticlePtr part = std::make_shared<HepMC3::GenParticle>(
          HepMC3::FourVector{p.p4().x, p.p4().y, p.p4().z, p.p4().t}, p.pdg,
          NuHepMC::ParticleStatus::IncomingBeam);
      part->set_generated_mass(p.mass());

#ifdef NUWROCONV_DEBUG
      std::cout << "  kIncomingBeamParticle: " << p.pdg << ", E: " << p.energy()
                << ", M: " << p.mass() << ", p4 = " << p.p4() << std::endl;
#endif

      primvertex->add_particle_in(part);

    } else { // incoming target nucleon
      HepMC3::GenParticlePtr part = std::make_shared<HepMC3::GenParticle>(
          HepMC3::FourVector{p.p4().x, p.p4().y, p.p4().z, p.p4().t}, p.pdg,
          NuHepMC::ParticleStatus::StruckNucleon);
      part->set_generated_mass(p.mass());

#ifdef NUWROCONV_DEBUG
      std::cout << "         kStruckNucleon: " << p.pdg << ", E: " << p.energy()
                << ", M: " << p.mass() << ", p4 = " << p.p4() << std::endl;
#endif

      primvertex->add_particle_in(part);
      IAVertex->add_particle_out(part);
    }
  }

  for (auto &p :
       ev.out) { // hard scatter final states, call them all documentation lines
                 // and push them through the FSI vertext
    HepMC3::GenParticlePtr part = std::make_shared<HepMC3::GenParticle>(
        HepMC3::FourVector{p.p4().x, p.p4().y, p.p4().z, p.p4().t}, p.pdg,
        NuHepMC::ParticleStatus::DocumentationLine);
    part->set_generated_mass(p.mass());

#ifdef NUWROCONV_DEBUG
    std::cout << "      kDocLine (PreFSI): " << p.pdg << ", E: " << p.energy()
              << ", M: " << p.mass() << ", p4 = " << p.p4() << std::endl;
#endif

    primvertex->add_particle_out(part);
    fsivertex->add_particle_in(part);
  }

  for (auto &p : ev.post) { // final state real particles
    HepMC3::GenParticlePtr part = std::make_shared<HepMC3::GenParticle>(
        HepMC3::FourVector{p.p4().x, p.p4().y, p.p4().z, p.p4().t}, p.pdg,
        NuHepMC::ParticleStatus::UndecayedPhysical);
    part->set_generated_mass(p.mass());

#ifdef NUWROCONV_DEBUG
    std::cout << "      kPhysicalParticle: " << p.pdg << ", E: " << p.energy()
              << ", M: " << p.mass() << ", p4 = " << p.p4() << std::endl;
#endif

    fsivertex->add_particle_out(part);
  }

  evt->add_vertex(IAVertex);
  evt->add_vertex(primvertex);
  evt->add_vertex(fsivertex);

  if (res_nuclear_PDG) {
    NuHepMC::PC2::SetRemnantNucleusParticleNumber(
        residual_nucleus_internal, (res_nuclear_PDG / 10000) % 1000,
        (res_nuclear_PDG / 10) % 1000);
  }

  // E.C.1
  evt->weight("CV") = 1;

  // E.R.3
  NuHepMC::ER3::SetProcessID(*evt, GetEC1Channel(ev.flag));

  // E.R.5
  NuHepMC::ER5::SetLabPosition(*evt, std::vector<double>{0, 0, 0, 0});

#ifdef NUWROCONV_DEBUG
  std::cout << "      ProcId: " << GetEC1Channel(ev.flag) << std::endl;
  std::cout << "<<<<<<<<<<<<<<" << std::endl;
#endif

  return evt;
}

} // namespace nuwroconv