#ifndef DQMSERVICES_COMPONENTS_DQMFILESAVEROUTPUT_H
#define DQMSERVICES_COMPONENTS_DQMFILESAVEROUTPUT_H

#include "FWCore/Framework/interface/global/EDAnalyzer.h"
#include "DQMServices/Core/interface/MonitorElement.h"

#include <sys/time.h>
#include <string>
#include <mutex>

#include "DQMFileSaverBase.h"

namespace dqm {

class DQMFileSaverOutput : public DQMFileSaverBase {
 public:
  DQMFileSaverOutput(const edm::ParameterSet &ps);
  ~DQMFileSaverOutput();

 protected:
  virtual void globalEndLuminosityBlock(const edm::LuminosityBlock &,
                                        const edm::EventSetup &) const;
  virtual void globalEndRun(const edm::Run &, const edm::EventSetup &) const;

 protected:
  std::string outputPrefix_;
  int backupLumi_;

  mutable int lumiIndex_;

  // snapshot making
  struct SnapshotFile {
    std::string data;
    std::string meta;
  };

  void makeSnapshot() const;
  mutable SnapshotFile currentSnapshot_;
};

}  // dqm namespace

#endif  // DQMSERVICES_COMPONENTS_DQMFILESAVEROUTPUT_H
