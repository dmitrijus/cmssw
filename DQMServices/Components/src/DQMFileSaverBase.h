#ifndef DQMSERVICES_COMPONENTS_DQMFILESAVERBASE_H
#define DQMSERVICES_COMPONENTS_DQMFILESAVERBASE_H

#include "FWCore/Framework/interface/global/EDAnalyzer.h"
#include "DQMServices/Core/interface/MonitorElement.h"

#include <sys/time.h>
#include <string>
#include <mutex>

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

 protected:
  // file name components, in order
  // lock should be acquired for r/w
  struct Filename {
    std::string base_;  // DQM or Playback
    int version_;
    std::string tag_;
    long run_;
    long lumi_;
    std::string child_;  // child of a fork
  };

  // I have no clue why all framework methods are const.
  mutable std::mutex lock_;
  mutable Filename fp_;

  virtual std::string basename(bool useLumi = false) const;

 protected:
  std::string writeMetadataOrigin(const std::string filename,
                                  const std::string final_filename);
};

}  // dqm namespace

#endif  // DQMSERVICES_COMPONENTS_DQMFILESAVERBASE_H
