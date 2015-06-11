#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Version/interface/GetReleaseVersion.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/MessageLogger/interface/JobReport.h"

#include "DQMFileSaverOutput.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <utility>
#include <TString.h>
#include <TSystem.h>

using namespace dqm;

DQMFileSaverOutput::DQMFileSaverOutput(const edm::ParameterSet &ps)
    : DQMFileSaverBase(ps) {
  backupLumi_ = 10;
  lumiIndex_ = 0;
}

DQMFileSaverOutput::~DQMFileSaverOutput() {}

void DQMFileSaverOutput::globalEndLuminosityBlock(
    const edm::LuminosityBlock &iLS, const edm::EventSetup &) const {
  lumiIndex_++;
  if (lumiIndex_ >= backupLumi_) {
    lumiIndex_ = 0;

    // actual saving is done here
    makeSnapshot();
  }

  // post lumi actions
}

void DQMFileSaverOutput::globalEndRun(const edm::Run &iRun,
                                      const edm::EventSetup &) const {
  makeSnapshot();
}

void DQMFileSaverOutput::makeSnapshot() const {
  std::unique_lock<std::mutex> lck(lock_);

  char rewrite[128];
  if (!outputPrefix_.empty()) {
    snprintf(rewrite, 128, "\\1Run %ld/\\2/Run summary/%s", fp_.run_,
             outputPrefix_.c_str());
  } else {
    snprintf(rewrite, 128, "\\1Run %ld/\\2/Run summary", fp_.run_);
  }

  int pid = getpid();
  char hostname[64];
  gethostname(hostname, 64);
  hostname[63] = 0;

  char suffix[128];
  snprintf(suffix, 128, "ls%08ld_host%s_pid%08d", fp_.lumi_, hostname, pid);

  std::string prefix = basename(false);

  std::string root_fp = prefix + ".root." + suffix;
  std::string meta_fp = prefix + ".output." + suffix;

  std::string tmp_root_fp = root_fp + ".tmp";
  std::string tmp_meta_fp = meta_fp + ".tmp";

  // run_ and lumi_ are ignored if dqmstore is not in multithread mode
  edm::Service<DQMStore> store;
  store->save(tmp_root_fp,                       /* filename      */
              "",                                /* path          */
              "^(Reference/)?([^/]+)",           /* pattern       */
              rewrite,                           /* rewrite       */
              store->mtEnabled() ? fp_.run_ : 0, /* run           */
              0,                                 /* lumi          */
              DQMStore::SaveWithReference,       /* ref           */
              dqm::qstatus::STATUS_OK,           /* ref minStatus */
              "RECREATE",                        /* fileupdate    */
              false                              /* resetMEs      */
              );

  SnapshotFile old = currentSnapshot_;
  currentSnapshot_ = { tmp_root_fp, tmp_meta_fp };
  // write metadata
  // clear old snapshot
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(DQMFileSaverOutput);
