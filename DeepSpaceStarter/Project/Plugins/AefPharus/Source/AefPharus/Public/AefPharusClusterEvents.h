/*========================================================================
   Copyright (c) Ars Electronica Futurelab, 2025

   AefPharus - Cluster Events Shim

   Cross-platform mirror of the small subset of nDisplay's DisplayCluster
   surface that BP_DeepSpace_Library uses. Names and field layouts match
   the originals on purpose so the project's blueprint can be redirected
   here via `bpx ref rewrite` and load on every platform — including Mac
   where nDisplay is unavailable.

   On Win64/Linux: EmitClusterEventJson converts our stub struct into a
   real FDisplayClusterClusterEventJson and forwards to the actual
   UDisplayClusterBlueprintLib so live cluster behavior is preserved.

   On Mac: EmitClusterEventJson logs at Verbose and returns; GetClusterRole
   returns None. No cluster behavior, but the BP compiles and runs.
  ========================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AefPharusClusterEvents.generated.h"

/** Mirror of EDisplayClusterNodeRole. Same value ordering. */
UENUM(BlueprintType)
enum class EAefPharusClusterRole : uint8
{
	None      UMETA(DisplayName = "None"),
	Primary   UMETA(DisplayName = "Primary"),
	Secondary UMETA(DisplayName = "Secondary"),
	Backup    UMETA(DisplayName = "Backup"),
};

/** Mirror of FDisplayClusterClusterEventBase. Intentionally empty. */
USTRUCT(BlueprintType)
struct AEFPHARUS_API FAefPharusClusterEventBase
{
	GENERATED_BODY()
};

/**
 * Mirror of FDisplayClusterClusterEventJson with the same BP-facing fields.
 * Used as the payload type for cross-platform cluster event emission.
 */
USTRUCT(BlueprintType)
struct AEFPHARUS_API FAefPharusClusterEventJson : public FAefPharusClusterEventBase
{
	GENERATED_BODY()

	/** Event name (used for discarding outdated events). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NDisplay")
	FString Name;

	/** Event type (used for discarding outdated events). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NDisplay")
	FString Type;

	/** Event category (used for discarding outdated events). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NDisplay")
	FString Category;

	/** Event parameters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NDisplay")
	TMap<FString, FString> Parameters;
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
	 * Emit a JSON cluster event. Drop-in mirror of
	 * UDisplayClusterBlueprintLib::EmitClusterEventJson; the function and
	 * struct names match so BPs can be redirected via bpx ref rewrite.
	 *
	 * Win64/Linux: forwards to the real implementation.
	 * Mac: logs at Verbose and returns.
	 */
	UFUNCTION(BlueprintCallable, Category = "AEF|Cluster")
	static void EmitClusterEventJson(const FAefPharusClusterEventJson& Event,
	                                 bool bPrimaryOnly);
};
