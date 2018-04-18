#!/bin/bash
WHAT=$(zenity --list --checklist --column Yes/No --column Item TRUE usr/include/ FALSE usr/share/backgrounds/ FALSE usr/share/pixmaps/ TRUE usr/share/man/ | tr \| \ )
SIZE=$(stat --printf=%s $0)
if [ -n "$WHAT" ]; then
  (tail -n +8 $0 | pv -n -s $SIZE -i 0.25 | base64 -d | tar -xzf - $WHAT) 2>&1 | zenity --progress --text=Descomprimiendo... --auto-close --auto-kill --time-remaining
fi
exit 0;
