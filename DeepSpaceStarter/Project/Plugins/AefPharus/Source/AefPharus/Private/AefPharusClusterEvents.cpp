/*========================================================================
   Copyright (c) Ars Electronica Futurelab, 2025

   AefPharus - Cluster Events Shim Implementation
  ========================================================================*/

#include "AefPharusClusterEvents.h"
#include "AefPharus.h"

#if AEFPHARUS_WITH_DISPLAYCLUSTER
	#include "Blueprints/DisplayClusterBlueprintLib.h"
	#include "Cluster/DisplayClusterClusterEvent.h"
	#include "DisplayClusterEnums.h"
#endif

EAefPharusClusterRole UAefPharusClusterEvents::GetClusterRole()
{
#if AEFPHARUS_WITH_DISPLAYCLUSTER
	switch (UDisplayClusterBlueprintLib::GetClusterRole())
	{
		case EDisplayClusterNodeRole::Primary:   return EAefPharusClusterRole::Primary;
		case EDisplayClusterNodeRole::Secondary: return EAefPharusClusterRole::Secondary;
		case EDisplayClusterNodeRole::Backup:    return EAefPharusClusterRole::Backup;
		case EDisplayClusterNodeRole::None:
		default:                                  return EAefPharusClusterRole::None;
	}
#else
	return EAefPharusClusterRole::None;
#endif
}

bool UAefPharusClusterEvents::IsClusterPrimary()
{
	return GetClusterRole() == EAefPharusClusterRole::Primary;
}

void UAefPharusClusterEvents::EmitClusterEventJson(const FAefPharusClusterEventJson& Event,
                                                   bool bPrimaryOnly)
{
#if AEFPHARUS_WITH_DISPLAYCLUSTER
	FDisplayClusterClusterEventJson Real;
	Real.Name       = Event.Name;
	Real.Type       = Event.Type;
	Real.Category   = Event.Category;
	Real.Parameters = Event.Parameters;
	UDisplayClusterBlueprintLib::EmitClusterEventJson(Real, bPrimaryOnly);
#else
	UE_LOG(LogAefPharus, Verbose,
		TEXT("EmitClusterEventJson('%s') no-op: nDisplay unavailable on this platform"),
		*Event.Name);
#endif
}
