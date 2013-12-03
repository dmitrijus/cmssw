#ifndef RecoLocalCalo_EcalRecProducers_plugins_EcalUncalibRecHitProducerMaxSample_h
#define RecoLocalCalo_EcalRecProducers_plugins_EcalUncalibRecHitProducerMaxSample_h

/**
 *  Produce ECAL uncalibrated rechits from dataframes.
 *
 *  \author G. Franzoni, E. Di Marco
 *
 */

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/EcalRecHit/interface/EcalUncalibratedRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerBase.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitMaxSampleAlgo.h"

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

#endif
