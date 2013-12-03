#ifndef RecoLocalCalo_EcalRecProducers_UnpackerWorkerCompat_hh
#define RecoLocalCalo_EcalRecProducers_UnpackerWorkerCompat_hh

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/PluginManager/interface/PluginFactory.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Event.h"

#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerBase.h"

#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

class EcalUncalibRecHitWorkerBase {
 public:
  EcalUncalibRecHitWorkerBase(const edm::ParameterSet &ps) {}
  ;
  virtual ~EcalUncalibRecHitWorkerBase() {}
  ;

  virtual void set(const edm::EventSetup &es) = 0;
  virtual bool run(const edm::Event &evt,
                   const EcalDigiCollection::const_iterator &digi,
                   EcalUncalibratedRecHitCollection &result) = 0;
};

typedef edmplugin::PluginFactory<EcalUncalibRecHitWorkerBase *(
    const edm::ParameterSet &)> EcalUncalibRecHitWorkerFactory;

#endif
