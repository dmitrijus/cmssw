/**
 *  Produce ECAL uncalibrated rechits from dataframes.
 *
 *  \author G. Franzoni, E. Di Marco
 *
 */

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/EcalRecHit/interface/EcalUncalibratedRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerBase.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitMaxSampleAlgo.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

class EcalUncalibRecHitProducerMaxSample
    : public EcalUncalibRecHitProducerBase {

 public:
  EcalUncalibRecHitProducerMaxSample(const edm::ParameterSet &ps);
  virtual ~EcalUncalibRecHitProducerMaxSample() {}
  ;

  virtual void set(const edm::EventSetup &es);
  virtual bool run(const edm::Event &evt,
                   const EcalDigiCollection::const_iterator &digi,
                   EcalUncalibratedRecHitCollection &result);

 private:
  EcalUncalibRecHitMaxSampleAlgo<EBDataFrame> ebAlgo_;
  EcalUncalibRecHitMaxSampleAlgo<EEDataFrame> eeAlgo_;
};

EcalUncalibRecHitProducerMaxSample::EcalUncalibRecHitProducerMaxSample(const edm::ParameterSet &ps)
    : EcalUncalibRecHitProducerBase(ps) {}

void EcalUncalibRecHitProducerMaxSample::set(const edm::EventSetup &es) {}

bool EcalUncalibRecHitProducerMaxSample::run(
    const edm::Event &evt, const EcalDigiCollection::const_iterator &itdg,
    EcalUncalibratedRecHitCollection &result) {

  DetId detid(itdg->id());

  if (detid.subdetId() == EcalBarrel) {
    result.push_back(ebAlgo_.makeRecHit(*itdg, 0, 0, 0, 0));
  } else {
    result.push_back(eeAlgo_.makeRecHit(*itdg, 0, 0, 0, 0));
  }

  return true;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(EcalUncalibRecHitProducerMaxSample);
