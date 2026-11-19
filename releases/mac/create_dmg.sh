#!/bin/bash
#
# Creates a MasterSim release disk image (dmg) for macOS (universal binary: arm64 + x86_64).
#
# Steps: build (deployment mode) -> assemble MasterSim.app -> macdeployqt -> clean rpaths
#        -> codesign -> create dmg -> (optional) notarize + staple
#
# Usage:  releases/mac/create_dmg.sh [options]
#
#   --skip-build            use the existing binaries in bin/release (must be a 'deployment' build!)
#   --skip-test             do not run the regression test suite after building
#   --sign "<identity>"     Developer ID Application identity, e.g.
#                           "Developer ID Application: Andreas Nicolai (TEAMID)"
#                           (default: env MSIM_SIGN_IDENTITY, otherwise ad-hoc signature)
#   --notarize <profile>    notarize/staple the dmg using a notarytool keychain profile
#                           (default: env MSIM_NOTARY_PROFILE; requires --sign)
#
# Environment overrides: AQT_QT_PREFIX (Qt kit, default ~/Qt/6.9.3/macos), MSIM_BUNDLE_ID
#
# Result: releases/mac/dist/MasterSim_<version>_macos_universal.dmg

set -e
set -o pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd -- "${SCRIPT_DIR}/../.." && pwd)"

SKIP_BUILD=0
SKIP_TEST=0
SIGN_IDENTITY="${MSIM_SIGN_IDENTITY:-}"
NOTARY_PROFILE="${MSIM_NOTARY_PROFILE:-}"
BUNDLE_ID="${MSIM_BUNDLE_ID:-io.github.ghorwin.MasterSim}"

while [[ $# -gt 0 ]]; do
	case "$1" in
		--skip-build) SKIP_BUILD=1 ;;
		--skip-test)  SKIP_TEST=1 ;;
		--sign)       SIGN_IDENTITY="$2"; shift ;;
		--notarize)   NOTARY_PROFILE="$2"; shift ;;
		-h|--help)    sed -n '2,22p' "$0"; exit 0 ;;
		*) echo "Unknown option '$1' (see --help)" >&2; exit 1 ;;
	esac
	shift
done

if [[ -n "$NOTARY_PROFILE" && -z "$SIGN_IDENTITY" ]]; then
	echo "ERROR: notarization requires a Developer ID signature (--sign)." >&2
	exit 1
fi

# --- Qt kit ---------------------------------------------------------------
AQT_QT_VERSION="${AQT_QT_VERSION:-6.9.3}"
if [[ -z "$AQT_QT_PREFIX" ]]; then
	for kit in macos clang_64; do
		AQT_QT_PREFIX="$HOME/Qt/${AQT_QT_VERSION}/$kit"
		[[ -d "$AQT_QT_PREFIX" ]] && break
	done
fi
MACDEPLOYQT="$AQT_QT_PREFIX/bin/macdeployqt"
if [[ ! -x "$MACDEPLOYQT" ]]; then
	echo "ERROR: macdeployqt not found at '$MACDEPLOYQT' (set AQT_QT_PREFIX)." >&2
	exit 1
fi
export AQT_QT_PREFIX AQT_QT_VERSION

# --- version ----------------------------------------------------------------
VERSION=$(sed -n 's/.*LONG_VERSION[^"]*"\([^"]*\)".*/\1/p' "$REPO_ROOT/MasterSim/src/MSIM_Constants.cpp")
[[ -n "$VERSION" ]] || { echo "ERROR: cannot read LONG_VERSION from MSIM_Constants.cpp" >&2; exit 1; }
BUILD_VERSION="$VERSION.$(git -C "$REPO_ROOT" rev-list --count HEAD 2>/dev/null || echo 0)"
MIN_MACOS="11.0"
echo "*** MasterSim $VERSION (build $BUILD_VERSION), Qt: $AQT_QT_PREFIX"

# --- 1. build ---------------------------------------------------------------
if [[ $SKIP_BUILD = 0 ]]; then
	echo "*** Building (release, deployment mode, universal binaries)"
	BUILD_ARGS="release deployment"
	[[ $SKIP_TEST = 1 ]] && BUILD_ARGS="$BUILD_ARGS skip-test"
	"$REPO_ROOT/build/build.sh" $BUILD_ARGS
fi

SRC_APP="$REPO_ROOT/bin/release/mastersim-gui.app"
SRC_SOLVER="$REPO_ROOT/bin/release/mastersim"
for f in "$SRC_APP" "$SRC_SOLVER"; do
	[[ -e "$f" ]] || { echo "ERROR: '$f' missing - build first." >&2; exit 1; }
done
for f in "$SRC_APP/Contents/MacOS/mastersim-gui" "$SRC_SOLVER"; do
	if ! lipo -info "$f" | grep -q "x86_64" || ! lipo -info "$f" | grep -q "arm64"; then
		echo "ERROR: '$f' is not a universal binary." >&2; exit 1
	fi
done
# a non-deployment build looks up resources in the source tree - would break in the dmg
if strings "$SRC_APP/Contents/MacOS/mastersim-gui" | grep -q "/../../../../MasterSimulatorUI/resources"; then
	echo "ERROR: bin/release contains a development build, rebuild with 'deployment' (omit --skip-build)." >&2
	exit 1
fi

# --- 2. assemble bundle -------------------------------------------------------
DIST_DIR="$SCRIPT_DIR/dist"
STAGE_DIR="$DIST_DIR/MasterSim-$VERSION"
APP="$STAGE_DIR/MasterSim.app"
DMG="$DIST_DIR/MasterSim_${VERSION}_macos_universal.dmg"

echo "*** Assembling $APP"
rm -rf "$STAGE_DIR" "$DMG"
mkdir -p "$STAGE_DIR"
# ditto preserves symlinks/extended attributes like Finder does
ditto "$SRC_APP" "$APP"
ditto "$SRC_SOLVER" "$APP/Contents/MacOS/mastersim"

sed -e "s/@BUNDLE_ID@/$BUNDLE_ID/" \
	-e "s/@VERSION@/$VERSION/" \
	-e "s/@BUILD_VERSION@/$BUILD_VERSION/" \
	-e "s/@MIN_MACOS@/$MIN_MACOS/" \
	-e "s/@YEAR@/$(date +%Y)/" \
	"$SCRIPT_DIR/Info.plist.in" > "$APP/Contents/Info.plist"
plutil -lint "$APP/Contents/Info.plist" >/dev/null
printf 'APPL????' > "$APP/Contents/PkgInfo"

RES="$APP/Contents/Resources"
mkdir -p "$RES/translations" "$RES/examples"
cp "$REPO_ROOT/MasterSimulatorUI/resources/translations/mastersim-gui_de.qm" "$RES/translations/"
cp "$REPO_ROOT/MasterSimulatorUI/resources/translations/qt_de.qm" "$RES/translations/"
cp "$REPO_ROOT/MasterSimulatorUI/resources/MasterSimulatorUI.icns" "$RES/"
ditto "$REPO_ROOT/data/examples/darwin64" "$RES/examples"
# no simulation results/working directories from the source tree
find "$RES/examples" -name ".DS_Store" -delete
# The example FMUs in data/examples/darwin64 may be old x86_64-only builds. The build creates
# universal (arm64+x86_64) versions of the same test FMUs in data/tests/darwin64 - use these.
find "$RES/examples" -name "*.fmu" | while read -r fmu; do
	rel="${fmu#$RES/examples/}"
	universal="$REPO_ROOT/data/tests/darwin64/$rel"
	if [[ -f "$universal" ]]; then
		cp "$universal" "$fmu"
	else
		echo "WARNING: no universal replacement for example FMU '$rel'"
	fi
done
find "$RES/examples" -type d -name "*-results" -prune -exec rm -rf {} +

# --- 3. Qt frameworks/plugins -------------------------------------------------
echo "*** Running macdeployqt"
"$MACDEPLOYQT" "$APP" -always-overwrite

# remove absolute build-time rpaths (build dir, Qt install), keep @executable_path/../Frameworks
for exe in "$APP/Contents/MacOS/mastersim-gui" "$APP/Contents/MacOS/mastersim"; do
	otool -l "$exe" | awk '/cmd LC_RPATH/{getline; getline; print $2}' | sort -u | while read -r rp; do
		if [[ "$rp" = /* ]]; then
			install_name_tool -delete_rpath "$rp" "$exe" 2>/dev/null || true
		fi
	done
done
if ! otool -l "$APP/Contents/MacOS/mastersim-gui" | grep -q "@executable_path/../Frameworks"; then
	install_name_tool -add_rpath "@executable_path/../Frameworks" "$APP/Contents/MacOS/mastersim-gui"
fi

# sanity check: no references to the build machine left
if otool -L "$APP/Contents/MacOS/"* | grep -E $'^\t' | grep -E "$HOME|/Qt/" ||
   otool -l "$APP/Contents/MacOS/"* | grep -A2 LC_RPATH | grep -E "path /" ; then
	echo "ERROR: binaries still reference absolute library paths (see above)." >&2
	exit 1
fi

# --- 4. code signing ----------------------------------------------------------
# nested code is signed first (inside-out), the bundle last; --deep is deprecated for signing.
if [[ -n "$SIGN_IDENTITY" ]]; then
	echo "*** Signing with '$SIGN_IDENTITY' (hardened runtime)"
	SIGN_ARGS=(--force --timestamp --options runtime --sign "$SIGN_IDENTITY")
else
	echo "*** No signing identity given -> ad-hoc signature (dmg not distributable without Gatekeeper warning)"
	SIGN_ARGS=(--force --sign -)
fi
ENT=(--entitlements "$SCRIPT_DIR/entitlements.plist")

find "$APP/Contents/PlugIns" "$APP/Contents/Frameworks" -type f -name "*.dylib" -print0 2>/dev/null |
	xargs -0 -n 20 codesign "${SIGN_ARGS[@]}"
find "$APP/Contents/Frameworks" -maxdepth 1 -name "*.framework" -print0 |
	xargs -0 -n 1 codesign "${SIGN_ARGS[@]}"
codesign "${SIGN_ARGS[@]}" "${ENT[@]}" "$APP/Contents/MacOS/mastersim"
codesign "${SIGN_ARGS[@]}" "${ENT[@]}" "$APP"
codesign --verify --strict --deep --verbose=2 "$APP"

# --- 5. dmg -------------------------------------------------------------------
echo "*** Creating dmg"
ln -s /Applications "$STAGE_DIR/Applications"
cp "$REPO_ROOT/LICENSE" "$STAGE_DIR/LICENSE.txt"
cp "$REPO_ROOT/doc/manual/MasterSim_manual_en.pdf" "$STAGE_DIR/MasterSim_Manual.pdf"
cp "$REPO_ROOT/doc/manual/MasterSim_manual_de.pdf" "$STAGE_DIR/MasterSim_Handbuch.pdf"
cp "$SCRIPT_DIR/MasterSim - Online Manual.webloc" "$SCRIPT_DIR/MasterSim - Online Handbuch.webloc" "$STAGE_DIR/"
cp "$SCRIPT_DIR/ReadMe-dmg.txt" "$STAGE_DIR/ReadMe.txt"

# ULMO = lzma compressed, read-only (macOS 10.15+)
hdiutil create -quiet -volname "MasterSim $VERSION" -srcfolder "$STAGE_DIR" \
	-fs APFS -format ULMO -ov "$DMG"

if [[ -n "$SIGN_IDENTITY" ]]; then
	codesign --force --timestamp --sign "$SIGN_IDENTITY" "$DMG"
fi

# --- 6. notarization ----------------------------------------------------------
if [[ -n "$NOTARY_PROFILE" ]]; then
	echo "*** Notarizing (this can take a few minutes)"
	xcrun notarytool submit "$DMG" --keychain-profile "$NOTARY_PROFILE" --wait
	xcrun stapler staple "$DMG"
	xcrun stapler validate "$DMG"
	spctl --assess --type open --context context:primary-signature --verbose=2 "$DMG"
fi

hdiutil verify -quiet "$DMG"
echo "*** Created $DMG ($(du -h "$DMG" | cut -f1))"
