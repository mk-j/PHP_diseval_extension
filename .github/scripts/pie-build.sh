#!/usr/bin/env bash
# Build, test and package one PIE pre-packaged binary into $OUT_DIR (default: dist/).
# The zip is named the way PIE looks for it:
#   php_<ext>-<tag>_php<X.Y>-<arch>-<os>-<libc>[-debug][-zts].zip
# Used by ci.yml (TAG=ci) and release.yml. This script needs no token: uploading is
# done by a separate job in release.yml, so build jobs never hold write access.
# env: TAG (version used in the asset name), OUT_DIR (optional),
#      EXPECT_ZTS (optional, true/false: fail if the PHP build's thread safety differs,
#      so a mislabelled image can't silently overwrite another job's binary).
set -euo pipefail

: "${TAG:?TAG must be set}"
OUT_DIR=${OUT_DIR:-dist}

# JSON is parsed with php itself (no jq: not installable on some old php:* images).
json() { php -r '$d = json_decode(stream_get_contents(STDIN), true); '"$1"; }
EXT_NAME=$(json 'echo $d["php-ext"]["extension-name"];' < composer.json)
BUILD_PATH=$(json 'echo $d["php-ext"]["build-path"] ?? ".";' < composer.json)
PHP_VER=$(php -r 'echo PHP_MAJOR_VERSION . "." . PHP_MINOR_VERSION;')
case "$(uname -m)" in
  x86_64|amd64)  ARCH=x86_64 ;;
  aarch64|arm64) ARCH=arm64 ;;
  *)             ARCH=$(uname -m) ;;
esac
case "$(uname -s)" in
  Darwin) OS=darwin; LIBC=bsdlibc; NPROC=$(sysctl -n hw.ncpu) ;;
  *)      OS=linux; NPROC=$(nproc)
          # Alpine's musl ldd prints to stderr and exits 1: merge streams, ignore status.
          LDD_OUT=$(ldd --version 2>&1 || true)
          if printf "%s" "$LDD_OUT" | grep -qi musl; then LIBC=musl; else LIBC=glibc; fi ;;
esac
ZTS_SUFFIX=$(php -r 'echo PHP_ZTS ? "-zts" : "";')
IS_ZTS=$(php -r 'echo PHP_ZTS ? "true" : "false";')
if [ -n "${EXPECT_ZTS:-}" ] && [ "$EXPECT_ZTS" != "$IS_ZTS" ]; then
  echo "ERROR: expected ZTS=$EXPECT_ZTS but PHP $(php -r 'echo PHP_VERSION;') is ZTS=$IS_ZTS" >&2
  exit 1
fi
DEBUG_SUFFIX=$(php -r 'echo PHP_DEBUG ? "-debug" : "";')
ASSET="php_${EXT_NAME}-${TAG}_php${PHP_VER}-${ARCH}-${OS}-${LIBC}${DEBUG_SUFFIX}${ZTS_SUFFIX}.zip"
echo "Target asset: $ASSET"

(cd "$BUILD_PATH" && phpize && ./configure --enable-"${EXT_NAME}" && make -j"$NPROC")
bash testing/run-tests.sh php "$PWD/$BUILD_PATH/modules/${EXT_NAME}.so"

mkdir -p "$OUT_DIR"
OUT="$(cd "$OUT_DIR" && pwd)/$ASSET"
rm -f "$OUT"
(cd "$BUILD_PATH/modules" && zip -j "$OUT" "${EXT_NAME}.so")
echo "Built $OUT"
if [ -n "${GITHUB_ENV:-}" ]; then echo "ASSET=$ASSET" >> "$GITHUB_ENV"; fi
