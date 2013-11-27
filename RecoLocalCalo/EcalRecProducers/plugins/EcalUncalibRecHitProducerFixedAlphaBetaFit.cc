/**
 *   Produce ECAL uncalibrated rechits from dataframes with the analytic
 * specific fit method,  with alfa and beta fixed.
 *
 *  \author A. Ghezzi, Mar 2006
 *
 */

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/EcalRecHit/interface/EcalUncalibratedRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"

#include "CondFormats/EcalObjects/interface/EcalGainRatios.h"
#include "CondFormats/EcalObjects/interface/EcalPedestals.h"
#include "CondFormats/EcalObjects/interface/EcalMGPAGainRatio.h"
#include "CondFormats/DataRecord/interface/EcalPedestalsRcd.h"
#include "CondFormats/DataRecord/interface/EcalGainRatiosRcd.h"

#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerBase.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitFixedAlphaBetaAlgo.h"

#include <iostream>
#include <cmath>
#include <fstream>

class EcalUncalibRecHitProducerFixedAlphaBetaFit
    : public EcalUncalibRecHitProducerBase {

 public:
  EcalUncalibRecHitProducerFixedAlphaBetaFit(const edm::ParameterSet& ps);
  virtual ~EcalUncalibRecHitProducerFixedAlphaBetaFit() {};

  virtual void set(const edm::EventSetup& es);
  virtual bool run(const edm::Event& evt,
                   const EcalDigiCollection::const_iterator& digi,
                   EcalUncalibratedRecHitCollection& result);

 private:

  double AmplThrEB_;
  double AmplThrEE_;

  EcalUncalibRecHitFixedAlphaBetaAlgo<EBDataFrame> algoEB_;
  EcalUncalibRecHitFixedAlphaBetaAlgo<EEDataFrame> algoEE_;

  double alphaEB_;
  double betaEB_;
  double alphaEE_;
  double betaEE_;
  std::vector<std::vector<std::pair<double, double> > >
      alphaBetaValues_;  // List of alpha and Beta values [SM#][CRY#](alpha,
                         // beta)
  bool useAlphaBetaArray_;
  std::string alphabetaFilename_;

  bool setAlphaBeta();  // Sets the alphaBetaValues_ vectors by the values
                        // provided in alphabetaFilename_

  edm::ESHandle<EcalGainRatios> pRatio;
  edm::ESHandle<EcalPedestals> pedHandle;
};

EcalUncalibRecHitProducerFixedAlphaBetaFit::
    EcalUncalibRecHitProducerFixedAlphaBetaFit(const edm::ParameterSet& ps)
    : EcalUncalibRecHitProducerBase(ps) {

  alphaEB_ = ps.getParameter<double>("alphaEB");
  betaEB_ = ps.getParameter<double>("betaEB");
  alphaEE_ = ps.getParameter<double>("alphaEE");
  betaEE_ = ps.getParameter<double>("betaEE");

  alphabetaFilename_ =
      ps.getUntrackedParameter<std::string>("AlphaBetaFilename", "NOFILE");
  useAlphaBetaArray_ =
      setAlphaBeta();  // set crystalwise values of alpha and beta
  if (!useAlphaBetaArray_) {
    edm::LogInfo("EcalUncalibRecHitError")
        << " No alfa-beta file found. Using the deafult values.";
  }

  algoEB_.SetMinAmpl(ps.getParameter<double>("MinAmplBarrel"));
  algoEE_.SetMinAmpl(ps.getParameter<double>("MinAmplEndcap"));

  bool dyn_pede = ps.getParameter<bool>("UseDynamicPedestal");
  algoEB_.SetDynamicPedestal(dyn_pede);
  algoEE_.SetDynamicPedestal(dyn_pede);
}

void EcalUncalibRecHitProducerFixedAlphaBetaFit::set(
    const edm::EventSetup& es) {
  // Gain Ratios
  LogDebug("EcalUncalibRecHitDebug") << "fetching gainRatios....";
  es.get<EcalGainRatiosRcd>().get(pRatio);
  LogDebug("EcalUncalibRecHitDebug") << "done.";

  // fetch the pedestals from the cond DB via EventSetup
  LogDebug("EcalUncalibRecHitDebug") << "fetching pedestals....";
  es.get<EcalPedestalsRcd>().get(pedHandle);
  LogDebug("EcalUncalibRecHitDebug") << "done.";
}

//Sets the alphaBetaValues_ vectors by the values provided in alphabetaFilename_
bool EcalUncalibRecHitProducerFixedAlphaBetaFit::setAlphaBeta() {
  std::ifstream file(alphabetaFilename_.c_str());
  if (!file.is_open()) return false;

  alphaBetaValues_.resize(36);

  char buffer[100];
  int sm, cry, ret;
  float a, b;
  std::pair<double, double> p(-1, -1);

  while (!file.getline(buffer, 100).eof()) {
    ret = sscanf(buffer, "%d %d %f %f", &sm, &cry, &a, &b);
    if ((ret != 4) || (sm <= 0) || (sm > 36) || (cry <= 0) || (cry > 1700)) {
      // send warning
      continue;
    }

    if (alphaBetaValues_[sm - 1].size() == 0) {
      alphaBetaValues_[sm - 1].resize(1700, p);
    }
    alphaBetaValues_[sm - 1][cry - 1].first = a;
    alphaBetaValues_[sm - 1][cry - 1].second = b;

  }

  file.close();
  return true;
}

bool EcalUncalibRecHitProducerFixedAlphaBetaFit::run(
    const edm::Event& evt, const EcalDigiCollection::const_iterator& itdg,
    EcalUncalibratedRecHitCollection& result) {

  const EcalGainRatioMap& gainMap =
      pRatio.product()->getMap();             // map of gain ratios
  EcalGainRatioMap::const_iterator gainIter;  // gain iterator
  EcalMGPAGainRatio aGain;                    // gain object for a single xtal

  const EcalPedestalsMap& pedMap =
      pedHandle.product()->getMap();  // map of pedestals
  EcalPedestalsMapIterator pedIter;   // pedestal iterator
  EcalPedestals::Item aped;           // pedestal object for a single xtal

  DetId detid(itdg->id());

  // find pedestals for this channel
  //LogDebug("EcalUncalibRecHitDebug") << "looking up pedestal for crystal: " <<
  //itdg->id();
  pedIter = pedMap.find(itdg->id());
  if (pedIter != pedMap.end()) {
    aped = (*pedIter);
  } else {
    edm::LogError("EcalUncalibRecHitProducerFixedAlphaBetaFit")
        << "error!! could not find pedestals for channel: ";
    if (detid.subdetId() == EcalBarrel) {
      edm::LogError("EcalUncalibRecHitProducerFixedAlphaBetaFit")
          << EBDetId(detid);
    } else {
      edm::LogError("EcalUncalibRecHitProducerFixedAlphaBetaFit")
          << EEDetId(detid);
    }
    edm::LogError("EcalUncalibRecHitProducerFixedAlphaBetaFit")
        << "\n  no uncalib rechit will be made for this digi!";
    return false;
  }
  double pedVec[3];
  pedVec[0] = aped.mean_x12;
  pedVec[1] = aped.mean_x6;
  pedVec[2] = aped.mean_x1;

  // find gain ratios
  //LogDebug("EcalUncalibRecHitDebug") << "looking up gainRatios for crystal: "
  //<< EBDetId(itdg->id()) ; // FIXME!!!!!!!!
  gainIter = gainMap.find(itdg->id());
  if (gainIter != gainMap.end()) {
    aGain = (*gainIter);
  } else {
    edm::LogError("EcalUncalibRecHitProducerFixedAlphaBetaFit")
        << "error!! could not find gain ratios for channel: ";
    if (detid.subdetId() == EcalBarrel) {
      edm::LogError("EcalUncalibRecHitProducerFixedAlphaBetaFit")
          << EBDetId(detid);
    } else {
      edm::LogError("EcalUncalibRecHitProducerFixedAlphaBetaFit")
          << EEDetId(detid);
    }
    edm::LogError("EcalUncalibRecHitProducerFixedAlphaBetaFit")
        << "\n  no uncalib rechit will be made for this digi!";
    return false;
  }
  double gainRatios[3];
  gainRatios[0] = 1.;
  gainRatios[1] = aGain.gain12Over6();
  gainRatios[2] = aGain.gain6Over1() * aGain.gain12Over6();

  if (detid.subdetId() == EcalBarrel) {
    // Define Alpha and Beta either by stored values or by default universal
    // values
    EBDetId ebDetId(detid);
    double a, b;
    if (useAlphaBetaArray_) {
      if (alphaBetaValues_[ebDetId.ism() - 1].size() != 0) {
        a = alphaBetaValues_[ebDetId.ism() - 1][ebDetId.ic() - 1].first;
        b = alphaBetaValues_[ebDetId.ism() - 1][ebDetId.ic() - 1].second;
        if ((a == -1) && (b == -1)) {
          a = alphaEB_;
          b = betaEB_;
        }
      } else {
        a = alphaEB_;
        b = betaEB_;
      }
    } else {
      a = alphaEB_;
      b = betaEB_;
    }
    algoEB_.SetAlphaBeta(a, b);
    result.push_back(algoEB_.makeRecHit(*itdg, pedVec, gainRatios, 0, 0));
  } else {
    //FIX ME load in a and b from a file
    algoEE_.SetAlphaBeta(alphaEE_, betaEE_);
    result.push_back(algoEE_.makeRecHit(*itdg, pedVec, gainRatios, 0, 0));
  }
  return true;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(EcalUncalibRecHitProducerFixedAlphaBetaFit);
