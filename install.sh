#!/usr/bin/env bash
#Made by ouroboros a.k.a WhereIsRooot
set -euo pipefail

SRC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC="$SRC_DIR/ipcensor.c"
BIN_DIR="$HOME/bin"
BIN="$BIN_DIR/ipcensor"
RC="$HOME/.bashrc"
MARK="# >>> ipcensor autostart >>>"

[ -f "$SRC" ] || { echo "No file $SRC" >&2; exit 1; }
command -v gcc >/dev/null 2>&1 || { echo "gcc is not install" >&2; exit 1; }

mkdir -p "$BIN_DIR"
gcc "$SRC" -o "$BIN" -lutil
chmod +x "$BIN"
chmod +x disable_auto_start.sh

if ! grep -qF "$MARK" "$RC" 2>/dev/null; then
    cat >> "$RC" <<'EOF'

# >>> ipcensor autostart >>>
if [[ $- == *i* && -z $CENSOR_ON && -x $HOME/bin/ipcensor ]]; then
    export CENSOR_ON=1
    exec "$HOME/bin/ipcensor"
fi
# <<< ipcensor autostart <<<
EOF
fi

export CENSOR_ON=1
exec "$BIN"