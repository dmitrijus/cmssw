#ifndef RecoLocalCalo_EcalRecProducers_plugins_EcalUncalibRecHitWorkerWeights_h
#define RecoLocalCalo_EcalRecProducers_plugins_EcalUncalibRecHitWorkerWeights_h

/**
  *  Template used to compute amplitude, pedestal, time jitter, chi2 of a pulse
  *  using a weights method.
  *
  *  \author R. Bruneliere - A. Zabi
  */

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "CondFormats/EcalObjects/interface/EcalPedestals.h"
#include "CondFormats/EcalObjects/interface/EcalGainRatios.h"
#include "CondFormats/EcalObjects/interface/EcalWeightXtalGroups.h"
#include "CondFormats/EcalObjects/interface/EcalTBWeights.h"

#include "SimCalorimetry/EcalSimAlgos/interface/EBShape.h"
#include "SimCalorimetry/EcalSimAlgos/interface/EEShape.h"

#include "RecoLocalCalo/EcalRecProducers/interface/UnpackerWorkerCompat.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitRecWeightsAlgo.h"

class EcalUncalibRecHitWorkerWeights : public EcalUncalibRecHitWorkerBase {
 public:
  EcalUncalibRecHitWorkerWeights(const edm::ParameterSet &ps)
      : EcalUncalibRecHitWorkerBase(ps) {}
  virtual ~EcalUncalibRecHitWorkerWeights() {}

  virtual void set(const edm::EventSetup &es);
  virtual bool run(const edm::Event &evt,
                   const EcalDigiCollection::const_iterator &digi,
                   EcalUncalibratedRecHitCollection &result);

 protected:

  edm::ESHandle<EcalPedestals> peds;
  edm::ESHandle<EcalGainRatios> gains;
  edm::ESHandle<EcalWeightXtalGroups> grps;
  edm::ESHandle<EcalTBWeights> wgts;

  double pedVec[3];
  double pedRMSVec[3];
  double gainRatios[3];

  const EcalWeightSet::EcalWeightMatrix *weights[2];
  const EcalWeightSet::EcalChi2WeightMatrix *chi2mat[2];

  EcalUncalibRecHitRecWeightsAlgo<EBDataFrame> uncalibMaker_barrel_;
  EcalUncalibRecHitRecWeightsAlgo<EEDataFrame> uncalibMaker_endcap_;

  const EEShape testbeamEEShape;  // used in the chi2
  const EBShape testbeamEBShape;  // can be replaced by simple shape arrays of
                                  // floats in the future (kostas)
};

#endif
