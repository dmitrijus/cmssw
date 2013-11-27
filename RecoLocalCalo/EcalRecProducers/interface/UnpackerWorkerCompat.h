#ifndef RecoLocalCalo_EcalRecProducers_UnpackerWorkerCompat_hh
#define RecoLocalCalo_EcalRecProducers_UnpackerWorkerCompat_hh

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/PluginManager/interface/PluginFactory.h"

#include "RecoLocalCalo/EcalRecProducers/plugins/EcalUncalibRecHitProducerBase.h"

typedef edmplugin::PluginFactory<EcalUncalibRecHitProducerBase*(const edm::ParameterSet&) > EcalUncalibRecHitWorkerFactory;

#endif
