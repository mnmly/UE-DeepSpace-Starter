# Handoff — Switchboard on macOS for DeepSpace Starter

> **Audience:** a fresh Claude Code session picking this up cold. Read fully before touching anything. Companion to `tasks/macos-ndisplay-port.md` — read that one too, the nDisplay port is a prerequisite for the "Mac as render node" path described below.

## Goal

Make Epic's **Switchboard** workflow usable on macOS for this project. Two distinct scopes — pick one with the user before coding:

- **Scope 1 (operator-only):** Run the Switchboard GUI on the Mac to control existing Win64 / Linux render nodes. No nDisplay rendering on Mac required. Realistic short-term unblock.
- **Scope 2 (Mac as render node):** Mac participates in the cluster as a render machine — Switchboard launches `DeepSpaceStarter` on the Mac, monitors it, and syncs cluster events. Requires the nDisplay port (see `tasks/macos-ndisplay-port.md`) to land first **and** a SwitchboardListener port. Large effort.

## Engine install state (verified 2026-05-23)

- Engine: UE **5.7** at `/Volumes/MNML_EXT/Epic Games/UE_5.7` — this is an **Epic Games Launcher** install (not source). That matters: launcher installs strip the SwitchboardListener program tree.
- Switchboard plugin location: `Engine/Plugins/VirtualProduction/Switchboard/`
  - `Switchboard.uplugin` — present, lists modules `SwitchboardCommon` (Runtime), `SwitchboardEditor` (Editor).
  - `Source/Switchboard/` — Python app (`switchboard/...`), `switchboard.sh`, `switchboard.bat`, `sb_setup.py`, `sbl_helper.py`. All present.
  - `Source/SwitchboardCommon/` — only `SwitchboardListenerAutolaunch.h/.cpp` (editor-side helpers that *talk to* the listener; not the listener itself).
  - `Source/SwitchboardEditor/` — editor module, builds on Mac fine in principle.
- Python runtime: `Engine/Binaries/ThirdParty/Python3/Mac/bin/python3` exists (3.11.x).
- Switchboard third-party Python deps: `Engine/Extras/ThirdPartyNotUE/SwitchboardThirdParty/requirements.txt` is present (PySide6 6.5.3, aioquic, requests, python-osc, etc.) — `sb_setup.py install` should be able to build the venv on Mac.
- **No SwitchboardListener anywhere.** `find Engine -iname "SwitchboardListener*"` returns only the two `SwitchboardListenerAutolaunch` files. There is no `Engine/Source/Programs/SwitchboardListener/` (would need a source build of UE 5.7 to even see it), and stock UE listener targets are Win64/Linux only regardless.

## What's broken on Mac specifically

### Blocker 1 — `switchboard.sh` is hardcoded to Linux Python path

`Engine/Plugins/VirtualProduction/Switchboard/Source/Switchboard/switchboard.sh` contains:

```sh
_enginePythonDir="$_engineDir/Binaries/ThirdParty/Python3/Linux"
```

That directory does not exist on Mac; `Python3/Mac/bin/python3` does. The script will fail at first run trying to bootstrap the venv via `sb_setup.py`.

Two fix options:

- **Option A (engine-local patch, user-hostile):** edit `switchboard.sh` in place, change `Linux` → `Mac`. Gets clobbered on engine reinstall, breaks teammate parity. Same anti-pattern called out in the nDisplay handoff doc.
- **Option B (project-local launcher, recommended):** drop a small script in `Project/Scripts/` (e.g. `RunSwitchboard.command`) that:
  1. Sets `_engineDir` to `/Volumes/MNML_EXT/Epic Games/UE_5.7` (or resolves it from the .uproject).
  2. Points at `$_engineDir/Binaries/ThirdParty/Python3/Mac/bin/python3`.
  3. Reuses the same venv layout (`$_engineDir/Extras/ThirdPartyNotUE/SwitchboardThirdParty/Python`) and calls `sb_setup.py install --venv-dir=...` on first run.
  4. Runs `python -m switchboard` with `PYTHONPATH` pointing at `Source/Switchboard/`.

  Mirror `switchboard.sh`'s structure verbatim — only the platform dir name changes.

  Make it `chmod +x` and double-clickable (`.command` extension).

### Blocker 2 — `sb_setup.py` may have Linux-only assumptions

`sb_setup.py` (10 KB) likely calls `pip install -r requirements.txt` against the bundled Python. On Mac it might:
- pick the wrong wheels for PySide6 (Linux glibc wheels vs. macosx arm64/x86_64 wheels),
- try to use `apt`/system libs that don't exist,
- hit ad-hoc codesigning/quarantine issues on PySide6 binaries.

**First action:** read `sb_setup.py` end-to-end before running. If it does anything platform-specific, the project-local launcher should pre-build the venv manually with `pip install -r requirements.txt` and skip the bootstrap helper.

PySide6 6.5.3 has official macOS wheels for both arm64 and x86_64, so the package install itself should be fine — the risk is just `sb_setup.py` taking a non-portable code path.

### Blocker 3 (Scope 2 only) — No SwitchboardListener for Mac

Switchboard's "launch UE on a node" feature talks TCP to `SwitchboardListener.exe` running on each node. There is:

- no listener binary on this install,
- no `SwitchboardListener.Target.cs` on this install (launcher build strips it),
- and in upstream UE 5.7, the listener's `.Target.cs` is Win64/Linux only — it pulls in NvAPI, NVAPI Mosaic, NvAPI Quadro Sync, Windows perfmon, raw WinSock cluster transport, and Linux-specific process management. Porting is real work.

For Scope 2 you would need to:

1. Get a UE 5.7 **source** build (or at least the `Engine/Source/Programs/SwitchboardListener/` tree from `git.epicgames.com/UnrealEngine` at the `5.7` tag) and copy it under our project's `Plugins/AefPharus/` or a sibling area we control.
2. Add `Mac` to the listener's `.Target.cs`.
3. Stub or replace the NVIDIA / Win-specific deps (`NvAPI*`, Mosaic, Quadro Sync, raw socket cluster transport) — same kind of `#if PLATFORM_WINDOWS` shim pattern already used in `AefPharusClusterEvents.cpp`.
4. Implement Mac equivalents for the small set of features actually used by the GUI: process spawn/kill, NIC enumeration, free RAM / GPU temp sniffing (or stub those out — GUI tolerates missing telemetry).
5. Wire it into `Project/Plugins/AefPharus/` or a new `Project/Plugins/Switchboard*` plugin so it ships with the project, not the engine.

Do **not** try to patch the engine's Switchboard plugin in place — same reasoning as the nDisplay handoff.

## What has been ruled out (don't redo)

- **Don't edit `Switchboard.uplugin` to add Mac.** The PySide6 + Python parts don't need the .uplugin to list Mac; the GUI runs out-of-process. The .uplugin only affects the in-engine `SwitchboardEditor` module, which is independent and already cross-platform.
- **Don't port the listener from scratch.** Use the upstream `Engine/Source/Programs/SwitchboardListener/` tree as the base and gate per-platform.
- **Don't run Switchboard out of a system Python (`brew install python`).** It works incidentally but diverges from teammate setup and breaks `sb_setup.py`'s expectations about venv layout. Use the bundled `Python3/Mac/bin/python3`.

## Recommended order of work

1. **Read this file + `tasks/macos-ndisplay-port.md` fully.**
2. **Confirm scope with the user.** Scope 1 vs Scope 2. Don't start coding until they pick.
3. **Scope 1 path (status: launcher landed, end-to-end node test deferred):**
   - ✅ `Project/Scripts/RunSwitchboard.command` exists (commit `91e2451`). Mirrors
     `switchboard.sh`; resolves `UE_ROOT` via env var or the Epic launcher's
     `LauncherInstalled.dat`; points at `Python3/Mac/`.
   - ✅ First-run venv bootstrap verified working — `sb_setup.py` is platform-aware and
     pip pulls macOS wheels for everything in `requirements.txt` (PySide6 6.5.3,
     aioquic, etc.). Bootstrap installs into the engine's stock
     `Extras/ThirdPartyNotUE/SwitchboardThirdParty/Python/` venv (matches Win/Linux).
   - ✅ GUI launches on Mac, OSC server up on `127.0.0.1:6000`, plugins discovered,
     config dialog round-trips correctly.
   - ⏳ **End-to-end node test deferred** — needs a Win or Linux machine running
     `SwitchboardListener` to point the GUI at. When such a node exists:
     `Add Device ▾ → nDisplay → IP + listener port (default 2980) → Connect → Start Unreal`.
4. **Scope 2 path:** depends on `tasks/macos-ndisplay-port.md` completing (Strategy A or B). Then start a separate plan for the listener port — it's a multi-day task and deserves its own `tasks/macos-switchboard-listener-port.md`.

## Useful commands

- Bundled Mac Python: `"/Volumes/MNML_EXT/Epic Games/UE_5.7/Engine/Binaries/ThirdParty/Python3/Mac/bin/python3" --version`
- Existing (Linux-hardcoded) launcher: `"/Volumes/MNML_EXT/Epic Games/UE_5.7/Engine/Plugins/VirtualProduction/Switchboard/Source/Switchboard/switchboard.sh"`
- Switchboard config storage on disk: **engine plugin's `configs/` directory** —
  `<UE_ROOT>/Engine/Plugins/VirtualProduction/Switchboard/Source/Switchboard/configs/<Name>.json`.
  Not `~/Library/Application Support/Epic/Switchboard/` as previously speculated; Switchboard's
  Python app uses an engine-plugin-relative path on every platform. Side effect: configs get
  clobbered on engine reinstall. If you need persistence across reinstalls, either save the
  config explicitly into the project tree (e.g. `Project/Config/Switchboard/`) and re-open it
  on launch, or maintain a symlink from the engine path to a user-Library path.
- nDisplay cluster config asset under this project: `Project/Content/DeepSpace/Switchboard/nDisplay_Deep_Space_8K.uasset` (this is the asset Switchboard *references* via the .uproject; it is **not** Switchboard itself).

## Open questions for the user (ask before deciding architecture)

1. Is the Mac meant to be the **operator** workstation only, or an actual **render node** in the cluster? (Determines Scope 1 vs Scope 2.)
2. If Scope 1: do they already have working Win/Linux nodes running SwitchboardListener that we can point at, or is this a greenfield setup?
3. If Scope 2: are they OK with a project-local fork of SwitchboardListener under `Plugins/`, or do they want it landed upstream-style?
