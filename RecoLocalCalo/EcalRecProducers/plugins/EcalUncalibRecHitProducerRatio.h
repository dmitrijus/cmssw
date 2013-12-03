#ifndef RecoLocalCalo_EcalRecProducers_plugins_EcalUncalibRecHitProducerRatio_h
#define RecoLocalCalo_EcalRecProducers_plugins_EcalUncalibRecHitProducerRatio_h

/**
  *  Template used to compute amplitude, pedestal, time jitter, chi2 of a pulse
  *  using a weights method.
  *
  *  \author A. Ledovskoy (Design) - M. Balazs (Implementation)
  */

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "CondFormats/EcalObjects/interface/EcalSampleMask.h"
#include "CondFormats/EcalObjects/interface/EcalPedestals.h"
#include "CondFormats/EcalObjects/interface/EcalGainRatios.h"

#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/EcalRecHit/interface/EcalUncalibratedRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerBase.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitRatioMethodAlgo.h"

class EcalUncalibRecHitProducerRatio : public EcalUncalibRecHitProducerBase {

 public:
  EcalUncalibRecHitProducerRatio(const edm::ParameterSet&);
  virtual ~EcalUncalibRecHitProducerRatio();

  virtual void set(const edm::EventSetup& es);
  virtual bool run(const edm::Event& evt,
                   const EcalDigiCollection::const_iterator& digi,
                   EcalUncalibratedRecHitCollection& result);

 protected:

  // determie which of the samples must actually be used by ECAL local reco
  edm::ESHandle<EcalSampleMask> sampleMaskHand_;

  edm::ESHandle<EcalPedestals> peds;
  edm::ESHandle<EcalGainRatios> gains;

  double pedVec[3];
  double pedRMSVec[3];
  double gainRatios[3];

  std::vector<double> EBtimeFitParameters_;
  std::vector<double> EEtimeFitParameters_;

  std::vector<double> EBamplitudeFitParameters_;
  std::vector<double> EEamplitudeFitParameters_;

  std::pair<double, double> EBtimeFitLimits_;
  std::pair<double, double> EEtimeFitLimits_;

  double EBtimeConstantTerm_;
  double EEtimeConstantTerm_;

  EcalUncalibRecHitRatioMethodAlgo<EBDataFrame> uncalibMaker_barrel_;
  EcalUncalibRecHitRatioMethodAlgo<EEDataFrame> uncalibMaker_endcap_;
};

#endif
