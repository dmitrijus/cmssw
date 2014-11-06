import FWCore.ParameterSet.Config as cms

from PhysicsTools.PatAlgos.mcMatchLayer0.muonMatch_cfi import *
from TrackingTools.TransientTrack.TransientTrackBuilder_cfi import *
from PhysicsTools.PatAlgos.producersLayer1.muonProducer_cfi import *

from PhysicsTools.PatAlgos.recoLayer0.pfParticleSelectionForIso_cff import *
from RecoMuon.MuonIsolation.muonPFIsolation_cff import *

sourceMuons = patMuons.muonSource

muPFIsoDepositChargedPAT    = muPFIsoDepositCharged.clone()
muPFIsoDepositChargedAllPAT = muPFIsoDepositChargedAll.clone()
muPFIsoDepositNeutralPAT    = muPFIsoDepositNeutral.clone()
muPFIsoDepositGammaPAT      = muPFIsoDepositGamma.clone()
muPFIsoDepositPUPAT         = muPFIsoDepositPU.clone()

muPFIsoDepositChargedPAT.src = patMuons.muonSource
muPFIsoDepositChargedAllPAT.src = patMuons.muonSource
muPFIsoDepositNeutralPAT.src = patMuons.muonSource
muPFIsoDepositGammaPAT.src = patMuons.muonSource
muPFIsoDepositPUPAT.src = patMuons.muonSource
                              
muPFIsoValueCharged04PAT    = muPFIsoValueCharged04.clone()
muPFIsoValueChargedAll04PAT = muPFIsoValueChargedAll04.clone()
muPFIsoValuePU04PAT         = muPFIsoValuePU04.clone()
muPFIsoValueNeutral04PAT    = muPFIsoValueNeutral04.clone()
muPFIsoValueGamma04PAT      = muPFIsoValueGamma04.clone()

muPFIsoValueCharged04PAT.deposits.src = cms.InputTag("muPFIsoDepositChargedPAT")
muPFIsoValueChargedAll04PAT.deposits.src = cms.InputTag("muPFIsoDepositChargedAllPAT")
muPFIsoValuePU04PAT.deposits.src = cms.InputTag("muPFIsoDepositPUPAT")
muPFIsoValueNeutral04PAT.deposits.src = cms.InputTag("muPFIsoDepositNeutralPAT")
muPFIsoValueGamma04PAT.deposits.src = cms.InputTag("muPFIsoDepositGammaPAT")

patMuons.isoDeposits = cms.PSet(
    pfChargedHadrons = cms.InputTag("muPFIsoDepositChargedPAT" ),
    pfChargedAll = cms.InputTag("muPFIsoDepositChargedAllPAT" ),
    pfPUChargedHadrons = cms.InputTag("muPFIsoDepositPUPAT" ),
    pfNeutralHadrons = cms.InputTag("muPFIsoDepositNeutralPAT" ),
    pfPhotons = cms.InputTag("muPFIsoDepositGammaPAT" ),
)

patMuons.isolationValues = cms.PSet(
    pfChargedHadrons = cms.InputTag("muPFIsoValueCharged04PAT"),
    pfChargedAll = cms.InputTag("muPFIsoValueChargedAll04PAT"),
    pfPUChargedHadrons = cms.InputTag("muPFIsoValuePU04PAT" ),
    pfNeutralHadrons = cms.InputTag("muPFIsoValueNeutral04PAT" ),
    pfPhotons = cms.InputTag("muPFIsoValueGamma04PAT" ),
)
