#include <cmath>

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/EcalRecHit/interface/EcalUncalibratedRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerBase.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitMaxSampleAlgo.h"

#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerMaxSample.h"

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
