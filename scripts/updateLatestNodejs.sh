#!/usr/bin/env bash
# Install the latest Node.js, rebuild ibm_db, and update the optional binary repo.
# Usage: ./updateLatestNodejs.sh
# Only POSIX options of grep/sed/tar are used so that it also works with AIX native tools.

set -o pipefail

CURR_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IBMDB_DIR="$(dirname "$CURR_DIR")"
BINARY_DIR="$(dirname "$IBMDB_DIR")/ibmdb-binaries"
NODEWORK="$(dirname "$(dirname "$(dirname "$CURR_DIR")")")"
NODE_COMMAND="$(command -v node 2>/dev/null || true)"
if [[ -n "$NODE_COMMAND" ]] && node -v >/dev/null 2>&1; then
  NODE_BIN_DIR="$(dirname "$NODE_COMMAND")"
  if [[ "$(basename "$NODE_BIN_DIR")" == "bin" ]]; then
    NODEJS_DIR="$(dirname "$NODE_BIN_DIR")"
  else
    NODEJS_DIR="$NODE_BIN_DIR"
  fi
  if [[ "$(basename "$NODEJS_DIR")" != "nodejs" ]]; then
    RESOLVED_NODE="$(readlink -f "$NODE_COMMAND" 2>/dev/null || printf '%s' "$NODE_COMMAND")"
    NODE_BIN_DIR="$(dirname "$RESOLVED_NODE")"
    if [[ "$(basename "$NODE_BIN_DIR")" == "bin" ]]; then
      NODEJS_DIR="$(dirname "$NODE_BIN_DIR")"
    else
      NODEJS_DIR="$NODE_BIN_DIR"
    fi
  fi
  if [[ "$(basename "$NODEJS_DIR")" == "nodejs" ]]; then
    NODEWORK="$(dirname "$NODEJS_DIR")"
  fi
fi
echo "Node work directory = $NODEWORK"

if [[ ! -e "$IBMDB_DIR/installer/driverInstall.js" ]]; then
  echo "Error: unable to find ibm_db directory at $IBMDB_DIR"
  exit 1
fi

osname=$(uname)
arch=$(uname -m)
# AIX reports the hardware id in uname -m.
[[ "$osname" == "AIX" ]] && arch="ppc64"

installHint() {
  case "$osname" in
    AIX)
      echo "  Install with AIX Toolbox (https://www.ibm.com/support/pages/aix-toolbox-open-source-software-overview):"
      echo "    yum install -y $1   (or: dnf install -y $1)" ;;
    Darwin)
      echo "  Install with: brew install $1" ;;
    *)
      echo "  Install with one of:"
      echo "    sudo yum install -y $1 | sudo dnf install -y $1 | sudo apt-get install -y $1 | sudo zypper install -y $1" ;;
  esac
}

MISSING=false
for cmd in curl gunzip tar grep sed awk head make; do
  if ! command -v $cmd &> /dev/null; then
    pkg=$cmd
    [[ "$cmd" == "gunzip" ]] && pkg="gzip"
    echo "Required command '$cmd' not found."
    installHint "$pkg"
    MISSING=true
  fi
done
if [[ "$osname" == "AIX" ]] && ! command -v gmake &> /dev/null; then
  echo "Required command 'gmake' not found (needed by node-gyp on AIX)."
  installHint "make"
  MISSING=true
fi
if ! command -v python3 &> /dev/null; then
  echo "Required command 'python3' not found (needed by node-gyp)."
  installHint "python3"
  MISSING=true
fi
if ! command -v g++ &> /dev/null && ! command -v c++ &> /dev/null && ! command -v xlC_r &> /dev/null; then
  echo "A C++ compiler was not found (needed by node-gyp)."
  installHint "gcc-c++"
  MISSING=true
fi
$MISSING && exit 1
ARCHIVE_PATTERN=""
BINARY_NAME=""
case "$osname" in
  Darwin)
    if [[ "$arch" == "arm64" ]]; then
      ARCHIVE_PATTERN="darwin-arm64.tar.gz"
      BINARY_NAME="odbc_bindings_macarm.node"
      if command -v brew &> /dev/null; then
        export PYTHON="$(brew --prefix python@3.11)/bin/python3.11"
      fi
    else
      ARCHIVE_PATTERN="darwin-x64.tar.gz"
      BINARY_NAME="odbc_bindings_mac.node"
    fi
    ;;
  Linux)
    if [[ "$arch" == "x86_64" ]]; then
      ARCHIVE_PATTERN="linux-x64.tar.gz"
      BINARY_NAME="odbc_bindings_linux.node"
    elif [[ "$arch" == "aarch64" || "$arch" == "arm64" ]]; then
      ARCHIVE_PATTERN="linux-arm64.tar.gz"
      BINARY_NAME="odbc_bindings_linuxarm.node"
    elif [[ "$arch" == "s390x" ]]; then
      ARCHIVE_PATTERN="linux-s390x.tar.gz"
      BINARY_NAME="odbc_bindings_zlinux.node"
    fi
    ;;
  AIX)
    ARCHIVE_PATTERN="aix-ppc64.tar.gz"
    BINARY_NAME="odbc_bindings_aix.node"
    ;;
esac
if [[ -z "$BINARY_NAME" ]]; then
  echo "$arch for $osname platform is not supported."
  exit 1
fi

# Find the latest Node.js release
BASE_URL="https://nodejs.org/download/release/latest/"
TARBALL=$(curl -fs "$BASE_URL" | sed -n "s/.*\(node-v[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*-$ARCHIVE_PATTERN\).*/\1/p" | head -n 1)
if [[ -z "$TARBALL" ]]; then
  echo "Failed to detect latest Node.js version."
  exit 1
fi
LATEST_VERSION=$(echo "$TARBALL" | sed "s/^node-\(v[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\)-.*/\1/")
NODEDIR="$NODEWORK/node$LATEST_VERSION"
echo "Latest Node.js = $LATEST_VERSION"

ACTIVE_DIR="$NODEWORK/nodejs"
ACTIVE_VERSION=""
if [[ -x "$ACTIVE_DIR/bin/node" ]]; then
  ACTIVE_VERSION=$("$ACTIVE_DIR/bin/node" -v 2>/dev/null)
fi

if [[ "$ACTIVE_VERSION" == "$LATEST_VERSION" ]]; then
  NODEDIR="$ACTIVE_DIR"
  echo "$ACTIVE_DIR is already running the latest Node.js."
elif [[ -x "$NODEDIR/bin/node" ]]; then
  echo "Found $NODEDIR"
else
  echo "Downloading ${BASE_URL}${TARBALL}"
  cd "$NODEWORK" || exit 1
  if ! curl -fLO "${BASE_URL}${TARBALL}"; then
    echo "Download failed!"
    exit 1
  fi
  if ! gunzip -c "$TARBALL" | tar xf -; then
    echo "Extraction failed!"
    exit 1
  fi
  if ! mv "${TARBALL%.tar.gz}" "$NODEDIR"; then
    echo "Failed to install Node.js into $NODEDIR"
    exit 1
  fi
  rm -f "$TARBALL"
  cd "$CURR_DIR" || exit 1
fi

if [[ "$NODEDIR" != "$ACTIVE_DIR" ]]; then
  if [[ -e "$ACTIVE_DIR" ]]; then
    OLD_VERSION="$ACTIVE_VERSION"
    if [[ -z "$OLD_VERSION" ]]; then
      echo "Error: cannot determine the version of $ACTIVE_DIR."
      exit 1
    fi
    PRESERVED_DIR="$NODEWORK/node$OLD_VERSION"
    if [[ -e "$PRESERVED_DIR" ]]; then
      PRESERVED_DIR="$NODEWORK/node${OLD_VERSION}.previous"
      suffix=1
      while [[ -e "$PRESERVED_DIR" ]]; do
        PRESERVED_DIR="$NODEWORK/node${OLD_VERSION}.previous.$suffix"
        suffix=$((suffix + 1))
      done
    fi
    mv "$ACTIVE_DIR" "$PRESERVED_DIR" || exit 1
    echo "Moved old nodejs to $PRESERVED_DIR"
  fi
  if ! mv "$NODEDIR" "$ACTIVE_DIR"; then
    echo "Failed to activate latest Node.js from $NODEDIR"
    exit 1
  fi
  NODEDIR="$ACTIVE_DIR"
  echo "Latest node $LATEST_VERSION is now $ACTIVE_DIR"
fi

export PATH="$NODEDIR/bin:$PATH"
if [[ "$(node -v)" != "$LATEST_VERSION" ]]; then
  echo "Error: expected Node.js $LATEST_VERSION, got $(node -v)."
  exit 1
fi
echo "Using node $LATEST_VERSION"

PATTERN="${osname} ${arch} NAPI Node Version = "

#export IBM_DB_HOME=
#export DOWNLOAD_CLIDRIVER=true
cd "$IBMDB_DIR" || exit 1
if ! npm install; then
  echo "npm install failed!"
  exit 1
fi

BUILT="$IBMDB_DIR/build/Release/odbc_bindings.node"
if [[ ! -f "$BUILT" ]]; then
  echo "Error: $BUILT was not created."
  exit 1
fi

if [[ -d "$BINARY_DIR" ]]; then
  VERSION_FILE="$BINARY_DIR/binaryVersions.txt"
  if ! cp "$BUILT" "$BINARY_DIR/$BINARY_NAME"; then
    echo "Failed to copy $BUILT to $BINARY_DIR/$BINARY_NAME"
    exit 1
  fi
  echo "Copied $BUILT to $BINARY_DIR/$BINARY_NAME"

  if [[ -f "$VERSION_FILE" ]] && grep -q "^${PATTERN}" "$VERSION_FILE"; then
    TMP_FILE=$(mktemp) || exit 1
    if ! awk -v p="$PATTERN" -v v="$LATEST_VERSION" \
      'index($0, p) == 1 { print p v; next } { print }' "$VERSION_FILE" > "$TMP_FILE"; then
      rm -f "$TMP_FILE"
      echo "Failed to update $VERSION_FILE"
      exit 1
    fi
    if ! cat "$TMP_FILE" > "$VERSION_FILE"; then
      rm -f "$TMP_FILE"
      echo "Failed to update $VERSION_FILE"
      exit 1
    fi
    rm -f "$TMP_FILE"
  else
    printf '%s%s\n' "$PATTERN" "$LATEST_VERSION" >> "$VERSION_FILE" || exit 1
  fi
else
  echo "Binary repository not found at $BINARY_DIR; skipping binary copy."
fi

cd "$CURR_DIR" || exit 1
echo "Done!"
