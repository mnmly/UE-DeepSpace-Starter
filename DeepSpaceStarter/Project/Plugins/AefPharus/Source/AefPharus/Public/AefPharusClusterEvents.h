/*========================================================================
   Copyright (c) Ars Electronica Futurelab, 2025

   AefPharus - Cluster Events Shim

   Cross-platform Blueprint API that mirrors a small subset of
   UDisplayClusterBlueprintLib so blueprints can call cluster operations
   without taking a hard dependency on the nDisplay module. On Win64 and
   Linux it forwards to the real DisplayCluster implementation; on Mac
   (or any platform where nDisplay is unavailable) it returns safe
   defaults and the calls become no-ops.

   Equivalent of the project's BP_DeepSpace_Library function
   "DeepSpace Cluster Spawn", re-expressed in C++ so the same graph can
   compile on every supported target.
  ========================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AefPharusClusterEvents.generated.h"

/** Mirror of EDisplayClusterNodeRole so blueprints don't import nDisplay types. */
UENUM(BlueprintType)
enum class EAefPharusClusterRole : uint8
{
	None      UMETA(DisplayName = "None"),
	Primary   UMETA(DisplayName = "Primary"),
	Secondary UMETA(DisplayName = "Secondary"),
	Backup    UMETA(DisplayName = "Backup"),
};

UCLASS()
class AEFPHARUS_API UAefPharusClusterEvents : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Current cluster node role.
	 * Win64/Linux: forwards to UDisplayClusterBlueprintLib::GetClusterRole.
	 * Mac: always returns None.
	 */
	UFUNCTION(BlueprintPure, Category = "AEF|Cluster")
	static EAefPharusClusterRole GetClusterRole();

	/** Convenience: true if GetClusterRole() == Primary. */
	UFUNCTION(BlueprintPure, Category = "AEF|Cluster")
	static bool IsClusterPrimary();

	/**
	 * Broadcast a named transform event across the cluster as
	 * DisplayClusterClusterEventJson with Parameters keys
	 * "Location" / "Rotation" / "Scale".
	 *
	 * Only runs on the Primary node (matches the original BP behavior of
	 * gating on GetClusterRole). On non-Primary nodes, on standalone, or
	 * on platforms without nDisplay, this is a no-op (logged at Verbose).
	 *
	 * @param EventName    Event name written into the JSON payload.
	 * @param Transform    Transform whose components are stringified into Parameters.
	 * @param bPrimaryOnly Forwarded to EmitClusterEventJson.
	 */
	UFUNCTION(BlueprintCallable, Category = "AEF|Cluster")
	static void BroadcastTransformEvent(const FString& EventName,
	                                    const FTransform& Transform,
	                                    bool bPrimaryOnly = false);
};
