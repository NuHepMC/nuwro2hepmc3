#pragma once

#include "HepMC3/GenEvent.h"

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "event1.h"
#pragma GCC diagnostic pop

namespace nuwroconv {

std::shared_ptr<HepMC3::GenRunInfo>
BuildRunInfo(int nevents, double flux_averaged_total_cross_section,
             params const &);
std::shared_ptr<HepMC3::GenEvent>
ToGenEvent(event &ev, std::shared_ptr<HepMC3::GenRunInfo> gri);

} // namespace nuwroconv