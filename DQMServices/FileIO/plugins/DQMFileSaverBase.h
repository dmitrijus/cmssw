#ifndef DQMSERVICES_COMPONENTS_DQMFILESAVERBASE_H
#define DQMSERVICES_COMPONENTS_DQMFILESAVERBASE_H

#include "FWCore/Framework/interface/global/EDAnalyzer.h"
#include "DQMServices/Core/interface/MonitorElement.h"

#include "EventFilter/Utilities/interface/FastMonitoringService.h"
#include "EventFilter/Utilities/interface/EvFDaqDirector.h"

#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include <sys/time.h>
#include <string>
#include <mutex>

#include <boost/property_tree/ptree.hpp>

namespace dqm {

struct NoCache {};

class DQMFileSaverBase
    : public edm::global::EDAnalyzer<edm::RunCache<NoCache>,
                                     edm::LuminosityBlockCache<NoCache> > {
 public:
  DQMFileSaverBase(const edm::ParameterSet &ps);
  ~DQMFileSaverBase();

 protected:
  virtual void beginJob(void);
  virtual std::shared_ptr<NoCache> globalBeginRun(
      const edm::Run &, const edm::EventSetup &) const;
  virtual std::shared_ptr<NoCache> globalBeginLuminosityBlock(
      const edm::LuminosityBlock &, const edm::EventSetup &) const;
  virtual void analyze(edm::StreamID, const edm::Event &e,
                       const edm::EventSetup &) const;
  virtual void globalEndLuminosityBlock(const edm::LuminosityBlock &,
                                        const edm::EventSetup &) const;
  virtual void globalEndRun(const edm::Run &, const edm::EventSetup &) const;
  virtual void endJob(void);
  virtual void postForkReacquireResources(unsigned int childIndex,
                                          unsigned int numberOfChildren);

  // these two should be overwritten
  // in some cases, hsitograms are deleted after saving
  // so we need to call all file savers
  virtual void saveLumi() const {};
  virtual void saveRun() const {};

  // also used by the JsonWritingTimedPoolOutputModule,
  // fms will be nullptr in such case
  static boost::property_tree::ptree fillJson(
      int run, int lumi, const std::string &dataFilePathName, const std::string transferDestinationStr,
      evf::FastMonitoringService *fms);

  static const std::string fillOrigin(const std::string filename,
                                  const std::string final_filename);

  // utilities
  void logFileAction(const std::string& msg, const std::string& fileName) const;

 protected:
  // file name components, in order
  // lock should be acquired for r/w
  struct FileParameters {
    std::string path_;  //
    std::string producer_;  // DQM or Playback
    int version_;
    std::string tag_;
    long run_;
    long lumi_;
    std::string child_;  // child of a fork

    // other parameters
    DQMStore::SaveReferenceTag saveReference_;
    int saveReferenceQMin_;
  };

  // I have no clue why all framework methods are const.
  mutable std::mutex lock_;
  mutable FileParameters fp_;

  virtual std::string filename(bool useLumi = false) const;

 public:
  static void fillDescription(edm::ParameterSetDescription& d);
};

}  // dqm namespace

#endif  // DQMSERVICES_COMPONENTS_DQMFILESAVERBASE_H
