#pragma once

#include "HepMC3/GenEvent.h"

#include "event1.h"

namespace NuWroNuHepMC {

namespace VertexStatus {
// NuHepMC standard vertex codes
const int kPrimaryVertex = 1;
const int kNuclearTargetVertex = 2;

// NuWro Extended codes
const int kFSIVertex = 3;
} // namespace VertexStatus

namespace ParticleStatus {
// HepMC3 standard vertex codes
const int kUndecayedPhysicalParticle = 1;
const int kDecayedParticle = 2;
const int kDocumentationLine = 3;
const int kIncomingBeamParticle = 4;

// NuHepMC standard vertex codes
const int kTargetParticle = 11;

// P.C.1
const int kStruckNucleon = 21;

} // namespace ParticleStatus

} // namespace NuWroNuHepMC

std::shared_ptr<HepMC3::GenRunInfo>
BuildRunInfo(int nevents, double flux_averaged_total_cross_section, params const &);
HepMC3::GenEvent ToGenEvent(event &ev, std::shared_ptr<HepMC3::GenRunInfo> gri);