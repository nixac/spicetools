#!/bin/bash
_G_buildir="bin/build"
_G_bindir="bin"
_G_debug=false
_G_clean=false


run_build_all() {
  # Auto-config for build_al.sh
  GIT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2> /dev/null || echo "none")
  GIT_HEAD=$(git rev-parse --short HEAD || echo "none")
  BUILD_TYPE=$({ $_G_debug && echo Debug; } || echo Release)
  OUTDIR="$_G_bindir/${BUILD_TYPE}"
  DIST_FOLDER="$_G_bindir"
  DIST_NAME="${GIT_BRANCH}-${GIT_HEAD}.zip"
  DIST_COMMENT="$(date +%Y)/$(date +%m)/$(date +%d):$(git rev-parse HEAD)"
  DIST_ENABLE=$({ $_G_debug && echo 0; } || echo 1)
  UPX_ENABLE="$DIST_ENABLE"
  INCLUDE_SRC=0
  UPX_FLAGS="-q --best --lzma --compress-exports=0"
  CLEAN_BUILD=$({ $_G_clean && echo 1; } || echo 0)
  DEBUG=$({ $_G_debug && echo 1; } || echo 0)
  TOOLCHAIN_32="/usr/share/mingw/toolchain-i686-w64-mingw32.cmake"
  TARGETS_32="spicetools_stubs_kbt spicetools_stubs_kld spicetools_cfg spicetools_spice"
  BUILDDIR_32="$_G_buildir/x86/${BUILD_TYPE}"
  TOOLCHAIN_64="/usr/share/mingw/toolchain-x86_64-w64-mingw32.cmake"
  TARGETS_64="spicetools_stubs_kbt64 spicetools_stubs_kld64 spicetools_spice64"
  BUILDDIR_64="$_G_buildir/x64/${BUILD_TYPE}"

  # Run build_all.sh
  source <(head -n +20 ./build_all.sh)
  source <(tail -n +59 ./build_all.sh | head -n -3 | sed 's~.*pushd ${OUTDIR}/.. > /dev/null*~\tpushd ${OUTDIR} > /dev/null~')
}

# Run build_all.sh
run_build_all

# Continue

