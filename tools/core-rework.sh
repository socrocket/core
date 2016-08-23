#!/usr/bin/bash

set -e

if [ ! -d core/base ] ; then

  mkdir core/base
  mkdir core/utils
  mkdir amba

  mv core/common/sr_iss core/sr_iss
  mv core/common/trapgen core/trapgen

  cp core/common/wscript core/base/wscript
  cp core/common/wscript core/utils/wscript
  cp core/common/wscript amba/wscript

  for i in base.h msb_lsb.h systemc.h sc_api.h sc_find.h socrocket.h strace.cpp vendian.h verbose.cpp verbose.h vmap.h waf.cpp waf.h ; do
    mv core/common/$i core/base/$i
  done

  for i in ahbdevicebase.h ahbdevice.h ahbdevice.tpp ahbmaster.h ahbmaster.tpp ahbslave.h ahbslave.tpp amba.h apbdevicebase.h apbdevice.h apbdevice.tpp apbslave.h ; do
    mv core/common/$i amba/$i
  done

  for i in msclogger.cpp msclogger.h reliabilitymanager.h timingmonitor.cpp timingmonitor.h powermonitor.cpp powermonitor.h ; do
    mv core/common/$i core/utils/$i
  done
fi

find core amba gaisler pysc \
  -name '*.h' -o \
  -name '*.hpp' -o \
  -name '*.i' -o \
  -name '*.cpp' -o \
  -name '*.tpp' | xargs -I {} sed -i.bak \
  -e 's|"core/common/amba.h"|"amba/amba.h"|g' \
  -e 's|"core/common/ahbdevicebase.h"|"amba/ahbdevicebase.h"|g' \
  -e 's|"core/common/apbdevicebase.h"|"amba/apbdevicebase.h"|g' \
  -e 's|"core/common/ahbdevice.h"|"amba/ahbdevice.h"|g' \
  -e 's|"core/common/apbdevice.h"|"amba/apbdevice.h"|g' \
  -e 's|"core/common/ahbdevice.tpp"|"amba/ahbdevice.tpp"|g' \
  -e 's|"core/common/apbdevice.tpp"|"amba/apbdevice.tpp"|g' \
  -e 's|"core/common/ahbmaster.h"|"amba/ahbmaster.h"|g' \
  -e 's|"core/common/ahbslave.h"|"amba/ahbslave.h"|g' \
  -e 's|"core/common/apbslave.h"|"amba/apbslave.h"|g' \
  -e 's|"core/common/powermonitor.h"|"core/utils/powermonitor.h"|g' \
  -e 's|"core/common/sc_find.h"|"core/base/sc_find.h"|g' \
  -e 's|"core/common/systemc.h"|"core/base/systemc.h"|g' \
  -e 's|"core/common/timingmonitor.h"|"core/utils/timingmonitor.h"|g' \
  -e 's|"core/common/vendian.h"|"core/base/vendian.h"|g' \
  -e 's|"core/common/verbose.h"|"core/base/verbose.h"|g' \
  -e 's|"core/common/vmap.h"|"core/base/vmap.h"|g' \
  -e 's|"core/common/waf.h"|"core/base/waf.h"|g' \
  -e 's|"core/common/base.h"|"core/base/base.h"|g' \
  -e 's|"core/common/msclogger.h"|"core/utils/msclogger.h"|g' \
  -e 's|core/common/trapgen/|core/trapgen/|g' \
  -e 's|core/common/sr_iss/|core/sr_iss/|g' \
  {}

