import FWCore.ParameterSet.Config as cms

def dumpOnEndJob(process):
    process.load("DQMServices.Components.DQMStreamStats_cfi")
    process.DQMStreamStats.dumpOnEndJob = True

    process.stream = cms.EndPath(process.DQMStreamStats)
    process.schedule.append(process.stream)
    return(process)

