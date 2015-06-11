#ifndef DQMSERVICES_COMPONENTS_DQMFILESAVEROUTPUT_H
#define DQMSERVICES_COMPONENTS_DQMFILESAVEROUTPUT_H

#include "FWCore/Framework/interface/global/EDAnalyzer.h"
#include "DQMServices/Core/interface/MonitorElement.h"

#include <sys/time.h>
#include <string>
#include <mutex>

#include "DQMFileSaverBase.h"

namespace dqm {

class DQMFileSaverOnline : public DQMFileSaverBase {
 public:
  DQMFileSaverOnline(const edm::ParameterSet &ps);
  ~DQMFileSaverOnline();

 protected:
  void saveLumi() const;
  void saveRun() const;

 protected:
  std::string outputPrefix_;
  int backupLumi_;

  mutable int lumiIndex_;

  // snapshot making
  struct SnapshotFile {
    std::string data;
    std::string meta;
  };

  void makeSnapshot(bool final) const;
  mutable SnapshotFile currentSnapshot_;

  void checkError(const char *msg, int status) const;

 public:
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
};

}  // dqm namespace

#endif  // DQMSERVICES_COMPONENTS_DQMFILESAVEROUTPUT_H
