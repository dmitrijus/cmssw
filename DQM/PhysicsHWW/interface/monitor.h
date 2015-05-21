#ifndef WW_monitor_h
#define WW_monitor_h
#include <vector>
#include <string>
#include "wwtypes.h"

using namespace HWWFunctions;


class EventMonitor
{
public:

  EventMonitor();

  struct Entry {
    unsigned int nevt[5];
    std::string name;
    int insert_order;

    Entry();
  };

  struct hypo_monitor {
    hypo_monitor(){}
    void addCounter(const char* name);
    void count(HypothesisType type, const char* name, double weight=1.0);

    std::map<std::string, EventMonitor::Entry> counters_;
  };

  hypo_monitor monitor;

};
#endif
