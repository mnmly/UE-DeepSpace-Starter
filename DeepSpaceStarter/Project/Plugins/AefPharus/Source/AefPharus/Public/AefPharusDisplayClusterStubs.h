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

   These stubs carry no UPROPERTY layout — Slice 1 only validates that the
   root actor + Blueprint shells let the asset load far enough to surface
   the next layer of missing classes (components, configuration data).
  ========================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/BlueprintGeneratedClass.h"
#if WITH_EDITORONLY_DATA
#include "Engine/Blueprint.h"
#endif

#include "AefPharusDisplayClusterStubs.generated.h"

UCLASS()
class AEFPHARUS_API ADisplayClusterRootActor : public AActor
{
	GENERATED_BODY()
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
