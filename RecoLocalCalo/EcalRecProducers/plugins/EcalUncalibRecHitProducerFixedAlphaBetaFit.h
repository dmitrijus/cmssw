#ifndef RecoLocalCalo_EcalRecProducers_plugins_EcalUncalibRecHitProducerFixedAlphaBetaFit_h
#define RecoLocalCalo_EcalRecProducers_plugins_EcalUncalibRecHitProducerFixedAlphaBetaFit_h

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

#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerBase.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitFixedAlphaBetaAlgo.h"

class EcalUncalibRecHitProducerFixedAlphaBetaFit
    : public EcalUncalibRecHitProducerBase {

 public:
  EcalUncalibRecHitProducerFixedAlphaBetaFit(const edm::ParameterSet& ps);
  virtual ~EcalUncalibRecHitProducerFixedAlphaBetaFit();

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

#endif
