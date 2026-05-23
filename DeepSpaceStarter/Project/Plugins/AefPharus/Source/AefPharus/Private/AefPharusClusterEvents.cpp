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

void UAefPharusClusterEvents::BroadcastTransformEvent(const FString& EventName,
                                                      const FTransform& Transform,
                                                      bool bPrimaryOnly)
{
#if AEFPHARUS_WITH_DISPLAYCLUSTER
	if (UDisplayClusterBlueprintLib::GetClusterRole() != EDisplayClusterNodeRole::Primary)
	{
		UE_LOG(LogAefPharus, Verbose,
			TEXT("BroadcastTransformEvent('%s') skipped: this node is not Primary"),
			*EventName);
		return;
	}

	FDisplayClusterClusterEventJson Event;
	Event.Name = EventName;
	Event.Parameters.Add(TEXT("Location"), Transform.GetLocation().ToString());
	Event.Parameters.Add(TEXT("Rotation"), Transform.GetRotation().Rotator().ToString());
	Event.Parameters.Add(TEXT("Scale"),    Transform.GetScale3D().ToString());
	UDisplayClusterBlueprintLib::EmitClusterEventJson(Event, bPrimaryOnly);
#else
	UE_LOG(LogAefPharus, Verbose,
		TEXT("BroadcastTransformEvent('%s') no-op: nDisplay unavailable on this platform"),
		*EventName);
#endif
}
