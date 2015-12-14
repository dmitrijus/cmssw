#include "DQMProtobufReader.h"

#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"

namespace dqmservices {

class DQMProtorootReader : public DQMProtobufReader {
 public:
  explicit DQMProtorootReader(edm::ParameterSet const&,
                             edm::InputSourceDescription const&);
  ~DQMProtorootReader();

 protected:
  // override this in sub classes, ie BRIL source
  virtual void loadDataFile(const std::string& path) override;
};

DQMProtorootReader::DQMProtorootReader(edm::ParameterSet const& pset,
                                     edm::InputSourceDescription const& desc)
    : DQMProtobufReader(pset, desc) {

    // pass
}

DQMProtorootReader::~DQMProtorootReader() {}

void DQMProtorootReader::loadDataFile(const std::string& path) {
  edm::Service<DQMStore> store;
  store->load(path);
};


}  // end of namespace

#include "FWCore/Framework/interface/InputSourceMacros.h"
#include "FWCore/Framework/interface/MakerMacros.h"

using dqmservices::DQMProtorootReader;
DEFINE_FWK_INPUT_SOURCE(DQMProtorootReader);
