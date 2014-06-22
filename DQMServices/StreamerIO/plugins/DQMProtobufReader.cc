#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/InputSourceMacros.h"
#include "FWCore/Framework/interface/RunPrincipal.h"
#include "FWCore/Framework/interface/LuminosityBlockPrincipal.h"

#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Sources/interface/ProducerSourceBase.h"
#include "FWCore/MessageLogger/interface/JobReport.h"

#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"

#include "DQMFileIterator.h"

namespace edm {
class DQMProtobufReader : public InputSource {

 public:
  explicit DQMProtobufReader(ParameterSet const&,
                             InputSourceDescription const&);
  ~DQMProtobufReader();
  static void fillDescriptions(ConfigurationDescriptions& descriptions);

 private:
  virtual edm::InputSource::ItemType getNextItemType() override;
  virtual boost::shared_ptr<edm::RunAuxiliary> readRunAuxiliary_() override;
  virtual boost::shared_ptr<edm::LuminosityBlockAuxiliary>
      readLuminosityBlockAuxiliary_() override;
  virtual void readRun_(edm::RunPrincipal& rpCache) override;
  virtual void readLuminosityBlock_(edm::LuminosityBlockPrincipal& lbCache)
      override;
  virtual void readEvent_(edm::EventPrincipal&) override;

  void logFileAction(char const* msg, char const* fileName) const;
  bool prepareNextFile();

  bool flagSkipFirstLumis_;
  bool flagEndOfRunKills_;
  bool flagDeleteDatFiles_;

  std::unique_ptr<double> streamReader_;
  DQMFileIterator fiterator_;

  InputSource::ItemType nextItemType;
};

DQMProtobufReader::DQMProtobufReader(ParameterSet const& pset,
                                     InputSourceDescription const& desc)
    : InputSource(pset, desc), fiterator_(pset) {

  flagSkipFirstLumis_ = pset.getUntrackedParameter<bool>("skipFirstLumis");
  flagEndOfRunKills_ = pset.getUntrackedParameter<bool>("endOfRunKills");
  flagDeleteDatFiles_ = pset.getUntrackedParameter<bool>("deleteDatFiles");
}

DQMProtobufReader::~DQMProtobufReader() {}

InputSource::ItemType DQMProtobufReader::getNextItemType() {
  typedef DQMFileIterator::State State;
  typedef DQMFileIterator::LumiEntry LumiEntry;

  fiterator_.logFileAction("getNextItemType");

  for (;;) {
    // check for end of run file and force quit
    if (flagEndOfRunKills_ && (fiterator_.state() != State::OPEN)) {
      return InputSource::IsStop;
    }

    // check for end of run and quit if everything has been processed.
    // this is the clean exit
    if ((!fiterator_.hasNext()) &&
        (fiterator_.state() == State::EOR)) {

      return InputSource::IsStop;
    }

    // skip to the next file if we have no files openned yet
    if (fiterator_.hasNext()) {
      const LumiEntry& lumi = fiterator_.front();
      std::string p = fiterator_.make_path_data(lumi);

      if (boost::filesystem::exists(p)) {
        //fiterator_.logFileAction("Initiating request to open file ", p);
        return InputSource::IsLumi;
      } else {
        fiterator_.logFileAction("Data file is missing ", p);
        fiterator_.pop();
        continue;
      }
    }

    fiterator_.delay();
    return InputSource::IsSynchronize;
  }

  // this is unreachable
}

boost::shared_ptr<edm::RunAuxiliary> DQMProtobufReader::readRunAuxiliary_() {
  fiterator_.logFileAction("readRunAuxiliary_");

  edm::RunAuxiliary *aux = new edm::RunAuxiliary(fiterator_.runNumber(),
    edm::Timestamp(),
    edm::Timestamp()
  );
  return boost::shared_ptr<edm::RunAuxiliary>(aux);
};

boost::shared_ptr<edm::LuminosityBlockAuxiliary>
DQMProtobufReader::readLuminosityBlockAuxiliary_() {
  fiterator_.logFileAction("readLuminosityBlockAuxiliary_");

  edm::LuminosityBlockAuxiliary *aux = new edm::LuminosityBlockAuxiliary(
    fiterator_.runNumber(),
    fiterator_.front().ls,
    edm::Timestamp(),
    edm::Timestamp()
  );

  return std::shared_ptr<edm::LuminosityBlockAuxiliary>(aux);
};

void DQMProtobufReader::readRun_(edm::RunPrincipal& rpCache) {
  fiterator_.logFileAction("readRun_");

  rpCache.fillRunPrincipal(processHistoryRegistryForUpdate());
}

void DQMProtobufReader::readLuminosityBlock_(
    edm::LuminosityBlockPrincipal& lbCache) {

  fiterator_.logFileAction("readLuminosityBlock_");
  edm::Service<DQMStore> store;

  const DQMFileIterator::LumiEntry& lumi = fiterator_.front();
  std::string p = fiterator_.make_path_data(lumi);

  fiterator_.logFileAction("Initiating request to open file ", p);
  fiterator_.logFileAction("Successfully opened file ", p);
  store->load(p);
  fiterator_.logFileAction("Closed file ", p);
  fiterator_.pop();

  edm::Service<edm::JobReport> jr;
  jr->reportInputLumiSection(lbCache.id().run(),lbCache.id().luminosityBlock());
  lbCache.fillLuminosityBlockPrincipal(processHistoryRegistryForUpdate());
};

void DQMProtobufReader::readEvent_(edm::EventPrincipal&) { };

void DQMProtobufReader::fillDescriptions(
    ConfigurationDescriptions& descriptions) {
  ParameterSetDescription desc;
  desc.setComment("Creates runs and lumis and fills the dqmstore from protocol buffer files.");
  ProducerSourceBase::fillDescription(desc);

   desc.addUntracked<bool>("skipFirstLumis", false)->setComment(
      "Skip (and ignore the minEventsPerLumi parameter) for the files "
      "which have been available at the begining of the processing. "
      "If set to true, the reader will open last available file for "
      "processing.");

  desc.addUntracked<bool>("deleteDatFiles", false)->setComment(
      "Delete data files after they have been closed, in order to "
      "save disk space.");

  desc.addUntracked<bool>("endOfRunKills", false)->setComment(
      "Kill the processing as soon as the end-of-run file appears, even if "
      "there are/will be unprocessed lumisections.");

  DQMFileIterator::fillDescription(desc);
  descriptions.add("source", desc);
}
}

using edm::DQMProtobufReader;
DEFINE_FWK_INPUT_SOURCE(DQMProtobufReader);
