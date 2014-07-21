#!/usr/bin/env bash

DEST=${1:-build/socrocket-emc2}
shift

if [ -e "$DEST" ] ; then
  rm -rf "${DEST}"
fi
mkdir -p ${DEST}

find . \
  ! \( -path './build**' \
    -o -path '**pyc' \
    -o -path '**~' \
    -o -path './adapters**' \
    -o -path './pysc**' \
    -o -path '**greth**' \
    -o -path '**mips**' \
    -o -path '**stimgen**' \
    -o -path '**ahbudp**' \
    -o -path '**ahbcamera**' \
    -o -path '**ahbdisplay**' \
    -o -path '**ahbshuffler**' \
    -o -path '**yuvstream**' \
    -o -path '**mips1sp**' \
    -o -path '**stimgensys**' \
    -o -path '**/tests**' \
    -o -path './platforms/base**' \
    -o -path './models/socwire**' \
    -o -path './software/demo**' \
    -o -path './doc/srw-2014-6**' \
    -o -path './.waf**' \
    -o -path './.git**' \
    -o -path '**mibench**' \
    -o -path '**visloc_ref_sw**' \
    -o -path '**rtems**' \
    -o -path '**extern/trap-gen**' \
    -o -path '**extern/LEON3/trapSources**' \
    -o -path '**extern/LEON3/iss_standaloneSources**' \
    -o -path '**ahbspacewire**' \
    -o -path '**extern/SpaceWire**' \
  \) -type f | xargs -I {} cp -p --parents -t ${DEST} {}

cd ${DEST}
./waf configure $@
./waf -j4
./waf distclean
cd ..
NAME=$(basename ${DEST})
tar -cjf ${NAME}.tar.bz2 ${NAME}
