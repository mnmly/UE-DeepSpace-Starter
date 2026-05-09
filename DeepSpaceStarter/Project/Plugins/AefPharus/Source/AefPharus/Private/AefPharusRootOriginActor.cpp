/*========================================================================
   Copyright (c) Ars Electronica Futurelab, 2025

   AefPharus - Root Origin Reference Actor Implementation
  ========================================================================*/

#include "AefPharusRootOriginActor.h"
#include "AefPharusSubsystem.h"
#include "AefPharus.h"
#include "Kismet/GameplayStatics.h"

// DisplayCluster component support is gated by Build.cs (Win64/Linux only).
// __has_include is unreliable here: UBT mounts nDisplay's public include path
// even on platforms where the plugin is disabled, so the .generated.h is
// missing at compile time despite the header being "found".
#if AEFPHARUS_WITH_DISPLAYCLUSTER
	#include "Components/DisplayClusterSceneComponentSyncThis.h"
	#define AefPharus_HAS_DISPLAYCLUSTER_COMPONENTS 1
#else
	#define AefPharus_HAS_DISPLAYCLUSTER_COMPONENTS 0
	#include "Components/SceneComponent.h"
#endif

// Editor-only components
#if WITH_EDITORONLY_DATA
	#include "Components/BillboardComponent.h"
	#include "Components/ArrowComponent.h"
	#include "UObject/ConstructorHelpers.h"
#endif

//--------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------

AAefPharusRootOriginActor::AAefPharusRootOriginActor()
{
	// This actor doesn't need to tick - position is queried on demand
	PrimaryActorTick.bCanEverTick = false;

	// Destroy existing root component before creating new one
	if (RootComponent)
	{
		RootComponent->DestroyComponent();
		RootComponent = nullptr;
	}

#if AefPharus_HAS_DISPLAYCLUSTER_COMPONENTS
	// Create DisplayCluster sync component as root for automatic cluster replication
	ClusterSyncRoot = CreateDefaultSubobject<UDisplayClusterSceneComponentSyncThis>(TEXT("ClusterSyncRoot"));
	RootComponent = ClusterSyncRoot;
	
	UE_LOG(LogAefPharus, Verbose, TEXT("AefPharusRootOriginActor: Created with DisplayCluster sync component"));
#else
	// Fallback to standard SceneComponent if DisplayCluster is not available
	ClusterSyncRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = ClusterSyncRoot;
	
	UE_LOG(LogAefPharus, Warning, TEXT("AefPharusRootOriginActor: DisplayCluster not available, using standard SceneComponent"));
#endif

#if WITH_EDITORONLY_DATA
	// Create billboard for editor visibility
	EditorBillboard = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("OriginBillboard"));
	if (EditorBillboard)
	{
		EditorBillboard->SetupAttachment(RootComponent);
		EditorBillboard->SetHiddenInGame(true);
		EditorBillboard->bIsScreenSizeScaled = true;
		
		// Try to load a custom icon for the origin marker
		static ConstructorHelpers::FObjectFinder<UTexture2D> IconTexture(
			TEXT("/Engine/EditorResources/S_TargetPoint.S_TargetPoint"));
		if (IconTexture.Succeeded())
		{
			EditorBillboard->SetSprite(IconTexture.Object);
		}
	}

	// Create arrow showing forward direction (X-axis in Unreal)
	ForwardArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("ForwardArrow"));
	if (ForwardArrow)
	{
		ForwardArrow->SetupAttachment(RootComponent);
		ForwardArrow->SetHiddenInGame(true);
		ForwardArrow->ArrowColor = FColor::Red;  // X-axis = Red
		ForwardArrow->ArrowSize = 2.0f;
		ForwardArrow->ArrowLength = 100.0f;
		ForwardArrow->SetRelativeRotation(FRotator::ZeroRotator);  // Points forward (X)
	}
#endif
}

//--------------------------------------------------------------------------------
// Lifecycle
//--------------------------------------------------------------------------------

void AAefPharusRootOriginActor::BeginPlay()
{
	Super::BeginPlay();

	// Register with AefPharusSubsystem as the origin source
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UAefPharusSubsystem* Subsystem = GameInstance->GetSubsystem<UAefPharusSubsystem>();
		if (Subsystem)
		{
			Subsystem->RegisterRootOriginActor(this);
			bIsRegistered = true;
			
			UE_LOG(LogAefPharus, Log, TEXT("AefPharusRootOriginActor '%s' registered at location: %s"),
				*GetName(), *GetActorLocation().ToString());
		}
		else
		{
			UE_LOG(LogAefPharus, Warning, TEXT("AefPharusRootOriginActor '%s': AefPharusSubsystem not found - origin will not be used"),
				*GetName());
		}
	}
	else
	{
		UE_LOG(LogAefPharus, Warning, TEXT("AefPharusRootOriginActor '%s': GameInstance not found"),
			*GetName());
	}
}

void AAefPharusRootOriginActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister from AefPharusSubsystem
	if (bIsRegistered)
	{
		UGameInstance* GameInstance = GetGameInstance();
		if (GameInstance)
		{
			UAefPharusSubsystem* Subsystem = GameInstance->GetSubsystem<UAefPharusSubsystem>();
			if (Subsystem)
			{
				Subsystem->UnregisterRootOriginActor(this);
				UE_LOG(LogAefPharus, Log, TEXT("AefPharusRootOriginActor '%s' unregistered"), *GetName());
			}
		}
		bIsRegistered = false;
	}

	Super::EndPlay(EndPlayReason);
}

//--------------------------------------------------------------------------------
// Public API
//--------------------------------------------------------------------------------

FVector AAefPharusRootOriginActor::GetOriginLocation() const
{
	return GetActorLocation();
}

FRotator AAefPharusRootOriginActor::GetOriginRotation() const
{
	return GetActorRotation();
}

bool AAefPharusRootOriginActor::IsRegisteredAsOrigin() const
{
	return bIsRegistered;
}
