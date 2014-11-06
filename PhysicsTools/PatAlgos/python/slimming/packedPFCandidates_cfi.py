import FWCore.ParameterSet.Config as cms

packedPFCandidates = cms.EDProducer("PATPackedCandidateProducer",
    inputCollection = cms.InputTag("particleFlow"),
    inputCollectionFromPVLoose = cms.InputTag("pfNoPileUpJMEPAT"),
    inputCollectionFromPVTight = cms.InputTag("pfNoPileUpPAT"),
    inputVertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
    originalVertices = cms.InputTag("offlinePrimaryVertices"),
    originalTracks = cms.InputTag("generalTracks"),
    minPtForTrackProperties = cms.double(0.95)	
)
