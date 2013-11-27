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

#include "CondFormats/DataRecord/interface/EcalGainRatiosRcd.h"
#include "CondFormats/DataRecord/interface/EcalPedestalsRcd.h"
#include "CondFormats/DataRecord/interface/EcalWeightXtalGroupsRcd.h"
#include "CondFormats/DataRecord/interface/EcalTBWeightsRcd.h"

#include "SimCalorimetry/EcalSimAlgos/interface/EBShape.h"
#include "SimCalorimetry/EcalSimAlgos/interface/EEShape.h"

#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerBase.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitRecWeightsAlgo.h"

class EcalUncalibRecHitProducerWeights : public EcalUncalibRecHitProducerBase {

 public:
  EcalUncalibRecHitProducerWeights(const edm::ParameterSet &ps)
      : EcalUncalibRecHitProducerBase(ps) {}
  ;
  virtual ~EcalUncalibRecHitProducerWeights() {}
  ;

  void set(const edm::EventSetup &es);
  bool run(const edm::Event &evt,
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

void EcalUncalibRecHitProducerWeights::set(const edm::EventSetup &es) {
  es.get<EcalGainRatiosRcd>().get(gains);
  es.get<EcalPedestalsRcd>().get(peds);
  es.get<EcalWeightXtalGroupsRcd>().get(grps);
  es.get<EcalTBWeightsRcd>().get(wgts);
}

bool EcalUncalibRecHitProducerWeights::run(
    const edm::Event &evt, const EcalDigiCollection::const_iterator &itdg,
    EcalUncalibratedRecHitCollection &result) {
  DetId detid(itdg->id());

  const EcalPedestals::Item *aped = 0;
  const EcalMGPAGainRatio *aGain = 0;
  const EcalXtalGroupId *gid = 0;
  EcalTBWeights::EcalTDCId tdcid(1);

  if (detid.subdetId() == EcalEndcap) {
    unsigned int hashedIndex = EEDetId(detid).hashedIndex();
    aped = &peds->endcap(hashedIndex);
    aGain = &gains->endcap(hashedIndex);
    gid = &grps->endcap(hashedIndex);
  } else {
    unsigned int hashedIndex = EBDetId(detid).hashedIndex();
    aped = &peds->barrel(hashedIndex);
    aGain = &gains->barrel(hashedIndex);
    gid = &grps->barrel(hashedIndex);
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

  // now lookup the correct weights in the map
  EcalTBWeights::EcalTBWeightMap const &wgtsMap = wgts->getMap();
  EcalTBWeights::EcalTBWeightMap::const_iterator wit;
  wit = wgtsMap.find(std::make_pair(*gid, tdcid));
  if (wit == wgtsMap.end()) {
    edm::LogError("EcalUncalibRecHitError")
        << "No weights found for EcalGroupId: " << gid->id()
        << " and  EcalTDCId: " << tdcid
        << "\n  skipping digi with id: " << detid.rawId();

    return false;
  }
  const EcalWeightSet &wset = wit->second;  // this is the EcalWeightSet

  const EcalWeightSet::EcalWeightMatrix &mat1 =
      wset.getWeightsBeforeGainSwitch();
  const EcalWeightSet::EcalWeightMatrix &mat2 =
      wset.getWeightsAfterGainSwitch();
  //        const EcalWeightSet::EcalChi2WeightMatrix& mat3 =
  // wset.getChi2WeightsBeforeGainSwitch();
  //        const EcalWeightSet::EcalChi2WeightMatrix& mat4 =
  // wset.getChi2WeightsAfterGainSwitch();

  weights[0] = &mat1;
  weights[1] = &mat2;

  //        chi2mat[0] = &mat3;
  //        chi2mat[1] = &mat4;
  /*
          if (detid.subdetId()==EcalEndcap) {
                  result.push_back(uncalibMaker_endcap_.makeRecHit(*itdg,
          pedVec, gainRatios, weights, chi2mat));
          } else {
                  result.push_back(uncalibMaker_barrel_.makeRecHit(*itdg,
          pedVec, gainRatios, weights, chi2mat));
          }
  */
  if (detid.subdetId() == EcalEndcap) {
    result.push_back(uncalibMaker_endcap_.makeRecHit(
        *itdg, pedVec, pedRMSVec, gainRatios, weights, testbeamEEShape));
  } else {
    result.push_back(uncalibMaker_barrel_.makeRecHit(
        *itdg, pedVec, pedRMSVec, gainRatios, weights, testbeamEBShape));
  }
  return true;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(EcalUncalibRecHitProducerWeights);

#include "RecoLocalCalo/EcalRecProducers/interface/UnpackerWorkerCompat.h"
typedef EcalUncalibRecHitProducerWeights EcalUncalibRecHitWorkerWeightsCompat;
typedef EcalUncalibRecHitProducerWeights EcalUncalibRecHitWorkerWeightsCompatOld;
DEFINE_EDM_PLUGIN(EcalUncalibRecHitWorkerFactory, EcalUncalibRecHitWorkerWeightsCompat, "EcalUncalibRecHitWorkerWeights");
DEFINE_EDM_PLUGIN(EcalUncalibRecHitWorkerFactory, EcalUncalibRecHitWorkerWeightsCompatOld, "EcalUncalibRecHitWorkerWeightsOld");
