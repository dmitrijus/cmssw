import os
import FWCore.ParameterSet.Config as cms
import ConfigParser

def loadDQMRunConfigFromFile():
    # try reading the config
    conf_locations = [
        "/etc/dqm_run_config",
        os.path.expanduser("~/.dqm_run_config"),
        os.path.join(os.curdir, "dqm_run_config"),
    ]
    
    config = ConfigParser.ConfigParser()
    files_read = config.read(conf_locations)

    print "Loaded configuration file from:", files_read
    return config

# default values, config in file overrides parts of it
# dqmEnv and dqmSaver will configure from this pset
dqmRunConfig = cms.PSet(
    type = cms.untracked.string("userarea"),
    collectorPort = cms.untracked.int32(9190),
    collectorHost = cms.untracked.string('dqm-c2d07-29.cms'),
)

# type should be loaded first, to populate the proper defaults
dqmFileConfig = loadDQMRunConfigFromFile()
if dqmFileConfig.has_option("host", "type"):
    dqmRunConfig.type = dqmFileConfig.get("host", "type")

# once the host name is set, populate it with type defaults
# ie dqm gui collector host goes here
if dqmRunConfig.type.value() == "playback":
    collectorPort = cms.untracked.int32(9090)
    collectorHost = cms.untracked.string('dqm-c2d07-11.cms')
elif dqmRunConfig.type.value() == "production":
    collectorPort = cms.untracked.int32(9090)
    collectorHost = cms.untracked.string('dqm-prod-local.cms')

# load the options from the config file, if set
if dqmFileConfig.has_option("host", "collectorPort"):
    dqmRunConfig.type = int(dqmFileConfig.get("host", "collectorPort"))

if dqmFileConfig.has_option("host", "collectorHost"):
    dqmRunConfig.type = dqmFileConfig.get("host", "collectorHost")


# now start the actual configuration
print "dqmRunConfig:", dqmRunConfig

from DQMServices.Core.DQMStore_cfi import *

DQM = cms.Service("DQM",
                  debug = cms.untracked.bool(False),
                  publishFrequency = cms.untracked.double(5.0),
                  collectorPort = dqmRunConfig.collectorPort,
                  collectorHost = dqmRunConfig.collectorHost,
                  filter = cms.untracked.string(''),
)

DQMMonitoringService = cms.Service("DQMMonitoringService")

from DQMServices.Components.DQMEventInfo_cfi import *
from DQMServices.FileIO.DQMFileSaverOnline_cfi import *

# upload should be either a directory or a symlink for dqm gui destination
dqmSaver.path = "./upload" 
dqmSaver.tag = "PID%06d" % os.getpid()
dqmSaver.producer = 'DQM'
dqmSaver.backupLumiCount = 15
