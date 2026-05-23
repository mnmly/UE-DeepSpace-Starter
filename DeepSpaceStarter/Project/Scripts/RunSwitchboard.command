#!/bin/sh
# Mac launcher for Epic's Switchboard GUI.
#
# Equivalent to Engine/Plugins/VirtualProduction/Switchboard/Source/Switchboard/switchboard.sh
# but pointed at Python3/Mac/ instead of Python3/Linux/, and locating the engine
# via $UE_ROOT or the Epic launcher registry instead of a relative ../../../ walk
# (this script lives in the project tree, not the engine).
#
# Usage:
#   double-click in Finder, or run from a terminal.
# Override engine path:
#   UE_ROOT="/path/to/UE_5.7" "$0"

set -e

# 1) Resolve engine root.
if [ -z "$UE_ROOT" ]; then
    _launcherDat="$HOME/Library/Application Support/Epic/UnrealEngineLauncher/LauncherInstalled.dat"
    if [ -f "$_launcherDat" ]; then
        UE_ROOT=$(python3 -c "
import json, sys
with open('$_launcherDat') as f:
    d = json.load(f)
for x in d.get('InstallationList', []):
    if x.get('AppName') == 'UE_5.7':
        print(x['InstallLocation'])
        sys.exit(0)
sys.exit(1)
" 2>/dev/null) || true
    fi
fi
if [ -z "$UE_ROOT" ] || [ ! -d "$UE_ROOT" ]; then
    echo "error: cannot find UE 5.7 install."
    echo "  - set UE_ROOT, or"
    echo "  - install UE 5.7 via the Epic Games Launcher and re-run."
    exit 1
fi
echo "engine: $UE_ROOT"

# 2) Path layout mirrors switchboard.sh; only Linux -> Mac changes.
_switchboardDir="$UE_ROOT/Engine/Plugins/VirtualProduction/Switchboard/Source/Switchboard"
_enginePythonDir="$UE_ROOT/Engine/Binaries/ThirdParty/Python3/Mac"
_venvDir="$UE_ROOT/Engine/Extras/ThirdPartyNotUE/SwitchboardThirdParty/Python"

if [ ! -x "$_enginePythonDir/bin/python3" ]; then
    echo "error: bundled Mac Python missing at $_enginePythonDir/bin/python3"
    exit 1
fi

# 3) Bootstrap venv on first run (sb_setup.py is platform-aware; non-Win path
#    uses bin/lib/lib64 which is correct on Mac, and PySide6 6.5.3 ships Mac wheels).
if [ ! -x "$_venvDir/bin/python3" ]; then
    echo "bootstrapping Switchboard venv at $_venvDir"
    "$_enginePythonDir/bin/python3" "$_switchboardDir/sb_setup.py" install --venv-dir="$_venvDir"
fi

# 4) Launch GUI.
cd "$_switchboardDir"
PYTHONPATH="$_switchboardDir:$PYTHONPATH" exec "$_venvDir/bin/python3" -m switchboard "$@"
