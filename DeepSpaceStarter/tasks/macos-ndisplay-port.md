# Handoff — macOS nDisplay port for DeepSpace Starter

> **Audience:** a fresh Claude Code session picking this up cold. Read all of it before touching anything.

## Goal

Make `nDisplay_Deep_Space_8K` (a `DisplayClusterBlueprintGeneratedClass` asset
placed in `Project/Content/DeepSpace/Maps/Rooms/Deep_Space_8K.umap`) load
and function on **macOS**, alongside the existing Win64 / Linux paths. Do **not**
remove or degrade Win64 / Linux behavior.

The macOS editor already launches, blueprints compile, and PIE runs — what's
missing is the cluster root actor that drives the display layout.

## Repo / branch state

- Project root: `/Volumes/MNML_EXT/Unreal Projects/UE-DeepSpace-Starter/DeepSpaceStarter`
- Branch: `feature/macos` (branched off `UE-5.7-DeepSpace-Starter`)
- Engine: UE **5.7** installed at `/Volumes/MNML_EXT/Epic Games/UE_5.7`
- Three commits already on the branch (ordered oldest → newest):
  1. `9470535` — Add macOS support to AefPharus plugin and project targets
  2. `4bf3a01` — Fix Mac CDO crash in AAefPharusClusterActor
  3. `a40f870` — Add cross-platform cluster events shim BPFL

There is also one **uncommitted in-place edit** to a `.uasset`:
- `Project/Content/DeepSpace/Library/BP_DeepSpace_Library.uasset` was rewritten via
  `bpx ref rewrite` to redirect its `/Script/DisplayCluster.*` import references
  to `/Script/AefPharus.*` (so the blueprint loads on Mac through our shim).
  Backup at `BP_DeepSpace_Library.uasset.backup`. **Do not blindly revert.**

## What's already working on macOS

- `DeepSpaceStarterEditor Mac Development` builds cleanly.
- AefPharus plugin compiles, runs in the editor.
- `BP_DeepSpace_Library` compiles cleanly (was previously red).
- PIE in `Deep_Space_8K.umap` starts and ticks.
- Pharus subsystem initializes, root origin actor binds.

## What's broken on macOS (the task)

`nDisplay_Deep_Space_8K` (the display cluster root actor blueprint at
`Project/Content/DeepSpace/Switchboard/nDisplay_Deep_Space_8K.uasset`) is a
`DisplayClusterBlueprintGeneratedClass` whose entire export chain depends on
classes from `/Script/DisplayCluster` and `/Script/DisplayClusterConfiguration`
(e.g. `DisplayClusterRootActor`, `DisplayClusterStageIsosphereComponent`,
`DisplayClusterSceneComponentSyncThis`, ~25 cluster component types). On Mac the
nDisplay modules aren't built, so:

- Asset fails to load (`CreateExport: Failed to load Outer for resource ...`
  for `Room Geometry`, `TXT_Floor/Wall/Dim`, `S_Mox_Description`, all `SCS_Node_*`,
  `LineBatcher`, etc.).
- The actor never instantiates in PIE → display layout doesn't render.

## What has been ruled out (and why) — do not redo this work

1. **Don't add Mac to `nDisplay.uplugin`'s `SupportedTargetPlatforms`.**
   - The plugin descriptor at
     `/Volumes/MNML_EXT/Epic Games/UE_5.7/Engine/Plugins/Runtime/nDisplay/nDisplay.uplugin`
     hard-codes `["Win64", "Linux"]` at the top level **and** in every one of its 25+
     module `PlatformAllowList` entries (some are Win64-only).
   - Even if you patch the .uplugin: the modules pull in DirectX, NVIDIA Quadro
     Sync / NvAPI, MOSAIC, MPCDI, WinSock cluster transports, and several
     Windows-specific render paths. The Linux build relies on Vulkan-portable code
     paths that don't have Metal equivalents. Hundreds of compile errors with no
     clear porting plan.
   - Patching an installed engine plugin is also user-hostile: gets clobbered on
     engine reinstall, breaks teammate parity.

2. **Don't try to write a "BSD socket" port of `UDPManager.cpp` from scratch.**
   - Already shimmed minimally for Mac in commit `9470535`; preserves the existing
     Win path. If you touch it, keep `#if PLATFORM_WINDOWS` for the original code.

3. **Don't redefine engine types in our module.**
   - Tried that mentally for `FDisplayClusterClusterEventJson` — name collisions
     when nDisplay is loaded on Win/Linux. Use **stub types in our own namespace**
     (we did: `FAefPharusClusterEventJson` etc.), then redirect imports.

## What worked for the BP layer (read before designing the actor-side approach)

The blueprint `BP_DeepSpace_Library` was fixed by:

1. Building a cross-platform shim BPFL in `Plugins/AefPharus/Source/AefPharus/`:
   - `Public/AefPharusClusterEvents.h` — declares `EAefPharusClusterRole`,
     `FAefPharusClusterEventBase`, `FAefPharusClusterEventJson`,
     `UAefPharusClusterEvents` with `GetClusterRole`,
     `IsClusterPrimary`, `EmitClusterEventJson`.
   - `Private/AefPharusClusterEvents.cpp` — gated by `AEFPHARUS_WITH_DISPLAYCLUSTER`.
     On Win/Linux internally converts our stub struct → real
     `FDisplayClusterClusterEventJson` and calls the real BPFL. On Mac logs
     Verbose and returns.
2. Running `bpx ref rewrite` on the .uasset to swap import name tokens. The
   tokens swapped (in order; substring replacement, so order matters):
   ```
   /Script/DisplayCluster          → /Script/AefPharus
   DisplayClusterBlueprintLib      → AefPharusClusterEvents
   EDisplayClusterNodeRole         → EAefPharusClusterRole
   DisplayClusterClusterEventBase  → AefPharusClusterEventBase
   DisplayClusterClusterEventJson  → AefPharusClusterEventJson
   ```
   `Default__DisplayClusterBlueprintLib` was rewritten transitively by the second swap.
   `EmitClusterEventJson` and `GetClusterRole` names match in both libraries so
   no swap needed.

This same pattern *can* in principle work for `nDisplay_Deep_Space_8K`, but it's
**a much bigger surface**: ~25 component classes, the
`DisplayClusterBlueprintGeneratedClass` itself, the
`DisplayClusterConfiguration*` data types, and an actor base (`ADisplayClusterRootActor`).

## Possible strategies for the task (in increasing order of effort)

### Strategy A — Don't load the asset on Mac, place a stand-in
- Keep `nDisplay_Deep_Space_8K` as-is.
- Use a `DefaultGameMode` or `AGameModeBase::InitGame` C++ hook to detect Mac at runtime
  and spawn a lightweight stand-in `AActor` that exposes the same Blueprint
  callable methods the rest of the project expects (look at every level/BP that
  references the cluster actor first — `bpx import graph Content --recursive
  --filter nDisplay_Deep_Space_8K --group-by object` lists them).
- Lowest effort, no rendering / cluster behavior on Mac. Possibly enough for
  pre-vis / asset iteration use cases.

### Strategy B — Stub the entire ADisplayClusterRootActor surface in C++
- Mirror the inheritance/component shape of the cluster root actor in our plugin
  with `#if !AEFPHARUS_WITH_DISPLAYCLUSTER` versions (empty implementations).
- Add ~25 stub UClass component types (one per component listed in `nDisplay.uplugin`'s
  module list — see `bpx export list` on the asset for the exact subset used).
- Use `bpx ref rewrite` (or write a small wrapper script) to swap every
  `/Script/DisplayCluster.*` and `/Script/DisplayClusterConfiguration.*` import
  in `nDisplay_Deep_Space_8K.uasset` to point at our stubs.
- Effort: probably a day. Risk: serialized BodySetup / asset data for components
  may not round-trip if the stub class doesn't have the same UPROPERTY layout.

### Strategy C — Real Metal port of the render path (don't)
- Out of scope. Would require Epic-level engineering on the nDisplay projection
  / warp / shader path.

The realistic choice is almost certainly **A** for short-term unblock and **B**
for long-term clean integration. Talk to the user before committing to B.

## Useful tools and commands

- **bpx** (`/usr/local/bin/bpx`, source at `~/Development-local/GitHub/go/openbpx`)
  - `bpx blueprint info <file.uasset>` — high-level summary
  - `bpx export list <file.uasset>` — every export (find subobjects)
  - `bpx import list <file.uasset>` — every import (find external dep)
  - `bpx import graph Content --recursive --filter <token> --group-by object` — repo-wide refs
  - `bpx ref rewrite <file.uasset> --from <old> --to <new> [--dry-run] [--backup]`
    — **substring** replacement in NameMap (verify scope first with `--dry-run`).
  - `bpx blueprint disasm <file.uasset> --export <n> --format text` — function bytecode
  - `bpx blueprint search ... --show member` — find specific call sites
  - `bpx validate <file.uasset>` — sanity check after edits
  - bpx can't edit K2Node pin payloads (custom serialization region — warns
    "pin payload parsed with N trailing bytes" on read).

- **Build:**
  ```
  cd "/Volumes/MNML_EXT/Epic Games/UE_5.7" && \
    ./Engine/Build/BatchFiles/Mac/Build.sh DeepSpaceStarterEditor Mac Development \
    -Project="/Volumes/MNML_EXT/Unreal Projects/UE-DeepSpace-Starter/DeepSpaceStarter/Project/DeepSpaceStarter.uproject" \
    -NoHotReloadFromIDE
  ```

- **Editor log:** `~/Library/Logs/Unreal Engine/DeepSpaceStarterEditor/DeepSpaceStarter.log`

- **Plugin source:** `Project/Plugins/AefPharus/Source/AefPharus/`
  - `AefPharus.Build.cs` — gates `DisplayCluster` dep + sets `AEFPHARUS_WITH_DISPLAYCLUSTER`
    define on Win64/Linux only. Same flag used in code to choose real vs stub paths.

## Open / pre-existing issues to be aware of

1. **`Failed to parse GlobalRotation: '(Pitch=0.0,Yaw=0.0,Roll=0.0)'`** at
   `AefPharusSubsystem.cpp:643`. Pre-existing Win/Linux bug too: `FRotator::InitFromString`
   expects `P=0.0 Y=0.0 R=0.0` format, not `(Pitch=…,Yaw=…,Roll=…)`. Falls back to identity.
   Easy fix when you're in there; not Mac-specific.

2. **Two pin-type errors in `BP_DeepSpace_Library`'s "Attach nDisplay to Camera"
   function** (Object Reference vs Actor Object Reference). The function's
   `Target` input parameter is typed as `Object` but feeds into
   `K2_AttachToActor` which expects `Actor`. Pre-existing, surfaced after our
   cluster fixes cleared the bigger compile blockers. Must be fixed **in the
   editor** — bpx and UE Python both lack the API to mutate
   `K2Node_FunctionEntry::UserDefinedPins` pin types cleanly.

3. **`DisplayClusterRootActor`** dangling import in `BP_DeepSpace_Library` —
   pointed at `/Script/AefPharus.DisplayClusterRootActor` (doesn't exist there).
   Harmless because no node references it, but Strategy B would naturally
   resolve this by introducing a real stub class with that name.

4. **`Binaries/Mac/`** directories under `Project/Plugins/AefPharus/Binaries/`
   are currently untracked and uncommitted. Add a `.gitignore` rule before
   committing anything else if appropriate for the repo's conventions.

## First moves for the new session

1. Read this file fully. Don't skim.
2. `git status` to confirm the working-tree state matches what's described above.
3. `bpx export list Project/Content/DeepSpace/Switchboard/nDisplay_Deep_Space_8K.uasset`
   and `bpx import list` on the same file — get the *exact* subset of cluster
   components actually used (it's a subset of the 25 modules in the .uplugin).
4. `bpx import graph Project/Content --recursive --filter nDisplay_Deep_Space_8K
   --group-by object` — find every asset referencing the cluster actor, so you
   know your blast radius before redirecting.
5. Ask the user which strategy (A vs B) they want, present the trade-offs.
   Don't start coding until they pick.
