#! /bin/sh

cat <<EOF
#include "NeoPop-SDL.h"

const char *event_names[] = {
EOF

sed -n 's/[	 ]*NPEV_\([A-Z0-9_]*\),.*/    "\1",/p' "$1" | tr A-Z_ 'a-z '

cat <<EOF
};
EOF
