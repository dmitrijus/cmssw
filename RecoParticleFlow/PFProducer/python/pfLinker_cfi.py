import FWCore.ParameterSet.Config as cms

pfLinker = cms.EDProducer("PFLinker",
                          PFCandidate = cms.VInputTag(cms.InputTag("particleFlowTmp")),
                          GsfElectrons = cms.InputTag("gedGsfElectrons"),
                          Photons = cms.InputTag("gedPhotons"),
                          Muons = cms.InputTag("muons","muons1stStep2muonsMap"),
                          ProducePFCandidates = cms.bool(True),
                          FillMuonRefs = cms.bool(True),
                          OutputPF = cms.string(""),
                          ValueMapElectrons = cms.string("electrons"),                              
                          ValueMapPhotons = cms.string("photons"),
                          ValueMapMerged = cms.string("all")
                          )
