#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerAnalFit.h"

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "CondFormats/EcalObjects/interface/EcalGainRatios.h"
#include "CondFormats/EcalObjects/interface/EcalPedestals.h"

#include "CondFormats/DataRecord/interface/EcalGainRatiosRcd.h"
#include "CondFormats/DataRecord/interface/EcalPedestalsRcd.h"

#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/EcalRecHit/interface/EcalUncalibratedRecHit.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/EcalDigi/interface/EBDataFrame.h"
#include "DataFormats/EcalDigi/interface/EEDataFrame.h"
#include "DataFormats/EcalDigi/interface/EcalMGPASample.h"
#include "DataFormats/Common/interface/Handle.h"

#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitRecAnalFitAlgo.h"
#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerBase.h"

#include <iostream>
#include <cmath>
#include <vector>

EcalUncalibRecHitProducerAnalFit::EcalUncalibRecHitProducerAnalFit(const edm::ParameterSet& ps)
    : EcalUncalibRecHitProducerBase(ps) {}

EcalUncalibRecHitProducerAnalFit::~EcalUncalibRecHitProducerAnalFit() {}

void EcalUncalibRecHitProducerAnalFit::set(const edm::EventSetup& es) {
  // Gain Ratios
  LogDebug("EcalUncalibRecHitDebug") << "fetching gainRatios....";
  es.get<EcalGainRatiosRcd>().get(pRatio);
  LogDebug("EcalUncalibRecHitDebug") << "done.";

  // fetch the pedestals from the cond DB via EventSetup
  LogDebug("EcalUncalibRecHitDebug") << "fetching pedestals....";
  es.get<EcalPedestalsRcd>().get(pedHandle);
  LogDebug("EcalUncalibRecHitDebug") << "done.";
}

bool EcalUncalibRecHitProducerAnalFit::run(
    const edm::Event& evt, const EcalDigiCollection::const_iterator& itdg,
    EcalUncalibratedRecHitCollection& result) {

  using namespace edm;

  const EcalGainRatioMap& gainMap =
      pRatio.product()->getMap();     // map of gain ratios
  const EcalPedestalsMap& pedMap =
      pedHandle.product()->getMap();  // map of pedestals

  EcalPedestalsMapIterator pedIter;  // pedestal iterator
  EcalPedestals::Item aped;          // pedestal object for a single xtal

  EcalGainRatioMap::const_iterator gainIter;  // gain iterator
  EcalMGPAGainRatio aGain;                    // gain object for a single xtal

  DetId detid(itdg->id());

  // find pedestals for this channel
  //LogDebug("EcalUncalibRecHitDebug") << "looking up pedestal for crystal: " <<
  //itdg->id(); // FIXME
  pedIter = pedMap.find(detid);
  if (pedIter != pedMap.end()) {
    aped = (*pedIter);
  } else {
    edm::LogError("EcalUncalibRecHitProducerAnalFit")
        << "error!! could not find pedestals for channel: ";
    if (detid.subdetId() == EcalBarrel) {
      edm::LogError("EcalUncalibRecHitProducerAnalFit") << EBDetId(detid);
    } else {
      edm::LogError("EcalUncalibRecHitProducerAnalFit") << EEDetId(detid);
    }
    edm::LogError("EcalUncalibRecHitProducerAnalFit")
        << "\n  no uncalib rechit will be made for this digi!";
    return false;
  }
  double pedVec[3];
  pedVec[0] = aped.mean_x12;
  pedVec[1] = aped.mean_x6;
  pedVec[2] = aped.mean_x1;

  // find gain ratios
  //LogDebug("EcalUncalibRecHitDebug") << "looking up gainRatios for crystal: "
  //<< itdg->id(); // FIXME
  gainIter = gainMap.find(detid);
  if (gainIter != gainMap.end()) {
    aGain = (*gainIter);
  } else {
    edm::LogError("EcalUncalibRecHitProducerAnalFit")
        << "error!! could not find gain ratios for channel: ";
    if (detid.subdetId() == EcalBarrel) {
      edm::LogError("EcalUncalibRecHitProducerAnalFit") << EBDetId(detid);
    } else {
      edm::LogError("EcalUncalibRecHitProducerAnalFit") << EEDetId(detid);
    }
    edm::LogError("EcalUncalibRecHitProducerAnalFit")
        << "\n  no uncalib rechit will be made for this digi!";
    return false;
  }
  double gainRatios[3];
  gainRatios[0] = 1.;
  gainRatios[1] = aGain.gain12Over6();
  gainRatios[2] = aGain.gain6Over1() * aGain.gain12Over6();

  if (detid.subdetId() == EcalBarrel) {
    EcalUncalibratedRecHit aHit =
        algoEB_.makeRecHit(*itdg, pedVec, gainRatios, 0, 0);
    result.push_back(aHit);
    if (aHit.amplitude() > 0.) {
      LogDebug("EcalUncalibRecHitInfo")
          << "EcalUncalibRecHitProducerAnalFit: processed EBDataFrame with id: "
          << EBDetId(detid) << "\n"
          << "uncalib rechit amplitude: " << aHit.amplitude();
    }
  } else {
    EcalUncalibratedRecHit aHit =
        algoEE_.makeRecHit(*itdg, pedVec, gainRatios, 0, 0);
    result.push_back(aHit);
    if (aHit.amplitude() > 0.) {
      LogDebug("EcalUncalibRecHitInfo")
          << "EcalUncalibRecHitProducerAnalFit: processed EEDataFrame with id: "
          << EEDetId(detid) << "\n"
          << "uncalib rechit amplitude: " << aHit.amplitude();
    }
  }
  return true;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(EcalUncalibRecHitProducerAnalFit);
