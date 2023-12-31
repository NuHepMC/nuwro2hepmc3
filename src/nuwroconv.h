#pragma once

#include "HepMC3/GenEvent.h"

#include "event1.h"

std::shared_ptr<HepMC3::GenRunInfo>
BuildRunInfo(int nevents, double flux_averaged_total_cross_section, params const &);
HepMC3::GenEvent ToGenEvent(event &ev, std::shared_ptr<HepMC3::GenRunInfo> gri);