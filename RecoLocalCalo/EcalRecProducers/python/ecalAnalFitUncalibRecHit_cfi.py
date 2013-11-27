import FWCore.ParameterSet.Config as cms

# producer of rechits starting from digis
ecalAnalFitUncalibRecHit = cms.EDProducer("EcalUncalibRecHitProducerAnalFit",
    EBdigiCollection = cms.InputTag("ecalDigis","ebDigis"),
    EEhitCollection = cms.string('EcalUncalibRecHitsEE'),
    EEdigiCollection = cms.InputTag("ecalDigis","eeDigis"),
    EBhitCollection = cms.string("EcalUncalibRecHitsEB"),
)
