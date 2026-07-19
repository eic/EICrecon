//
// root -l 'hepmc-writer-single-track.C("out.hepmc", 1000, pdg, pmin, pmax, etamin, etamax, phimin, phimax)'
//

#include "HepMC3/GenEvent.h"
#include "HepMC3/ReaderAscii.h"
#include "HepMC3/WriterAscii.h"
#include "HepMC3/Print.h"

#include <iostream>
#include <random>
#include <cmath>
#include <math.h>
#include <TMath.h>

using namespace HepMC3;

void hepmc_writer_drich_and_pfrich(const char* out_fname, int n_events) //, int pdg,
//double pmin, double pmax, double etamin, double etamax, double phimin, double phimax)
{
  auto* DatabasePDG = new TDatabasePDG();
  auto* particle    = DatabasePDG->GetParticle(211); //pdg);

  WriterAscii hepmc_output(out_fname);
  int events_parsed = 0;
  GenEvent evt(Units::GEV, Units::MM);

  unsigned int seed = 0x12345678;
  // Random number generator
  TRandom* rdmn_gen = new TRandom(seed);

  for (events_parsed = 0; events_parsed < n_events; events_parsed++) {
    GenVertexPtr v1 = std::make_shared<GenVertex>();

    // type 1 is final state;
    {
      Double_t p   = 10.0;
      double eta   = 2.0;
      Double_t th  = 2 * std::atan(exp(-eta));
      Double_t phi = M_PI / 6;

      Double_t px = p * std::cos(phi) * std::sin(th);
      Double_t py = p * std::sin(phi) * std::sin(th);
      Double_t pz = p * std::cos(th);

      GenParticlePtr pq = std::make_shared<GenParticle>(
          FourVector(px, py, pz, sqrt(p * p + pow(particle->Mass(), 2))), particle->PdgCode(), 1);
      v1->add_particle_out(pq);
    }
    {
      Double_t p   = 7.0;
      double eta   = -2.5;
      Double_t th  = 2 * std::atan(exp(-eta));
      Double_t phi = M_PI / 2;

      Double_t px = p * std::cos(phi) * std::sin(th);
      Double_t py = p * std::sin(phi) * std::sin(th);
      Double_t pz = p * std::cos(th);

      GenParticlePtr pq = std::make_shared<GenParticle>(
          FourVector(px, py, pz, sqrt(p * p + pow(particle->Mass(), 2))), particle->PdgCode(), 1);
      v1->add_particle_out(pq);
    }

    evt.add_vertex(v1);

    if (events_parsed == 0) {
      std::cout << "First event: " << std::endl;
      Print::listing(evt);
    } //if

    evt.set_event_number(events_parsed);

    hepmc_output.write_event(evt);
    if (events_parsed % 10000 == 0) {
      std::cout << "Event: " << events_parsed << std::endl;
    }
    evt.clear();
  }
  hepmc_output.close();
  std::cout << "Events parsed and written: " << events_parsed << std::endl;
  exit(0);
}
