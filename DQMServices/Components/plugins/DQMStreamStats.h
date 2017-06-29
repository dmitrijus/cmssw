#ifndef DQMStreamStats_H
#define DQMStreamStats_H

/** \class DQMStreamStats
 * *
 *  DQM Store Stats - new version, for multithreaded framework
 *
 *  \author Dmitrijus Bugelskis CERN
 */

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <utility>
#include <fstream>
#include <sstream>

#include "FWCore/Framework/interface/stream/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/DQMEDHarvester.h"
#include "DQMServices/Core/interface/MonitorElement.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace dqmservices {

struct Node{
  std::string name;
  long size;
  std::string parent;
  std::vector<Node> children;
};

struct Dimension{
  int nBin = 0;
  double low = 0, up = 0;
  double mean = 0, meanError = 0; 
  double rms = 0, rmsError = 0; 
  double underflow = 0, overflow = 0;  
};

class HistoEntry {
 public:
  std::string path;
  std::string name;

  const char *type;
  size_t bin_count = 0;
  size_t bin_size = 0;
  size_t extra = 0;

  mutable size_t total = 0;
  mutable size_t total_count = 0;

  double entries = 0;
  int maxBin = 0, minBin = 0;
  double maxValue = 0, minValue = 0;
  Dimension dimX, dimY, dimZ; 

  bool operator<(const HistoEntry &rhs) const { return path < rhs.path; }
};

typedef std::set<HistoEntry> HistoStats;

class DQMStreamStats : public DQMEDHarvester {
 public:
  DQMStreamStats(edm::ParameterSet const & iConfig);
  virtual ~DQMStreamStats();

  void dqmEndLuminosityBlock(DQMStore::IBooker &, DQMStore::IGetter &,
                             edm::LuminosityBlock const &,
                             edm::EventSetup const &) override;

  void dqmEndJob(DQMStore::IBooker &iBooker,
                 DQMStore::IGetter &iGetter) override;

  static void fillDescriptions(edm::ConfigurationDescriptions &);

 protected:
  HistoStats collect(DQMStore::IGetter &iGetter);
  std::string toString(boost::property_tree::ptree doc);
  void writeMemoryJson(const std::string &fn, const HistoStats &stats);

  // analyze a single monitor element
  HistoEntry analyze(MonitorElement *m);

 private:
  void getDimensionX(Dimension &d, MonitorElement *m);
  void getDimensionY(Dimension &d, MonitorElement *m);
  void getDimensionZ(Dimension &d, MonitorElement *m);

  std::string filePrefix_;
  int maxDepth_;
  bool dumpOnEndLumi_;
  bool dumpOnEndJob_;
};

}  // end of namespace
#endif
