#! /bin/sh

cat <<EOF
#include "NeoPop-SDL.h"

const char *npev_names[] = {
EOF

sed -n 's/^[	 ]*NPEV_\([A-Z0-9_]*\),.*/    "\1",/p' "$1" | tr A-Z_ 'a-z '

cat <<EOF
};

const char *nprc_names[] = {
EOF

sed -n 's/^[	 ]*NPRC_\([A-Z0-9_]*\),.*/    "\1",/p' "$1" | tr A-Z_ 'a-z-'

cat <<EOF
};

const char *comms_names[] = {
EOF

sed -n 's/^[	 ]*COMMS_\([A-Z0-9_]*\),.*/    "\1",/p' "$1" | tr A-Z_ 'a-z-'

cat <<EOF
};
EOF
