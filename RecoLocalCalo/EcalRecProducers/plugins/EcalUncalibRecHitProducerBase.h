#ifndef RecoLocalCalo_EcalRecProducers_EcalUncalibRecHitProducerBase_hh
#define RecoLocalCalo_EcalRecProducers_EcalUncalibRecHitProducerBase_hh

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"

#include "DataFormats/EcalDigi/interface/EEDataFrame.h"
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"


class EBDigiCollection;
class EEDigiCollection;

class EcalUncalibRecHitProducerBase : public edm::EDProducer {

 public:
  explicit EcalUncalibRecHitProducerBase(const edm::ParameterSet& ps);
  ~EcalUncalibRecHitProducerBase();

  virtual void produce(edm::Event& evt, const edm::EventSetup& es);

  /* worker interface */
  virtual bool run(const edm::Event& evt,
           const EcalDigiCollection::const_iterator& digi,
           EcalUncalibratedRecHitCollection& result) = 0;

  virtual void set(const edm::EventSetup& es) = 0;

 private:

  edm::EDGetTokenT<EBDigiCollection> ebDigiCollectionToken_;
  edm::EDGetTokenT<EEDigiCollection> eeDigiCollectionToken_;

  std::string ebHitCollection_;
  std::string eeHitCollection_;
};

#endif
