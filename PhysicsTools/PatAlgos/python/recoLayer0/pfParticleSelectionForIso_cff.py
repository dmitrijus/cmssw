from RecoParticleFlow.PFProducer.particleFlowTmpPtrs_cfi import particleFlowTmpPtrs
from CommonTools.ParticleFlow.pfParticleSelection_cff import *

pfPileUp.PFCandidates = 'particleFlowTmpPtrs'
pfNoPileUp.bottomCollection = 'particleFlowTmpPtrs'
pfPileUpIso.PFCandidates = 'particleFlowTmpPtrs'
pfNoPileUpIso.bottomCollection='particleFlowTmpPtrs'
pfPileUpJME.PFCandidates = 'particleFlowTmpPtrs'
pfNoPileUpJME.bottomCollection='particleFlowTmpPtrs'

pfParticleSelectionForIsoSequence = cms.Sequence(
    particleFlowTmpPtrs +
    pfNoPileUpIsoSequence +
    pfParticleSelectionSequence
    )
