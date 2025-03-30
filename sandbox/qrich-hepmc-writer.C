//
// root -l 'qrich-hepmc-writer.C("out.hepmc", 100)'
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

/** Generate single muon event with fixed three momentum **/
void qrich_hepmc_writer(const char* out_fname, int n_events)
{
  auto *DatabasePDG = new TDatabasePDG();
  //int pdg = 211;
  //auto *particle = DatabasePDG->GetParticle(pdg);
  auto *pion = DatabasePDG->GetParticle(211), *kaon = DatabasePDG->GetParticle(321);

  WriterAscii hepmc_output(out_fname);
  int events_parsed = 0;
  GenEvent evt(Units::GEV, Units::MM);

  //std::random_device rd;
  unsigned int seed = 0x12345678;//(unsigned int)abs(rd());
  std::cout << "init seed for random generator is " << seed << std::endl;
  // Random number generator
  TRandom *rdmn_gen = new TRandom(seed);

  for (events_parsed = 0; events_parsed < n_events; events_parsed++) {
    //FourVector(px,py,pz,e,pdgid,status)
    // type 4 is beam
    // pdgid 2212 - proton
    //GenParticlePtr p1 =
    //  std::make_shared<GenParticle>(FourVector(0.0, 0.0, 12.0, 12.0), 11, 4);
    //GenParticlePtr p2 = std::make_shared<GenParticle>(
    //  FourVector(0.0, 0.0, 100.0, 100.004), 2212, 4); 

    GenVertexPtr v1 = std::make_shared<GenVertex>();//FourVector(0,0,30,0));
    //v1->add_particle_in(p1);
    //v1->add_particle_in(p2);

    // type 1 is final state; 211: pion; FIXME: give a proper mass;
    for(int iq=0; iq</*2*/1; iq++){
      auto particle = pion;//iq ? pion : kaon;
      Double_t eta   = 2.40;//rdmn_gen->Uniform(1.30, 3.70);
      //Double_t eta   = rdmn_gen->Uniform(1.5, 1.6);//2.9, 3.0);//2.0, 2.1);
      Double_t th    = 2*std::atan(exp(-eta));
      //Double_t th    = rdmn_gen->Uniform(3.0, 3.1)*M_PI/180;
      Double_t p     = rdmn_gen->Uniform(6.999, 7.001);// + (iq ? 1.0 : 0.0);//30.0, 30.0001);
      //Double_t phi   = 0;//M_PI/2;//rdmn_gen->Uniform(0.0, 2*M_PI);
      Double_t phi   = 0;//rdmn_gen->Uniform(0.0, 2*M_PI);
      //Double_t phi   = rdmn_gen->Uniform(-5.0+120, 5.0+120)*M_PI/180;

      Double_t px    = p * std::cos(phi) * std::sin(th);
      Double_t py    = p * std::sin(phi) * std::sin(th);
      Double_t pz    = p * std::cos(th);

      //cout<<"px,py,pz: "<<px<<" "<<py<<" "<<pz<<endl;
      
      GenParticlePtr pq = std::make_shared<GenParticle>(FourVector(
								   px, py, pz,
								   sqrt(p*p + pow(particle->Mass(), 2))),
							//pdg, 1);
							particle->PdgCode(), 1);
      v1->add_particle_out(pq);
    }//iq	
    evt.add_vertex(v1);

    if (events_parsed == 0) {
      std::cout << "First event: " << std::endl;
      Print::listing(evt);
    }

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
