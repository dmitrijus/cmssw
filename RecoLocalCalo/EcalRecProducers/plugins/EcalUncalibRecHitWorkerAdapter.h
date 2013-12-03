#ifndef RecoLocalCalo_EcalRecProducers_EcalUncalibRecHitWorkerAdapter_hh
#define RecoLocalCalo_EcalRecProducers_EcalUncalibRecHitWorkerAdapter_hh

/**
  *  Adapter class to convert old style worker classes to new style producer
  * classes.
  *
  *  \author Dmitrijus Bugelskis
  */

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerBase.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalUncalibRecHitRecWeightsAlgo.h"

template <class TWorker>
class EcalUncalibRecHitWorkerAdapter : public EcalUncalibRecHitProducerBase {
 public:
  EcalUncalibRecHitWorkerAdapter(const edm::ParameterSet &ps)
      : EcalUncalibRecHitProducerBase(ps), worker_(ps) {}
  virtual ~EcalUncalibRecHitWorkerAdapter() {}

  virtual void set(const edm::EventSetup &es) { worker_.set(es); }

  virtual bool run(const edm::Event &evt,
                   const EcalDigiCollection::const_iterator &digi,
                   EcalUncalibratedRecHitCollection &result) {

    return worker_.run(evt, digi, result);
  }

 protected:
  TWorker worker_;
};

#endif
