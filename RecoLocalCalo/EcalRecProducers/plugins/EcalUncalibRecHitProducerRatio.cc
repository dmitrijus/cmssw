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

#include "CondFormats/DataRecord/interface/EcalSampleMaskRcd.h"
#include "CondFormats/DataRecord/interface/EcalGainRatiosRcd.h"
#include "CondFormats/DataRecord/interface/EcalPedestalsRcd.h"

#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerBase.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitRatioMethodAlgo.h"

class EcalUncalibRecHitProducerRatio : public EcalUncalibRecHitProducerBase {

 public:
  EcalUncalibRecHitProducerRatio(const edm::ParameterSet&);
  virtual ~EcalUncalibRecHitProducerRatio() {};

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

EcalUncalibRecHitProducerRatio::EcalUncalibRecHitProducerRatio(
    const edm::ParameterSet& ps)
    : EcalUncalibRecHitProducerBase(ps) {
  EBtimeFitParameters_ =
      ps.getParameter<std::vector<double> >("EBtimeFitParameters");
  EEtimeFitParameters_ =
      ps.getParameter<std::vector<double> >("EEtimeFitParameters");

  EBamplitudeFitParameters_ =
      ps.getParameter<std::vector<double> >("EBamplitudeFitParameters");
  EEamplitudeFitParameters_ =
      ps.getParameter<std::vector<double> >("EEamplitudeFitParameters");

  EBtimeFitLimits_.first = ps.getParameter<double>("EBtimeFitLimits_Lower");
  EBtimeFitLimits_.second = ps.getParameter<double>("EBtimeFitLimits_Upper");

  EEtimeFitLimits_.first = ps.getParameter<double>("EEtimeFitLimits_Lower");
  EEtimeFitLimits_.second = ps.getParameter<double>("EEtimeFitLimits_Upper");

  EBtimeConstantTerm_ = ps.getParameter<double>("EBtimeConstantTerm");
  EEtimeConstantTerm_ = ps.getParameter<double>("EEtimeConstantTerm");
}

void EcalUncalibRecHitProducerRatio::set(const edm::EventSetup& es) {

  // which of the samples need be used
  es.get<EcalSampleMaskRcd>().get(sampleMaskHand_);

  es.get<EcalGainRatiosRcd>().get(gains);
  es.get<EcalPedestalsRcd>().get(peds);

}

bool EcalUncalibRecHitProducerRatio::run(
    const edm::Event& evt, const EcalDigiCollection::const_iterator& itdg,
    EcalUncalibratedRecHitCollection& result) {
  DetId detid(itdg->id());

  const EcalSampleMask* sampleMask_ = sampleMaskHand_.product();

  const EcalPedestals::Item* aped = 0;
  const EcalMGPAGainRatio* aGain = 0;

  if (detid.subdetId() == EcalEndcap) {
    unsigned int hashedIndex = EEDetId(detid).hashedIndex();
    aped = &peds->endcap(hashedIndex);
    aGain = &gains->endcap(hashedIndex);
  } else {
    unsigned int hashedIndex = EBDetId(detid).hashedIndex();
    aped = &peds->barrel(hashedIndex);
    aGain = &gains->barrel(hashedIndex);
  }

  pedVec[0] = aped->mean_x12;
  pedVec[1] = aped->mean_x6;
  pedVec[2] = aped->mean_x1;
  pedRMSVec[0] = aped->rms_x12;
  pedRMSVec[1] = aped->rms_x6;
  pedRMSVec[2] = aped->rms_x1;
  gainRatios[0] = 1.;
  gainRatios[1] = aGain->gain12Over6();
  gainRatios[2] = aGain->gain6Over1() * aGain->gain12Over6();

  float clockToNsConstant = 25.;
  EcalUncalibratedRecHit uncalibRecHit;

  if (detid.subdetId() == EcalEndcap) {

    uncalibRecHit = uncalibMaker_endcap_.makeRecHit(
        *itdg, *sampleMask_, pedVec, pedRMSVec, gainRatios,
        EEtimeFitParameters_, EEamplitudeFitParameters_,
        EEtimeFitLimits_);  //GF pass mask here

    EcalUncalibRecHitRatioMethodAlgo<EEDataFrame>::CalculatedRecHit crh =
        uncalibMaker_endcap_.getCalculatedRecHit();
    uncalibRecHit.setAmplitude(crh.amplitudeMax);
    uncalibRecHit.setJitter(crh.timeMax - 5);
    uncalibRecHit.setJitterError(
        std::sqrt(pow(crh.timeError, 2) + std::pow(EEtimeConstantTerm_, 2) /
                                              std::pow(clockToNsConstant, 2)));

  } else {

    bool gainSwitch = uncalibMaker_barrel_.fixMGPAslew(*itdg);

    uncalibRecHit = uncalibMaker_barrel_.makeRecHit(
        *itdg, *sampleMask_, pedVec, pedRMSVec, gainRatios,
        EBtimeFitParameters_, EBamplitudeFitParameters_,
        EBtimeFitLimits_);  //GF pass mask here

    EcalUncalibRecHitRatioMethodAlgo<EBDataFrame>::CalculatedRecHit crh =
        uncalibMaker_barrel_.getCalculatedRecHit();

    uncalibRecHit.setAmplitude(crh.amplitudeMax);
    if (gainSwitch) {
      // introduce additional 1ns shift
      uncalibRecHit.setJitter(crh.timeMax - 5 - 0.04);
    } else {
      uncalibRecHit.setJitter(crh.timeMax - 5);
    }
    uncalibRecHit.setJitterError(
        std::sqrt(pow(crh.timeError, 2) + std::pow(EBtimeConstantTerm_, 2) /
                                              std::pow(clockToNsConstant, 2)));

  }
  result.push_back(uncalibRecHit);

  return true;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(EcalUncalibRecHitProducerRatio);
