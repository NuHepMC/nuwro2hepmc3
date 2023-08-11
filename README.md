# nuwro2hepmc3
Tools for converting [NuWro](https://github.com/NuWro/nuwro) event vector ROOT files to [HepMC3](https://gitlab.cern.ch/hepmc/HepMC3).

## build requirements

ROOT 6+
C++17 Compiler

## build like

```bash
cd /path/to/nuwro2hepmc3
mkdir build; cd build
cmake ..
make install
```

This will pull down version 21.09.02 of NuWro to build the event vector reading libraries from. Does not require an install of NuWro to exist.

## Usage

```
$ nuwro2hepmc3 -?
[USAGE]: nuwro2hepmc3
  -i <events.root>         : neutvect file to read
  -N <NMax>                : Process at most <NMax> events
  -o <nuwro.hepmc3>        : hepmc3 file to write
  -z                       : Write to .gz compress ASCII file
```