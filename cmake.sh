#!/bin/bash
: ${_G_buildir:="bin/build"}
: ${_G_bindir:="bin"}
: ${_G_debug:=false}
: ${_G_clean:=false}

# Hidden build all temporary environment for sourcing
<<__BUILDALLINIT__
# Auto-config for build_al.sh
GIT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2> /dev/null || echo "none")
GIT_HEAD=$(git rev-parse --short HEAD || echo "none")
BUILD_TYPE=$({ $_G_debug && echo Debug; } || echo Release)
OUTDIR="$_G_bindir/${BUILD_TYPE}"
DIST_FOLDER="$_G_bindir/Distribution"
DIST_NAME="${GIT_BRANCH}-${GIT_HEAD}.zip"
DIST_COMMENT="$(date +%Y)/$(date +%m)/$(date +%d):$(git rev-parse HEAD)"
: ${DIST_ENABLE:=$({ $_G_debug && echo 0; } || echo 1)}
UPX_ENABLE="$DIST_ENABLE"
INCLUDE_SRC=0
UPX_FLAGS="-q --best --lzma --compress-exports=0"
CLEAN_BUILD=$({ $_G_clean && echo 1; } || echo 0)
DEBUG=$({ $_G_debug && echo 1; } || echo 0)
TOOLCHAIN_32="external/cmake-various/toolchain/toolchain-mingw_nix-win-x86.cmake"
TARGETS_32="spicetools_stubs_kbt spicetools_stubs_kld spicetools_cfg spicetools_spice"
BUILDDIR_32="$_G_buildir/x86/${BUILD_TYPE}"
TOOLCHAIN_64="external/cmake-various/toolchain/toolchain-mingw_nix-win-x64.cmake"
TARGETS_64="spicetools_stubs_kbt64 spicetools_stubs_kld64 spicetools_spice64"
BUILDDIR_64="$_G_buildir/x64/${BUILD_TYPE}"
__BUILDALLINIT__

run_build_all() {
  # Init build_all.sh
  source <(sed -n '/__BUILDALLINIT__/{:a;n;/__BUILDALLINIT__/q;p;$!ba}' "$0")
  source <(head -n +20 ./build_all.sh)

  # Run build_all.sh
  source <(tail -n +59 ./build_all.sh | head -n -3 | sed 's~.*pushd ${OUTDIR}/.. > /dev/null*~\tpushd ${OUTDIR} > /dev/null~')
}

run_deploy_all() {
  DIST_ENABLE=0

  # Init build_all.sh
  source <(sed -n '/__BUILDALLINIT__/{:a;n;/__BUILDALLINIT__/q;p;$!ba}' "$0")
  source <(head -n +20 ./build_all.sh)

  # Run deploy in build_all.sh
  source <(tail -n +150 ./build_all.sh | head -n -3 | sed 's~.*pushd ${OUTDIR}/.. > /dev/null*~\tpushd ${OUTDIR} > /dev/null~')
}

run_distribute_all() {
  # Init build_all.sh
  source <(sed -n '/__BUILDALLINIT__/{:a;n;/__BUILDALLINIT__/q;p;$!ba}' "$0")
  source <(head -n +20 ./build_all.sh)

  # Run deploy in build_all.sh
  source <(tail -n +150 ./build_all.sh | head -n -3 | sed 's~.*pushd ${OUTDIR}/.. > /dev/null*~\tpushd ${OUTDIR} > /dev/null~')
}

# Sanity checks
[ -d "external/cmake-various/toolchain" ] || { echo "Toolchain not found" ; exit 1; }
[ "$(md5sum ./build_all.sh | cut -c -32)" = "9a89bba3fa2435983e5bce423d0f4a61" ] || { echo "Version mismatch for build_all.sh " ; exit 1; }

# Run requested task
if [[ $(type -t "run_${1}_${2}") == function ]]; then
  echo "Starting task: 'run_${1}_${2}'"
  "run_${1}_${2}"
else
  echo "Invalid task: 'run_${1}_${2}'"
  exit 1
fi

