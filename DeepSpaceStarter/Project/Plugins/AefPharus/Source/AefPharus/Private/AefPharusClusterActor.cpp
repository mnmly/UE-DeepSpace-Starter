/*========================================================================
   Copyright (c) Ars Electronica Futurelab, 2025

   AefPharus - Cluster Actor Implementation
  ========================================================================*/

#include "AefPharusClusterActor.h"
#include "AefPharus.h"

// DisplayCluster component support is gated by Build.cs (Win64/Linux only).
// See AefPharusRootOriginActor.cpp for rationale on not using __has_include.
#if AEFPHARUS_WITH_DISPLAYCLUSTER
	#include "Components/DisplayClusterSceneComponentSyncThis.h"
	#define AefPharus_HAS_DISPLAYCLUSTER_COMPONENTS 1
#else
	#define AefPharus_HAS_DISPLAYCLUSTER_COMPONENTS 0
	#include "Components/SceneComponent.h"
#endif

//--------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------

AAefPharusClusterActor::AAefPharusClusterActor()
{
#if AefPharus_HAS_DISPLAYCLUSTER_COMPONENTS
	// Replace the parent's default SceneComponent root with a DisplayCluster
	// sync component to enable automatic transform synchronization across
	// cluster nodes. Safe because the new subobject uses a different name.
	if (RootComponent)
	{
		RootComponent->DestroyComponent();
		RootComponent = nullptr;
	}
	ClusterSyncComponent = CreateDefaultSubobject<UDisplayClusterSceneComponentSyncThis>(TEXT("ClusterSyncRoot"));
	RootComponent = ClusterSyncComponent;

	UE_LOG(LogAefPharus, Verbose, TEXT("AefPharusClusterActor created with DisplayCluster sync component"));
#else
	// Without DisplayCluster, keep the parent's "Root" USceneComponent as-is
	// and expose it through ClusterSyncComponent. CreateDefaultSubobject would
	// crash with "subobject already exists" since the parent registers the
	// same name; DestroyComponent at construction time only marks the runtime
	// instance for destruction and does not free the CDO entry.
	ClusterSyncComponent = RootComponent;

	UE_LOG(LogAefPharus, Warning, TEXT("AefPharusClusterActor: DisplayCluster not available, using inherited SceneComponent root"));
#endif
}
