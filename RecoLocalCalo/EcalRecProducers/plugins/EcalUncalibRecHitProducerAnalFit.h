#ifndef RecoLocalCalo_EcalRecProducers_plugins_EcalUncalibRecHitProducerAnalFit_h
#define RecoLocalCalo_EcalRecProducers_plugins_EcalUncalibRecHitProducerAnalFit_h

/*
 *  Produce ECAL uncalibrated rechits from dataframes with the analytical fit
 * method.
 *
 *  \author Shahram Rahatlou, University of Rome & INFN, Sept 2005
 *	\author Dmitrijus Bugelskis, October 2013
 */

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/EcalRecHit/interface/EcalUncalibratedRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

#include "CondFormats/EcalObjects/interface/EcalMGPAGainRatio.h"
#include "CondFormats/EcalObjects/interface/EcalGainRatios.h"
#include "CondFormats/EcalObjects/interface/EcalPedestals.h"

#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitRecAnalFitAlgo.h"
#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerBase.h"

class EcalUncalibRecHitProducerAnalFit : public EcalUncalibRecHitProducerBase {

 public:
  EcalUncalibRecHitProducerAnalFit(const edm::ParameterSet& ps);
  virtual ~EcalUncalibRecHitProducerAnalFit();

  virtual void set(const edm::EventSetup& es);
  virtual bool run(const edm::Event& evt,
                   const EcalDigiCollection::const_iterator& digi,
                   EcalUncalibratedRecHitCollection& result);

 private:
  EcalUncalibRecHitRecAnalFitAlgo<EBDataFrame> algoEB_;
  EcalUncalibRecHitRecAnalFitAlgo<EEDataFrame> algoEE_;

  edm::ESHandle<EcalGainRatios> pRatio;
  edm::ESHandle<EcalPedestals> pedHandle;
};

#endif
