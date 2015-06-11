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

#include "DQMFileSaverOnline.h"

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

DQMFileSaverOnline::DQMFileSaverOnline(const edm::ParameterSet &ps)
    : DQMFileSaverBase(ps) {
  backupLumi_ = 1;
  lumiIndex_ = 0;
}

DQMFileSaverOnline::~DQMFileSaverOnline() {}

void DQMFileSaverOnline::saveLumi() const {
  lumiIndex_++;
  if (lumiIndex_ >= backupLumi_) {
    lumiIndex_ = 0;

    // actual saving is done here
    makeSnapshot(false);
  }
}

void DQMFileSaverOnline::saveRun() const {
  makeSnapshot(true);

  // do the final rename
  //std::string prefix = filename(false);
  //std::string root_fp = prefix + ".root";
  //std::string meta_fp = prefix + ".output";

  //logFileAction("Final DQM Root file: ", root_fp);
  //logFileAction("Final DQM Origin file: ", meta_fp);

  //checkError("rename failed", ::rename(currentSnapshot_.data.c_str(), root_fp.c_str()));
  //checkError("rename failed", ::rename(currentSnapshot_.meta.c_str(), meta_fp.c_str()));
}

void DQMFileSaverOnline::makeSnapshot(bool final) const {
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
  if (!final) {
    snprintf(suffix, 127, ".ls%08ld_host%s_pid%08d", fp_.lumi_, hostname, pid);
  } else {
    suffix[0] = 0;
  }

  std::string prefix = filename(false);

  std::string root_fp = prefix + ".root" + suffix;
  std::string meta_fp = prefix + ".root.origin" + suffix;

  std::string tmp_root_fp = root_fp + ".tmp";
  std::string tmp_meta_fp = meta_fp + ".tmp";

  // run_ and lumi_ are ignored if dqmstore is not in multithread mode
  edm::Service<DQMStore> store;

  logFileAction("Writing DQM Root file: ", root_fp);
  logFileAction("Writing DQM Origin file: ", meta_fp);

  store->save(tmp_root_fp,                       /* filename      */
              "",                                /* path          */
              "^(Reference/)?([^/]+)",           /* pattern       */
              rewrite,                           /* rewrite       */
              store->mtEnabled() ? fp_.run_ : 0, /* run           */
              0,                                 /* lumi          */
              fp_.saveReference_,                /* ref           */
              fp_.saveReferenceQMin_,            /* ref minStatus */
              "RECREATE",                        /* fileupdate    */
              false                              /* resetMEs      */
              );

  // write metadata
  // format.origin: md5:d566a34b27f48d507150a332b189398b 294835 /home/dqmprolocal/output/DQM_V0001_FED_R000194224.root
  std::ofstream fp(tmp_meta_fp);
  fp << this->fillOrigin(tmp_root_fp, root_fp);
  fp.close();

  checkError("rename failed", ::rename(tmp_root_fp.c_str(), root_fp.c_str()));
  checkError("rename failed", ::rename(tmp_meta_fp.c_str(), meta_fp.c_str()));

  SnapshotFile old = currentSnapshot_;
  currentSnapshot_ = { root_fp, meta_fp };

  // clear old snapshot
  if (!old.data.empty()) {
    logFileAction("Deleting old snapshot (root): ", old.data);
    checkError("unlink failed", ::unlink(old.data.c_str()));
  }

  if (!old.meta.empty()) {
    logFileAction("Deleting old snapshot (origin): ", old.meta);
    checkError("unlink failed", ::unlink(old.meta.c_str()));
  }
}

void DQMFileSaverOnline::checkError(const char *msg, int status) const {
  if (status != 0) {
    ::perror(msg);
  }
}

void DQMFileSaverOnline::fillDescriptions(
    edm::ConfigurationDescriptions& descriptions) {

  edm::ParameterSetDescription desc;
  desc.setComment("Saves histograms from DQM store, online workflow.");

  desc.addUntracked<int>("backupLumiCount", 10)->setComment(
      "How often the backup file will be generated, in lumisections.");

  DQMFileSaverBase::fillDescription(desc);

  descriptions.add("saver", desc);
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(DQMFileSaverOnline);
