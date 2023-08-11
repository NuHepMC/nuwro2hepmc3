#include "nuwroconv.h"

#include "HepMC3/GenParticle.h"
#include "HepMC3/GenVertex.h"

#include <utility>

using namespace NuWroNuHepMC;

// Lazy way of choosing the right attribute type via TMP
template <typename T> struct attr_traits {};

template <> struct attr_traits<int> {
  typedef HepMC3::IntAttribute type;
};

template <> struct attr_traits<std::vector<int>> {
  typedef HepMC3::VectorIntAttribute type;
};

template <> struct attr_traits<double> {
  typedef HepMC3::DoubleAttribute type;
};

template <> struct attr_traits<std::vector<double>> {
  typedef HepMC3::VectorDoubleAttribute type;
};

template <> struct attr_traits<std::string> {
  typedef HepMC3::StringAttribute type;
};

template <size_t N> struct attr_traits<char[N]> {
  typedef HepMC3::StringAttribute type;
};

template <> struct attr_traits<std::vector<std::string>> {
  typedef HepMC3::VectorStringAttribute type;
};

template <typename T>
void add_attribute(HepMC3::GenEvent &ge, std::string const &name,
                   T const &val) {
  ge.add_attribute(name, std::make_shared<typename attr_traits<T>::type>(val));
}

template <typename T>
void add_attribute(std::shared_ptr<HepMC3::GenRunInfo> gri,
                   std::string const &name, T const &val) {
  gri->add_attribute(name,
                     std::make_shared<typename attr_traits<T>::type>(val));
}

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

  int anti = flag.anty ? -1 : 1;
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

std::shared_ptr<HepMC3::GenRunInfo>
BuildRunInfo(int nevents, double flux_averaged_total_cross_section,
             params const &par) {

  // G.R.1 Valid GenRunInfo
  auto run_info = std::make_shared<HepMC3::GenRunInfo>();

  // G.R.2 NuHepMC Version
  add_attribute(run_info, "NuHepMC.Version.Major", 0);
  add_attribute(run_info, "NuHepMC.Version.Minor", 1);
  add_attribute(run_info, "NuHepMC.Version.Patch", 0);

  // G.R.3 Generator Identification
  run_info->tools().emplace_back(HepMC3::GenRunInfo::ToolInfo{
      "NuWro", NUWRO_VERSION_STR,
      "https://doi.org/10.1016/j.nuclphysbps.2012.09.136"});
  run_info->tools().emplace_back(HepMC3::GenRunInfo::ToolInfo{
      "nuwro2hepmc3", PROJECT_VERSION_STR, "github.com/NuHepMC/nuwro2hepmc3"});

  // G.R.4 Process Metadata
  // E.C.1
  std::vector<std::tuple<std::string, std::string, int>>
      ChannelNameIndexModeMapping = {
          {"CC coh", "coherent", 100},
          {"NC coh", "coherent", 150},

          {"CC qel", "(quasi) elastic", 200},
          {"NC qel", "(quasi) elastic", 250},

          {"CC hyp", "hyperon production", 201},
          {"NC hyp", "hyperon production", 251},

          {"CC mec", "meson exhange current", 300},
          {"NC mec", "meson exhange current", 350},

          {"CC res", "delta resonant", 400},
          {"NC res", "delta resonant", 450},

          {"CC non-delta SPP", "non-delta single pion", 500},
          {"NC non-delta SPP", "non-delta single pion", 550},

          {"CC dis", "deep inelastic", 600},
          {"NC dis", "deep inelastic", 650},

          {"CC lep", "neutrino-lepton", 700},
          {"NC lep", "neutrino-lepton", 750},
      };
  std::vector<int> pids;
  for (auto const &pid : ChannelNameIndexModeMapping) {
    pids.push_back(std::get<2>(pid));
    add_attribute(run_info,
                  "NuHepMC.ProcessInfo[" + std::to_string(std::get<2>(pid)) +
                      "].Name",
                  std::get<0>(pid));
    add_attribute(run_info,
                  "NuHepMC.ProcessInfo[" + std::to_string(std::get<2>(pid)) +
                      "].Description",
                  std::get<1>(pid));
  }

  add_attribute(run_info, "NuHepMC.ProcessIDs", pids);

  // G.R.5 Vertex Status Metadata
  add_attribute(run_info,
                "NuHepMC.VertexStatusInfo[" +
                    std::to_string(VertexStatus::kPrimaryVertex) + "].Name",
                "PrimaryVertex");
  add_attribute(run_info,
                "NuHepMC.VertexStatusInfo[" +
                    std::to_string(VertexStatus::kPrimaryVertex) +
                    "].Description",
                "The neutrino hard-scatter vertex");
  add_attribute(run_info,
                "NuHepMC.VertexStatusInfo[" +
                    std::to_string(VertexStatus::kFSIVertex) + "].Name",
                "FSIVertex");
  add_attribute(run_info,
                "NuHepMC.VertexStatusInfo[" +
                    std::to_string(VertexStatus::kFSIVertex) + "].Description",
                "A single vertex representing the cascade");

  add_attribute(
      run_info, "NuHepMC.VertexStatusIDs",
      std::vector<int>{VertexStatus::kPrimaryVertex, VertexStatus::kFSIVertex});

  // G.R.6 Particle Status Metadata
  add_attribute(run_info,
                "NuHepMC.ParticleStatusInfo[" +
                    std::to_string(ParticleStatus::kUndecayedPhysicalParticle) +
                    "].Name",
                "UndecayedPhysicalParticle");
  add_attribute(run_info,
                "NuHepMC.ParticleStatusInfo[" +
                    std::to_string(ParticleStatus::kUndecayedPhysicalParticle) +
                    "].Description",
                "Physical final state particles produced by this simulation");

  add_attribute(run_info,
                "NuHepMC.ParticleStatusInfo[" +
                    std::to_string(ParticleStatus::kDecayedParticle) + "].Name",
                "DecayedParticle");
  add_attribute(run_info,
                "NuHepMC.ParticleStatusInfo[" +
                    std::to_string(ParticleStatus::kDecayedParticle) +
                    "].Description",
                "Particle was decayed by the simulation");
  add_attribute(run_info,
                "NuHepMC.ParticleStatusInfo[" +
                    std::to_string(ParticleStatus::kDocumentationLine) +
                    "].Name",
                "DocumentationLine");
  add_attribute(run_info,
                "NuHepMC.ParticleStatusInfo[" +
                    std::to_string(ParticleStatus::kDocumentationLine) +
                    "].Description",
                "Documentation line, not considered a real particle");
  add_attribute(run_info,
                "NuHepMC.ParticleStatusInfo[" +
                    std::to_string(ParticleStatus::kIncomingBeamParticle) +
                    "].Name",
                "IncomingBeamParticle");
  add_attribute(run_info,
                "NuHepMC.ParticleStatusInfo[" +
                    std::to_string(ParticleStatus::kIncomingBeamParticle) +
                    "].Description",
                "Incoming beam particle");
  add_attribute(run_info,
                "NuHepMC.ParticleStatusInfo[" +
                    std::to_string(ParticleStatus::kTargetParticle) + "].Name",
                "TargetParticle");
  add_attribute(run_info,
                "NuHepMC.ParticleStatusInfo[" +
                    std::to_string(ParticleStatus::kTargetParticle) +
                    "].Description",
                "The target particle in the hard scatter");

  add_attribute(run_info,
                "NuHepMC.ParticleStatusInfo[" +
                    std::to_string(ParticleStatus::kStruckNucleon) + "].Name",
                "StruckNucleon");
  add_attribute(run_info,
                "NuHepMC.ParticleStatusInfo[" +
                    std::to_string(ParticleStatus::kStruckNucleon) +
                    "].Description",
                "The nucleon involved in the hard scatter");

  add_attribute(run_info, "NuHepMC.ParticleStatusIDs",
                std::vector<int>{
                    ParticleStatus::kUndecayedPhysicalParticle,
                    ParticleStatus::kDecayedParticle,
                    ParticleStatus::kDocumentationLine,
                    ParticleStatus::kIncomingBeamParticle,
                    ParticleStatus::kTargetParticle,
                    ParticleStatus::kStruckNucleon,
                });

  // G.R.7 Event Weights
  run_info->set_weight_names({
      "CV",
  });

  // G.C.1 Signalling Followed Conventions
  add_attribute(run_info, "NuHepMC.Conventions",
                std::vector<std::string>{
                    "G.C.1",
                    "G.C.2",
                    "G.C.4"
                    "E.C.1",
                });

  // G.C.2 File Exposure (Standalone)
  add_attribute(run_info, "NuHepMC.Exposure.NEvents", nevents);

  // G.C.4 Flux-averaged Total Cross Section
  add_attribute(run_info, "NuHepMC.FluxAveragedTotalCrossSection",
                flux_averaged_total_cross_section);

  return run_info;
}

HepMC3::GenEvent ToGenEvent(event &ev,
                            std::shared_ptr<HepMC3::GenRunInfo> gri) {

#ifdef NUWROCONV_DEBUG
  std::cout << ">>>>>>>>>>>>>>>" << std::endl;
#endif

  HepMC3::GenEvent evt(HepMC3::Units::GEV, HepMC3::Units::CM);
  evt.set_run_info(gri);

  HepMC3::GenVertexPtr IAVertex =
      std::make_shared<HepMC3::GenVertex>(HepMC3::FourVector{});
  IAVertex->set_status(VertexStatus::kNuclearTargetVertex);

  int nuclear_PDG = 1000000000 + ev.par.nucleus_p * 10000 +
                    (ev.par.nucleus_p + ev.par.nucleus_n) * 10;

  HepMC3::GenParticlePtr target_nucleus = std::make_shared<HepMC3::GenParticle>(
      HepMC3::FourVector{0, 0, 0, 0}, nuclear_PDG,
      ParticleStatus::kTargetParticle);

#ifdef NUWROCONV_DEBUG
  std::cout << "        kTargetParticle: " << nuclear_PDG << std::endl;
#endif

  IAVertex->add_particle_in(target_nucleus);

  int res_nuclear_PDG = 1000000000 + ev.pr * 10000 + (ev.pr + ev.nr) * 10;

  HepMC3::GenParticlePtr residual_nucleus =
      std::make_shared<HepMC3::GenParticle>(
          HepMC3::FourVector{0, 0, 0, 0}, res_nuclear_PDG,
          ParticleStatus::kUndecayedPhysicalParticle);

#ifdef NUWROCONV_DEBUG
  std::cout << "      kPhysicalParticle: " << res_nuclear_PDG << std::endl;
#endif

  IAVertex->add_particle_out(residual_nucleus);

  // E.R.5
  HepMC3::GenVertexPtr primvertex =
      std::make_shared<HepMC3::GenVertex>(HepMC3::FourVector{});
  primvertex->set_status(VertexStatus::kPrimaryVertex);

  HepMC3::GenVertexPtr fsivertex =
      std::make_shared<HepMC3::GenVertex>(HepMC3::FourVector{});
  fsivertex->set_status(VertexStatus::kFSIVertex);

  for (auto &p : ev.in) {

    if ((std::abs(p.pdg) > 10) && (std::abs(p.pdg) < 17)) { // incoming neutrino

      HepMC3::GenParticlePtr part = std::make_shared<HepMC3::GenParticle>(
          HepMC3::FourVector{p.p4().x, p.p4().y, p.p4().z, p.p4().t}, p.pdg,
          ParticleStatus::kIncomingBeamParticle);
      part->set_generated_mass(p.mass());

#ifdef NUWROCONV_DEBUG
      std::cout << "  kIncomingBeamParticle: " << p.pdg << ", E: " << p.energy()
                << ", M: " << p.mass() << ", p4 = " << p.p4() << std::endl;
#endif

      primvertex->add_particle_in(part);

    } else { // incoming target nucleon
      HepMC3::GenParticlePtr part = std::make_shared<HepMC3::GenParticle>(
          HepMC3::FourVector{p.p4().x, p.p4().y, p.p4().z, p.p4().t}, p.pdg,
          ParticleStatus::kStruckNucleon);
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
        ParticleStatus::kDocumentationLine);
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
        ParticleStatus::kUndecayedPhysicalParticle);
    part->set_generated_mass(p.mass());

#ifdef NUWROCONV_DEBUG
    std::cout << "      kPhysicalParticle: " << p.pdg << ", E: " << p.energy()
              << ", M: " << p.mass() << ", p4 = " << p.p4() << std::endl;
#endif

    fsivertex->add_particle_out(part);
  }

  evt.add_vertex(IAVertex);
  evt.add_vertex(primvertex);
  evt.add_vertex(fsivertex);

  // E.C.1
  evt.weight("CV") = 1;

  // E.R.4
  add_attribute(evt, "LabPos", std::vector<double>{0, 0, 0, 0});

  add_attribute(evt, "ProcId", GetEC1Channel(ev.flag));

#ifdef NUWROCONV_DEBUG
  std::cout << "      ProcId: " << GetEC1Channel(ev.flag) << std::endl;
  std::cout << "<<<<<<<<<<<<<<" << std::endl;
#endif

  return evt;
}