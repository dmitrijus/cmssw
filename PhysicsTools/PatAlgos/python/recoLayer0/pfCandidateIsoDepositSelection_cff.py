import FWCore.ParameterSet.Config as cms

from CommonTools.ParticleFlow.ParticleSelectors.pfSortByType_cff import *
from CommonTools.ParticleFlow.pfNoPileUpIso_cff  import *

pfPileUpIso.PFCandidates = 'particleFlowTmpPtrs'
pfNoPileUpIso.bottomCollection='particleFlowTmpPtrs'

patPFCandidateIsoDepositSelection = cms.Sequence(
       pfNoPileUpIsoSequence +
       pfSortByTypeSequence
       )
