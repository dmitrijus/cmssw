#include "DQM/PhysicsHWW/interface/monitor.h"
#include "FWCore/Utilities/interface/Exception.h"

EventMonitor::Entry::Entry()
{
  for (unsigned int i=0; i<5; ++i){
    nevt[i] = 0;
  }
}

void EventMonitor::hypo_monitor::addCounter(const char* name)
{
  auto counter = counters_.find(name);
  if (counter == counters_.end()) {
    Entry entry;
    entry.insert_order = counters_.size();

    counters_[name] = entry;
  } else {
    throw cms::Exception("counterAlreadyExists") << "Name: " << name;
  }
}

void EventMonitor::hypo_monitor::count(HypothesisType type, const char* name, double weight)
{
  auto counter = counters_.find(name);
  if (counter != counters_.end()) {
    counter->second.nevt[type]++;
    counter->second.nevt[ALL]++;
  } else {
    throw cms::Exception("counterNotFound") << "Name: " << name;
  }
}


EventMonitor::EventMonitor()
{
  //Constructor sets order of selections in the monitor.
  //This ensures that the selections in the dqm histograms 
  //will always be in the same order.
  monitor.addCounter("total events"                              );
  monitor.addCounter("baseline"                                  );
  monitor.addCounter("opposite sign"                             );
  monitor.addCounter("full lepton selection"                     );
  monitor.addCounter("extra lepton veto"                         );
  monitor.addCounter("met > 20 GeV"                              );
  monitor.addCounter("mll > 12 GeV"                              );
  monitor.addCounter("|mll - mZ| > 15 GeV"                       );
  monitor.addCounter("minMET > 20 GeV"                           );
  monitor.addCounter("minMET > 40 GeV for ee/mm"                 );
  monitor.addCounter("dPhiDiLepJet < 165 dg for ee/mm"           );
  monitor.addCounter("SoftMuons==0"                              );
  monitor.addCounter("top veto"                                  );
  monitor.addCounter("ptll > 45 GeV"                             );
  monitor.addCounter("njets == 0"                                );
  monitor.addCounter("max(lep1.pt(),lep2.pt())>30"               );
  monitor.addCounter("min(lep1.pt(),lep2.pt())>25"               );
  monitor.addCounter("njets == 1"                                );
  monitor.addCounter("njets == 2 or 3"                           );
  monitor.addCounter("abs(jet1.eta())<4.7 && abs(jet2.eta())<4.7");
  monitor.addCounter("no central jets"                           );
}
