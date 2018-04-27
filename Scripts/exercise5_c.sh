#!/bin/bash
WHAT=$(zenity --list --checklist --column Yes/No --column Item TRUE usr/include/ FALSE usr/share/backgrounds/ FALSE usr/share/pixmaps/ TRUE usr/share/man/ | tr \| \ )
SIZE=$(stat --printf=%s $0)
if [ -n "$WHAT" ]; then
  # Prompt the user with the path selector and save it into the WHERE variable, then modify the tail number with the added lines and pass it to tar comand with -C
  WHERE=$(zenity --file-selection --directory)
  (tail -n +10 $0 | pv -n -s $SIZE -i 0.25 | base64 -d | tar -C $WHERE -xzf - $WHAT) 2>&1 | zenity --progress --text=Descomprimiendo... --auto-close --auto-kill --time-remaining
fi
exit 0;