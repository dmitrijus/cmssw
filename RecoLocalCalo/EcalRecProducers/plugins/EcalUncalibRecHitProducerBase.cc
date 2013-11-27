#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerBase.h"

EcalUncalibRecHitProducerBase::EcalUncalibRecHitProducerBase(
    const edm::ParameterSet& ps) {
  ebHitCollection_ = ps.getParameter<std::string>("EBhitCollection");
  eeHitCollection_ = ps.getParameter<std::string>("EEhitCollection");
  produces<EBUncalibratedRecHitCollection>(ebHitCollection_);
  produces<EEUncalibratedRecHitCollection>(eeHitCollection_);

  ebDigiCollectionToken_ = consumes<EBDigiCollection>(
      ps.getParameter<edm::InputTag>("EBdigiCollection"));

  eeDigiCollectionToken_ = consumes<EEDigiCollection>(
      ps.getParameter<edm::InputTag>("EEdigiCollection"));
}

EcalUncalibRecHitProducerBase::~EcalUncalibRecHitProducerBase() {}

void EcalUncalibRecHitProducerBase::produce(edm::Event& evt,
                                            const edm::EventSetup& es) {

  using namespace edm;

  Handle<EBDigiCollection> pEBDigis;
  Handle<EEDigiCollection> pEEDigis;

  const EBDigiCollection* ebDigis = 0;
  const EEDigiCollection* eeDigis = 0;

  evt.getByToken(ebDigiCollectionToken_, pEBDigis);
  ebDigis = pEBDigis.product();  // get a ptr to the produc
  edm::LogInfo("EcalUncalibRecHitInfo")
      << "total # ebDigis: " << ebDigis->size();

  evt.getByToken(eeDigiCollectionToken_, pEEDigis);
  eeDigis = pEEDigis.product();  // get a ptr to the product
  edm::LogInfo("EcalUncalibRecHitInfo")
      << "total # eeDigis: " << eeDigis->size();

  // tranparently get things from event setup
  this->set(es);

  // prepare output
  std::auto_ptr<EBUncalibratedRecHitCollection> ebUncalibRechits(
      new EBUncalibratedRecHitCollection);
  std::auto_ptr<EEUncalibratedRecHitCollection> eeUncalibRechits(
      new EEUncalibratedRecHitCollection);

  // loop over EB digis
  if (ebDigis) {
    ebUncalibRechits->reserve(ebDigis->size());
    for (EBDigiCollection::const_iterator itdg = ebDigis->begin();
         itdg != ebDigis->end(); ++itdg) {
      this->run(evt, itdg, *ebUncalibRechits);
    }
  }

  // loop over EB digis
  if (eeDigis) {
    eeUncalibRechits->reserve(eeDigis->size());
    for (EEDigiCollection::const_iterator itdg = eeDigis->begin();
         itdg != eeDigis->end(); ++itdg) {
      this->run(evt, itdg, *eeUncalibRechits);
    }
  }

  // put the collection of recunstructed hits in the event
  evt.put(ebUncalibRechits, ebHitCollection_);
  evt.put(eeUncalibRechits, eeHitCollection_);
}
