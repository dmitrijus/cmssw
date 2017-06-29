#include "DQMStreamStats.h"

#include "FWCore/ServiceRegistry/interface/Service.h"


#include <chrono>
#include <ctime>

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

namespace dqmservices {

DQMStreamStats::DQMStreamStats(edm::ParameterSet const & iConfig) :
    filePrefix_(iConfig.getUntrackedParameter<std::string>("filePrefix", "DQMSTATS_")),
    maxDepth_(iConfig.getUntrackedParameter<int>("maxDepth", -1)),
    dumpOnEndLumi_(iConfig.getUntrackedParameter<bool>("dumpOnEndLumi", false)),
    dumpOnEndJob_(iConfig.getUntrackedParameter<bool>("dumpOnEndJob", false))
    {}

DQMStreamStats::~DQMStreamStats() {}

//Set all possible x dimension parameters
void DQMStreamStats::getDimensionX(Dimension &d, MonitorElement *m){
  d.nBin = m->getNbinsX();
  d.low = m->getTH1()->GetXaxis()->GetXmin();
  d.up = m->getTH1()->GetXaxis()->GetXmax();
  d.mean = m->getTH1()->GetMean();
  d.meanError = m->getTH1()->GetMeanError();
  d.rms = m->getTH1()->GetRMS();
  d.rmsError = m->getTH1()->GetRMSError();
  d.underflow = m->getTH1()->GetBinContent(0);
  d.overflow = m->getTH1()->GetBinContent(d.nBin + 1);
}

void DQMStreamStats::getDimensionY(Dimension &d, MonitorElement *m){
  d.nBin = m->getNbinsY();
  d.low = m->getTH1()->GetYaxis()->GetXmin();
  d.up = m->getTH1()->GetYaxis()->GetXmax();
  d.mean = m->getTH1()->GetMean(2);
  d.meanError = m->getTH1()->GetMeanError(2);
  d.rms = m->getTH1()->GetRMS(2);
  d.rmsError = m->getTH1()->GetRMSError(2);
}

void DQMStreamStats::getDimensionZ(Dimension &d, MonitorElement *m){
  d.nBin = m->getNbinsZ();
  d.low = m->getTH1()->GetZaxis()->GetXmin();
  d.up = m->getTH1()->GetZaxis()->GetXmax();
  d.mean = m->getTH1()->GetMean(3);
  d.meanError = m->getTH1()->GetMeanError(3);
  d.rms = m->getTH1()->GetRMS(3);
  d.rmsError = m->getTH1()->GetRMSError(3);
}

HistoEntry DQMStreamStats::analyze(MonitorElement *m) {
  HistoEntry e;
  e.name = m->getName();
  e.path = m->getFullname();
  e.type = "unknown";

  switch (m->kind()) {
    case MonitorElement::DQM_KIND_INVALID:
      e.type = "INVALID";
      e.bin_size = -1;
      break;
    case MonitorElement::DQM_KIND_INT:
      e.type = "INT";
      e.bin_size = sizeof(int);
      break;
    case MonitorElement::DQM_KIND_REAL:
      e.type = "REAL";
      e.bin_size = sizeof(float);
      break;
    case MonitorElement::DQM_KIND_STRING:
      e.type = "STRING";
      e.bin_size = sizeof(char);
      break;

    // one-dim ME
    case MonitorElement::DQM_KIND_TH1F:
      e.type = "TH1F";
      e.bin_size = sizeof(float);
      getDimensionX(e.dimX, m);
      break;
    case MonitorElement::DQM_KIND_TH1S:
      e.type = "TH1S";
      e.bin_size = sizeof(short);
      getDimensionX(e.dimX, m);
      break;
    case MonitorElement::DQM_KIND_TH1D:
      e.type = "TH1D";
      e.bin_size = sizeof(double);
      getDimensionX(e.dimX, m);
      break;
    case MonitorElement::DQM_KIND_TPROFILE:
      e.type = "TProfile";
      e.bin_size = sizeof(double);
      getDimensionX(e.dimX, m);
      break;

    // two-dim ME
    case MonitorElement::DQM_KIND_TH2F:
      e.type = "TH2F";
      e.bin_size = sizeof(float);
      getDimensionX(e.dimX, m);
      getDimensionY(e.dimY, m);
      break;
    case MonitorElement::DQM_KIND_TH2S:
      e.type = "TH2S";
      e.bin_size = sizeof(short);
      getDimensionX(e.dimX, m);
      getDimensionY(e.dimY, m);
      break;
    case MonitorElement::DQM_KIND_TH2D:
      e.type = "TH2D";
      e.bin_size = sizeof(double);
      getDimensionX(e.dimX, m);
      getDimensionY(e.dimY, m);
      break;
    case MonitorElement::DQM_KIND_TPROFILE2D:
      e.type = "TProfile2D";
      e.bin_size = sizeof(double);
      getDimensionX(e.dimX, m);
      getDimensionY(e.dimY, m);
      break;

    // three-dim ME
    case MonitorElement::DQM_KIND_TH3F:
      e.type = "TH3F";
      e.bin_size = sizeof(float);
      getDimensionX(e.dimX, m);
      getDimensionY(e.dimY, m);
      getDimensionZ(e.dimZ, m);
      break;

    default:
      e.type = "unknown";
      e.bin_size = 0;
  };
  
  // skip "INVALID", "INT", "REAL", "STRING", "unknown"
  if (strcmp(e.type,"INVALID") && strcmp(e.type,"INT") && strcmp(e.type,"REAL") && strcmp(e.type,"STRING") && strcmp(e.type,"unknown")) {
      e.bin_count = m->getTH1()->GetNcells();
      e.entries = m->getEntries();
      e.maxBin = m->getTH1()->GetMaximumBin();
      e.minBin = m->getTH1()->GetMinimumBin();
      e.maxValue = m->getTH1()->GetMaximum();
      e.minValue = m->getTH1()->GetMinimum();
  }

  e.total = e.bin_count * e.bin_size + e.extra;
  e.total_count = 1;

  return e;
}

HistoStats DQMStreamStats::collect(DQMStore::IGetter &iGetter) {
  // new stat frame
  HistoStats stats;

  auto collect_f = [&stats, this](MonitorElement *m) -> void {
    auto frame = this->analyze(m);

    // calculate depth and group if necessary
    int depth = 0;
    bool summary = false;
    std::stringstream spath;

    std::vector<std::string> tokens;
    boost::split(tokens, frame.path, boost::is_any_of("/"));

    for (auto const& token: tokens) {
      spath << ((depth == 0)?"":"/");
      depth++;

      if ((maxDepth_ >= 0) && (depth > maxDepth_)) {
        spath << "_summary";
        summary = true;
        break;
      } else {
        spath << token;
      }
    }

    if (summary) {
      // add it to the summary
      HistoEntry e;
      e.name = "_summary";
      e.path = spath.str();
      e.type = "summary";
      e.total = 0;
      e.total_count = 0;

      auto entry = stats.insert(e);
      entry.first->total += frame.total;
      entry.first->total_count += 1;
    } else {
      stats.insert(frame);
    }
  };

  // correct stream id don't matter - no booking will be done
  iGetter.getAllContents_(collect_f, "");

  return stats;
}

std::string DQMStreamStats::toString(boost::property_tree::ptree doc)
{

    boost::regex exp("\"([0-9]+(\\.[0-9]+)?)\"");
    std::stringstream ss;
    boost::property_tree::json_parser::write_json(ss, doc);
    std::string rv = boost::regex_replace(ss.str(), exp, "$1");

    return rv;
}

void DQMStreamStats::writeMemoryJson(const std::string &fn, const HistoStats &stats) {
  using boost::property_tree::ptree;

  ptree doc;

  doc.put("pid", ::getpid());
  doc.put("n_entries", stats.size());
  doc.put("update_timestamp", std::time(NULL));

  ptree histograms;
  
  ptree info;
  ptree paths;
  ptree totals;
  for (auto &stat : stats) {
    ptree child;
    child.put("", stat.path);
    paths.push_back(std::make_pair("", child));
    child.put("", stat.total);
    totals.push_back(std::make_pair("", child));
  }

  info.add_child("path", paths);
  info.add_child("total", totals);
  histograms.push_back(std::make_pair("", info));

  doc.add_child("histograms", histograms);

  std::ofstream file(fn);
  file << toString(doc);
  file.close();
}

void DQMStreamStats::dqmEndLuminosityBlock(DQMStore::IBooker &,
                                           DQMStore::IGetter &iGetter,
                                           edm::LuminosityBlock const &iLS,
                                           edm::EventSetup const &) {

  if (dumpOnEndLumi_){
    char suffix[64];
    long run  = iLS.id().run();
    long lumi = iLS.id().luminosityBlock();
    long pid = ::getpid();
    sprintf(suffix, "%sENDLUMI_R%09ld_LS%09ld_PID%09ld.json", filePrefix_.c_str(), run, lumi, pid);

    HistoStats st = collect(iGetter);
    std::string fileName = suffix;
    writeMemoryJson(fileName, st);
  }
}


void DQMStreamStats::dqmEndJob(DQMStore::IBooker &iBooker,
                 DQMStore::IGetter &iGetter) {

  if (dumpOnEndJob_){
    char suffix[64];
    long pid = ::getpid();
    sprintf(suffix, "%sENDJOB_PID%09ld.json", filePrefix_.c_str(), pid);

    HistoStats st = collect(iGetter);
    std::string fileName = suffix;
    writeMemoryJson(fileName, st);
  }
}


void DQMStreamStats::fillDescriptions(edm::ConfigurationDescriptions& d) {
  edm::ParameterSetDescription desc;
  desc.setComment("Dumps DQMStore statistics into json files.");

  desc.addUntracked<std::string>("filePrefix", "DQMSTATS_")->setComment("File prefix.");
  desc.addUntracked<int>("maxDepth", -1)->setComment("The depth of subdirectories to dump. -1 means 'dump everything'.");
  desc.addUntracked<bool>("dumpOnEndLumi", false)->setComment("Dump stats at the end of each lumisection.");
  desc.addUntracked<bool>("dumpOnEndJob", false)->setComment("Dump stats at the endjob.");

  d.add("DQMStreamStats", desc);
}

}  // end of namespace

#include "FWCore/Framework/interface/MakerMacros.h"
typedef dqmservices::DQMStreamStats DQMStreamStats;
DEFINE_FWK_MODULE(DQMStreamStats);
