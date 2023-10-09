#include "TChain.h"
#include "TFile.h"
#include "TH1D.h"

#include "nuwroconv.h"

#include "NuHepMC/make_writer.hxx"

#include <iostream>

std::vector<std::string> files_to_read;
std::string file_to_write;

Long64_t nmaxevents = std::numeric_limits<Long64_t>::max();

void SayUsage(char const *argv[]) {
  std::cout
      << "[USAGE]: " << argv[0] << "\n"
      << "\t-i <events.root> [nv2.root ...]  : nuwro event file to read\n"
      << "\t-N <NMax>                        : Process at most <NMax> events\n"
      << "\t-o <nuwro.hepmc3>                : hepmc3 file to write\n"
      << std::endl;
}

void handleOpts(int argc, char const *argv[]) {
  int opt = 1;
  while (opt < argc) {
    if (std::string(argv[opt]) == "-?" || std::string(argv[opt]) == "--help") {
      SayUsage(argv);
      exit(0);
    } else if ((opt + 1) < argc) {
      if (std::string(argv[opt]) == "-i") {
        while (((opt + 1) < argc) && (argv[opt + 1][0] != '-')) {
          files_to_read.push_back(argv[++opt]);
          std::cout << "[INFO]: Reading from " << files_to_read.back()
                    << std::endl;
        }
      } else if (std::string(argv[opt]) == "-N") {
        nmaxevents = std::stol(argv[++opt]);
        std::cout << "[INFO]: Processing at most " << nmaxevents << " events."
                  << std::endl;
      } else if (std::string(argv[opt]) == "-o") {
        file_to_write = argv[++opt];
      }
    } else {
      std::cout << "[ERROR]: Unknown option: " << argv[opt] << std::endl;
      SayUsage(argv);
      exit(1);
    }
    opt++;
  }
}

int main(int argc, char const *argv[]) {

  handleOpts(argc, argv);

  if (!files_to_read.size() || !file_to_write.length()) {
    std::cout << "[ERROR]: Expected -i and -o arguments." << std::endl;
    return 1;
  }

  TChain chin("treeout");

  for (auto const &ftr : files_to_read) {
    if (!chin.Add(ftr.c_str(), 0)) {
      std::cout << "[ERROR]: Failed to find tree: \"neuttree\" in file: \""
                << ftr << "\"." << std::endl;
      return 1;
    }
  }

  Long64_t ents = chin.GetEntries();
  // need to do this before opening the other file or... kablamo
  chin.GetEntry(0);

  event *ev = nullptr;
  auto branch_status = chin.SetBranchAddress("e", &ev);

  chin.GetEntry(0);

  Long64_t ents_to_run = std::min(ents, nmaxevents);
  Long64_t shout_every = std::min(10000LL, ents_to_run / 10);

  // Trust that the first entry has the right weight.
  double fatx = ev->weight;

  auto gri = BuildRunInfo(ents_to_run, fatx, ev->par);
  std::unique_ptr<HepMC3::Writer> output(
      NuHepMC::Writer::make_writer(file_to_write, gri));

  if (output->failed()) {
    return 2;
  }

  for (Long64_t i = 0; i < ents_to_run; ++i) {
    chin.GetEntry(i);
    if (i && shout_every && !(i % shout_every)) {
      std::cout << "\rConverting " << i << "/" << ents_to_run << std::flush;
    }
    auto hepev = ToGenEvent(*ev, gri);
    output->write_event(hepev);
  }
  std::cout << "\rConverting " << ents_to_run << "/" << ents_to_run
            << std::endl;

  output->close();
}