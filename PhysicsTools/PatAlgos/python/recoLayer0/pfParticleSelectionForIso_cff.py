from RecoParticleFlow.PFProducer.pfLinker_cff import particleFlowPtrs
from CommonTools.ParticleFlow.pfParticleSelection_cff import *

#pfPileUp.PFCandidates = 'particleFlowPtrs'
#pfNoPileUp.bottomCollection = 'particleFlowPtrs'
#pfPileUpIso.PFCandidates = 'particleFlowPtrs'
#pfNoPileUpIso.bottomCollection='particleFlowPtrs'
#pfPileUpJME.PFCandidates = 'particleFlowPtrs'
#pfNoPileUpJME.bottomCollection='particleFlowPtrs'

#pfParticleSelectionForIsoSequence = cms.Sequence(
#    #particleFlowPtrs +
#    pfNoPileUpIsoSequence +
#    pfParticleSelectionSequence
#)

pfPileUpPAT = pfPileUp.clone(
    PFCandidates = cms.InputTag('particleFlowPtrs'),
)

pfNoPileUpPAT = pfNoPileUp.clone(
    bottomCollection = cms.InputTag('particleFlowPtrs'),
    topCollection = cms.InputTag('pfPileUpPAT'),
)

pfPileUpJMEPAT = pfPileUpJME.clone(
    PFCandidates = cms.InputTag('particleFlowPtrs')
)

pfNoPileUpJMEPAT = pfNoPileUpJME.clone(
    bottomCollection = cms.InputTag('particleFlowPtrs'),
    topCollection = cms.InputTag('pfPileUpJMEPAT')
)
