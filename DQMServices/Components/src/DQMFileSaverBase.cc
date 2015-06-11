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

#include "DQMFileSaverBase.h"

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

DQMFileSaverBase::DQMFileSaverBase(const edm::ParameterSet &ps) {
  std::unique_lock<std::mutex> lck(lock_);
  fp_.base_ = "DQM";
  fp_.run_ = 0;
  fp_.tag_ = "NOTSET";
  fp_.lumi_ = 0;
  fp_.version_ = 1;
  fp_.child_ = "";
}

DQMFileSaverBase::~DQMFileSaverBase() {}

// void
// DQMFileSaverBase::saveJobReport(const std::string &filename) const
//{
//
//  // Report the file to job report service.
//  edm::Service<edm::JobReport> jr;
//  if (jr.isAvailable())
//  {
//    std::map<std::string, std::string> info;
//    info["Source"] = "DQMStore";
//    info["FileClass"] = "DQM";
//    jr->reportAnalysisFile(filename, info);
//  }
//
//}

void DQMFileSaverBase::beginJob() {}

std::shared_ptr<NoCache> DQMFileSaverBase::globalBeginRun(
    const edm::Run &r, const edm::EventSetup &) const {
  std::unique_lock<std::mutex> lck(lock_);
  fp_.run_ = r.id().run();

  return nullptr;
}

std::shared_ptr<NoCache> DQMFileSaverBase::globalBeginLuminosityBlock(
    const edm::LuminosityBlock &l, const edm::EventSetup &) const {
  std::unique_lock<std::mutex> lck(lock_);
  fp_.lumi_ = l.id().luminosityBlock();

  return nullptr;
}

void DQMFileSaverBase::analyze(edm::StreamID, const edm::Event &e,
                               const edm::EventSetup &) const {
  // not supported
}

void DQMFileSaverBase::globalEndLuminosityBlock(const edm::LuminosityBlock &iLS,
                                                const edm::EventSetup &) const {
  // empty
}

void DQMFileSaverBase::globalEndRun(const edm::Run &iRun,
                                    const edm::EventSetup &) const {
  // empty
}

void DQMFileSaverBase::endJob(void) {}

void DQMFileSaverBase::postForkReacquireResources(
    unsigned int childIndex, unsigned int numberOfChildren) {
  // this is copied from IOPool/Output/src/PoolOutputModule.cc, for consistency
  unsigned int digits = 0;
  while (numberOfChildren != 0) {
    ++digits;
    numberOfChildren /= 10;
  }
  // protect against zero numberOfChildren
  if (digits == 0) {
    digits = 3;
  }

  char buffer[digits + 2];
  snprintf(buffer, digits + 2, "_%0*d", digits, childIndex);

  std::unique_lock<std::mutex> lck(lock_);
  fp_.child_ = std::string(buffer);
}

std::string DQMFileSaverBase::basename(bool useLumi) const {
  std::unique_lock<std::mutex> lck(lock_);

  char buf[256];
  if (useLumi) {
    snprintf(buf, 256, "%s_V%04d_%s_R%09ld_L%09ld%s", fp_.base_.c_str(),
             fp_.version_, fp_.tag_.c_str(), fp_.run_, fp_.lumi_,
             fp_.child_.c_str());
  } else {
    snprintf(buf, 256, "%s_V%04d_%s_R%09ld%s", fp_.base_.c_str(), fp_.version_,
             fp_.tag_.c_str(), fp_.run_, fp_.child_.c_str());
  }
  buf[255] = 0;

  return std::string(buf);
}
