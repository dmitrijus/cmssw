import FWCore.ParameterSet.Config as cms

from CommonTools.ParticleFlow.pfPileUp_cfi  import *
from CommonTools.ParticleFlow.TopProjectors.pfNoPileUp_cfi import *
from CommonTools.ParticleFlow.goodOfflinePrimaryVertices_cfi import *

pfPileUpJME = pfPileUp.clone( PFCandidates='particleFlowTmpPtrs',
                              Vertices = 'goodOfflinePrimaryVertices',
                              checkClosestZVertex = False )
pfNoPileUpJME = pfNoPileUp.clone( topCollection = 'pfPileUpJME',
                                  bottomCollection = 'particleFlowTmpPtrs' )



pfNoPileUpJMESequence = cms.Sequence(
    goodOfflinePrimaryVertices +
    pfPileUpJME +
    pfNoPileUpJME
    )
