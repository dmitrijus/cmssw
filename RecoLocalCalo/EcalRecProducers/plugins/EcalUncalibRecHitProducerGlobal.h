#ifndef RecoLocalCalo_EcalRecProducers_plugins_EcalUncalibRecHitProducerGlobal_h
#define RecoLocalCalo_EcalRecProducers_plugins_EcalUncalibRecHitProducerGlobal_h

/**
 *  Template used to compute amplitude, pedestal using a weights method,
 *                           time using a ratio method,
 *                           chi2 using express method.
 *
 *  \author R. Bruneliere - A. Zabi
 */

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "CondFormats/EcalObjects/interface/EcalTimeCalibConstants.h"
#include "CondFormats/EcalObjects/interface/EcalTimeOffsetConstant.h"
#include "CondFormats/EcalObjects/interface/EcalPedestals.h"
#include "CondFormats/EcalObjects/interface/EcalGainRatios.h"
#include "CondFormats/EcalObjects/interface/EcalWeightXtalGroups.h"
#include "CondFormats/EcalObjects/interface/EcalTBWeights.h"
#include "CondFormats/EcalObjects/interface/EcalSampleMask.h"
#include "CondFormats/EcalObjects/interface/EcalTimeBiasCorrections.h"

#include "CondFormats/DataRecord/interface/EcalGainRatiosRcd.h"
#include "CondFormats/DataRecord/interface/EcalPedestalsRcd.h"
#include "CondFormats/DataRecord/interface/EcalWeightXtalGroupsRcd.h"
#include "CondFormats/DataRecord/interface/EcalTBWeightsRcd.h"
#include "CondFormats/DataRecord/interface/EcalSampleMaskRcd.h"
#include "CondFormats/DataRecord/interface/EcalTimeCalibConstantsRcd.h"
#include "CondFormats/DataRecord/interface/EcalTimeOffsetConstantRcd.h"
#include "CondFormats/DataRecord/interface/EcalTimeBiasCorrectionsRcd.h"

#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"

#include "SimCalorimetry/EcalSimAlgos/interface/EBShape.h"
#include "SimCalorimetry/EcalSimAlgos/interface/EEShape.h"

#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerBase.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitRecWeightsAlgo.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitRecChi2Algo.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitRatioMethodAlgo.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitLeadingEdgeAlgo.h"

class EcalUncalibRecHitProducerGlobal : public EcalUncalibRecHitProducerBase {

 public:
  EcalUncalibRecHitProducerGlobal(const edm::ParameterSet&);
  virtual ~EcalUncalibRecHitProducerGlobal();

  virtual bool run(const edm::Event& evt,
           const EcalDigiCollection::const_iterator& digi,
           EcalUncalibratedRecHitCollection& result);

  virtual void set(const edm::EventSetup& es);

 protected:

  double pedVec[3];
  double pedRMSVec[3];
  double gainRatios[3];

  edm::ESHandle<EcalPedestals> peds;
  edm::ESHandle<EcalGainRatios> gains;

  template <class C> int isSaturated(const C& digi);

  double timeCorrectionEB(float ampliEB);
  double timeCorrectionEE(float ampliEE);

  // weights method
  edm::ESHandle<EcalWeightXtalGroups> grps;
  edm::ESHandle<EcalTBWeights> wgts;
  const EcalWeightSet::EcalWeightMatrix* weights[2];
  const EcalWeightSet::EcalChi2WeightMatrix* chi2mat[2];
  EcalUncalibRecHitRecWeightsAlgo<EBDataFrame> weightsMethod_barrel_;
  EcalUncalibRecHitRecWeightsAlgo<EEDataFrame> weightsMethod_endcap_;
  const EEShape testbeamEEShape;  // used in the chi2
  const EBShape testbeamEBShape;  // can be replaced by simple shape arrays of
                                  // float in the future

  // determie which of the samples must actually be used by ECAL local reco
  edm::ESHandle<EcalSampleMask> sampleMaskHand_;

  // ratio method
  std::vector<double> EBtimeFitParameters_;
  std::vector<double> EEtimeFitParameters_;
  std::vector<double> EBamplitudeFitParameters_;
  std::vector<double> EEamplitudeFitParameters_;
  std::pair<double, double> EBtimeFitLimits_;
  std::pair<double, double> EEtimeFitLimits_;

  bool doEBtimeCorrection_;
  bool doEEtimeCorrection_;

  EcalUncalibRecHitRatioMethodAlgo<EBDataFrame> ratioMethod_barrel_;
  EcalUncalibRecHitRatioMethodAlgo<EEDataFrame> ratioMethod_endcap_;

  double EBtimeConstantTerm_;
  double EBtimeNconst_;
  double EEtimeConstantTerm_;
  double EEtimeNconst_;
  double outOfTimeThreshG12pEB_;
  double outOfTimeThreshG12mEB_;
  double outOfTimeThreshG61pEB_;
  double outOfTimeThreshG61mEB_;
  double outOfTimeThreshG12pEE_;
  double outOfTimeThreshG12mEE_;
  double outOfTimeThreshG61pEE_;
  double outOfTimeThreshG61mEE_;
  double amplitudeThreshEB_;
  double amplitudeThreshEE_;
  double ebSpikeThresh_;

#ifdef USE_ES_FOR_TIMEBIAS
  edm::ESHandle<EcalTimeBiasCorrections> timeCorrBias_;
#else
  EcalTimeBiasCorrections *timeCorrBias_;
#endif

  // leading edge method
  edm::ESHandle<EcalTimeCalibConstants> itime;
  edm::ESHandle<EcalTimeOffsetConstant> offtime;

  std::vector<double> ebPulseShape_;
  std::vector<double> eePulseShape_;
  EcalUncalibRecHitLeadingEdgeAlgo<EBDataFrame> leadingEdgeMethod_barrel_;
  EcalUncalibRecHitLeadingEdgeAlgo<EEDataFrame> leadingEdgeMethod_endcap_;

  // chi2 method
  bool kPoorRecoFlagEB_;
  bool kPoorRecoFlagEE_;
  double chi2ThreshEB_;
  double chi2ThreshEE_;
  std::vector<double> EBchi2Parameters_;
  std::vector<double> EEchi2Parameters_;
};

#endif
