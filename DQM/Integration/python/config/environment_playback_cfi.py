import FWCore.ParameterSet.Config as cms
import os

#from DQMServices.Core.DQM_cfg import *
from DQMServices.Core.DQMStore_cfi import *

DQM = cms.Service("DQM",
                  debug = cms.untracked.bool(False),
                  publishFrequency = cms.untracked.double(5.0),
                  collectorPort = cms.untracked.int32(9090),
                  collectorHost = cms.untracked.string('dqm-c2d07-11.cms'),
                  filter = cms.untracked.string('')
                  )

DQMMonitoringService = cms.Service("DQMMonitoringService")

from DQMServices.Components.DQMEventInfo_cfi import *
from DQMServices.FileIO.DQMFileSaverOnline import *

dqmSaver.dirName = '.'
dqmSaver.tag = "PID%06d" % os.getpid()
dqmSaver.producer = 'Playback'

dqmSystem = cms.untracked.string("playback")
