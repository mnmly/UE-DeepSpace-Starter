/*========================================================================
   Copyright (c) Ars Electronica Futurelab, 2025

   AefPharus - DisplayCluster Class Stubs

   Empty shadow UCLASSes that share names with the real nDisplay classes,
   declared in /Script/AefPharus so cluster assets (DisplayClusterBlueprint-
   based BPs) can deserialize on Mac where the nDisplay engine plugin isn't
   built. The asset's import table is rewritten to point at /Script/AefPharus
   instead of /Script/DisplayCluster; on Win/Linux a [CoreRedirects] rule
   bounces those references back to the real classes at load time, so the
   stubs are dead weight there (UHT requires UCLASS at file scope, so we
   can't ifdef them out per-platform).

   UPROPERTY layout is the minimum required for the project's cluster BP
   (nDisplay_Deep_Space_8K) to compile: BP graphs reference
   bPreviewEnable / bFollowLocalPlayerCamera / CurrentConfigData on the
   root actor. Config UObjects are empty; their serialized fields don't
   need to round-trip on Mac (we never run a real cluster there).
  ========================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#if WITH_EDITORONLY_DATA
#include "Engine/Blueprint.h"
#endif

#include "AefPharusDisplayClusterStubs.generated.h"

class UDisplayClusterConfigurationData;

// -- Components ----------------------------------------------------------

UCLASS()
class AEFPHARUS_API UDisplayClusterCameraComponent : public USceneComponent
{
	GENERATED_BODY()
};

UCLASS()
class AEFPHARUS_API UDisplayClusterDisplayDeviceComponent : public UActorComponent
{
	GENERATED_BODY()
};

UCLASS()
class AEFPHARUS_API UDisplayClusterOriginComponent : public USceneComponent
{
	GENERATED_BODY()
};

UCLASS()
class AEFPHARUS_API UDisplayClusterScreenComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
#if WITH_EDITORONLY_DATA
public:
	/** Mirror of engine's Size UPROPERTY so SCS archetype data round-trips. */
	UPROPERTY(EditDefaultsOnly, Category = "Screen Size")
	FVector2D Size = FVector2D::ZeroVector;
#endif
};

UCLASS()
class AEFPHARUS_API UDisplayClusterStageGeometryComponent : public UActorComponent
{
	GENERATED_BODY()
};

UCLASS()
class AEFPHARUS_API UDisplayClusterStageIsosphereComponent : public USceneComponent
{
	GENERATED_BODY()
};

UCLASS()
class AEFPHARUS_API UDisplayClusterSyncTickComponent : public UActorComponent
{
	GENERATED_BODY()
};

UCLASS()
class AEFPHARUS_API UDisplayClusterXformComponent : public USceneComponent
{
	GENERATED_BODY()
};

// -- Configuration data --------------------------------------------------

UCLASS()
class AEFPHARUS_API UDisplayClusterConfigurationHostDisplayData : public UObject
{
	GENERATED_BODY()
};

UCLASS()
class AEFPHARUS_API UDisplayClusterConfigurationViewport : public UObject
{
	GENERATED_BODY()
};

UCLASS()
class AEFPHARUS_API UDisplayClusterConfigurationClusterNode : public UObject
{
	GENERATED_BODY()
};

UCLASS()
class AEFPHARUS_API UDisplayClusterConfigurationCluster : public UObject
{
	GENERATED_BODY()
};

UCLASS()
class AEFPHARUS_API UDisplayClusterConfigurationData : public UObject
{
	GENERATED_BODY()
public:
	/** Mirror of engine's bFollowLocalPlayerCamera; the cluster BP touches it via CurrentConfigData. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Configuration")
	bool bFollowLocalPlayerCamera = false;
};

// -- Root actor + blueprint shells --------------------------------------

UCLASS()
class AEFPHARUS_API ADisplayClusterRootActor : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NDisplay")
	bool bPreviewEnable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NDisplay")
	TObjectPtr<UDisplayClusterConfigurationData> CurrentConfigData;
};

UCLASS()
class AEFPHARUS_API UDisplayClusterBlueprintGeneratedClass : public UBlueprintGeneratedClass
{
	GENERATED_BODY()
};

#if WITH_EDITORONLY_DATA
UCLASS()
class AEFPHARUS_API UDisplayClusterBlueprint : public UBlueprint
{
	GENERATED_BODY()
};
#endif
