#!/usr/bin/env bash
#
# Script that automates Snips software download to Sailfish OS
# Copyright (c) Michal Kosciesza <michal@mkiol.net>
# 
# ---------------------- Important notice --------------------------
# Keep in mind that Snips is not an open source software. The use of
# Snips is is governed by Snips Terms of Use: https://docs.snips.ai/
# additional-resources/legal-and-privacy/website-terms-of-use
# ------------------------------------------------------------------
#
# Example usages:
#
# Download Snips on SFOS to default dir:
# $ ./snips_download.sh
#
# Download Snips to specific dir (can be executed on SFOS or any other machine):
# $ ./snips_download.sh -d <dir>
#
# Check is all needed files exist in specific dir:
# $ ./snips_download.sh -c -d <dir>
#
# Display usage help:
# $ ./snips_download.sh -h
#
# ------------------------------------------------------------------
#
# Only binaries for ARM CPU can be downloade. Jolla Tablet and any
# other non-ARM based devices are not supported right now.
#
# ------------------------------------------------------------------
#
# Script downloads following files:
#
# from https://raspbian.snips.ai/stretch:
#  libsnips_kaldi.so
#  snips-asr
#  snips-dialogue
#  snips-hotword
#  snips-nlu
#  snips-tts
#
# from http://ftp.debian.org/debian:
#  libgfortran.so.3
#  mosquitto
#  libstdc++.so.6
#  libwrap.so.0
#  libcrypto.so.1.1
#  libssl.so.1.1
#  libwebsockets.so.8
#  libnsl.so.1
#  libev.so.4
#  libuv.so.1
#  libatlas.so.3
#  libcblas.so.3
#  libf77blas.so.3
#  liblapack_atlas.so.3
#  bspatch
#  libbz2.so.1.0
#  libttspico.so.0
#  pico2wave
#
# from https://github.com/mkiol:
#  assistant_proj_BAYAr2l4k5z.zip
#

VERSION=1.0.0

INSTALL_SNIPS=1
INSTALL_DEBIAN_MAIN=1
INSTALL_DEBIAN_NONFREE=1
INSTALL_ASSISTANT=1
PATCH_PICO2WAVE=1
KEEP_PACKAGES_FILE=0
KEEP_DEBS=0
KEEP_ASSISTANT=0
CHECK_ONLY=0
REUSE_DEB_FILES=0
ARCH=armhf # Snips provides only binaries for armhf and amd64

SNIPS_DIR_DEFAULT=/home/nemo/.cache/harbour-snipek/harbour-snipek/snips
SNIPS_REPO_ROOT=https://raspbian.snips.ai/stretch
SNIPS_DIST=stable
DEBIAN_REPO_ROOT=http://ftp.debian.org/debian
DEBIAN_DIST=stretch
SNIPEK_ASSISTANT=https://github.com/mkiol/Snipek/raw/master/assistant/assistant_proj_BAYAr2l4k5z.zip

# Binary path for pico2wave. It changes dir where pico2wave expects
# lang files (/usr/share/pico/lang/ => ./pico/lang/)
PICO2WAVE_PATCH="\
QlNESUZGNDA/AAAAAAAAAC4AAAAAAAAAgCYAAAAAAABCWmg5MUFZJlNZh5qAJwAAGWhAeTEAAkhA\
QAAgACGmoZkjQgGgCXNOnSAEMwiAb7rZ8sCv4u5IpwoSEPNQBOBCWmg5MUFZJlNZuZq0BwAAE0Ag\
wAAACAAIIAAwzAUpplGxUeLuSKcKEhczVoDgQlpoOTFBWSZTWWLU0qEAAALQAFAAAAEgACEmQZiQ\
uLuSKcKEgxamlQg="

# ------------------------------------------------------------------

work_dir=$(pwd) # current working dir
declare -a pkgs_to_install_names
declare -a pkgs_to_install_vers
declare -a files_to_install

print() {
  echo "$1"
}

print_error() {
  echo "$1" 1>&2
}

usage() {
  print "Usage: $(basename "$0") [OPTION]..."
  print "Download essential Snips binaries."
  print ""
  print "Options:"
  print "  -d <DIR>     directory where files should be downloaded to (default is $SNIPS_DIR_DEFAULT)"
  print "  -c           instead new download, check if all needed files are present"
  print "  -a           download only assistant"
  print "  -k           keep deb packages and assistant archive file"
  print "  -r           instead download use existing deb packages and assistant archive file"
  print "  -h           display this help and exit"
  print "  -V           output version information and exit"
}

exit_abnormal() {
  local txt="$1"
  local show_usage="$2"
  if [ -n "$txt" ]; then
    print_error "$1"
  fi
  if [ -n "$show_usage" ]; then
    if [ -n "$txt" ]; then
      print
    fi
    usage
  fi
  cd "$work_dir"
  exit 1
}

download() {
  local url=$1
  local file="$2"
  
  if ! curl -L --retry 1 -o "$file" "$url"; then
    return 1
  fi
  if [ ! -f "$file" ]; then
    return 1
  fi
}

download_and_extract_assistant() {
  print "Downloading assistant file: $SNIPEK_ASSISTANT"
  
  local assistant_archive="$SNIPS_DIR/assistant.zip"
  local assistant_dir="$SNIPS_DIR/assistant"

  if [ $REUSE_DEB_FILES -eq 1 ] && [ -f "$assistant_archive" ]; then
    print "No need to download assistant file because it exists."
  else
    if ! download "$SNIPEK_ASSISTANT" "$assistant_archive"; then
      print_error "Error: Cannot download assistant file."
      return 1
    fi
  fi
  
  if ! unzip -o "$assistant_archive" -d "$SNIPS_DIR" || [ ! -d "$assistant_dir" ]; then
    print_error "Error: Cannot extract assistant file."
    
    if [ $KEEP_ASSISTANT -eq 0 ] && [ -f "$assistant_archive" ]; then
      print "Deleting file: $assistant_archive"
      rm "$assistant_archive"
    fi
    
    return 1
  fi
  
  if [ $KEEP_ASSISTANT -eq 0 ] && [ -f "$assistant_archive" ]; then
    print "Deleting file: $assistant_archive"
    rm "$assistant_archive"
  fi
}

download_and_extract_pkgs() {
  local repo_root="$1"
  local package_file_url="$2"
  
  if [ ${#pkgs_to_install_names[*]} -eq 0 ]; then
    print_error "No packages to install."
    return 1
  fi

  # download debian repo Package file

  print "Downloading repo Packages file: $package_file_url"
  
  local packages_archive="$SNIPS_DIR/Packages.gz"
  local packages_file="$SNIPS_DIR/Packages"
  
  if ! download "$package_file_url" "$packages_archive"; then
    print_error "Error: Cannot download repo Packages file."
    return 1
  fi
  
  if ! gzip -d -f "$packages_archive" || [ ! -f "$packages_file" ]; then
    print_error "Error: Cannot extract Packages file."
    return 1
  fi

  pkg_names=( $( sed -r 's/^Package: (.*)$/\1/g;t;d' "$packages_file" ) )
  pkg_vers=( $( sed -r 's/^Version: (.*)$/\1/g;t;d' "$packages_file" ) )
  pkg_fnames=( $( sed -r 's/^Filename: (.*)$/\1/g;t;d' "$packages_file" ) )
  
  if [ $KEEP_PACKAGES_FILE -eq 0 ]; then
    echo "Deleting file: $packages_file"
    rm "$packages_file"
  fi

  # find packages for install

  declare -a found_pkg_names
  declare -a found_pkg_idx # idx of found pkg in pkg_names

  print "Needed packages:"
  for idx in "${!pkgs_to_install_names[@]}"
  do
    echo " ${pkgs_to_install_names[$idx]} (${pkgs_to_install_vers[$idx]})"
  done

  print "Finding needed packages in the repo."
  
  for idx in "${!pkgs_to_install_names[@]}"
  do
    local needed_name="${pkgs_to_install_names[$idx]}"
    local needed_ver="${pkgs_to_install_vers[$idx]}"
    print "Searching for $needed_name ($needed_ver)..."
    
    for idx in ${!pkg_names[@]}
    do
      if [ "${pkg_names[$idx]}" == "$needed_name" ] && [[ "${pkg_vers[$idx]}" == "$needed_ver"* ]]; then
          found_pkg_names+=("${pkg_names[$idx]}")
          found_pkg_idx+=("$idx")
          break
      fi
    done
  done

  # check if all packages found
  
  local found1=1
  for name_needed in "${pkgs_to_install_names[@]}"
  do
    local found2=0
    for name_found in "${found_pkg_names[@]}"
    do
      if [ "$name_needed" == "$name_found" ]; then
        found2=1
        print "Found package: $name_found"
        break
      fi
    done
    if [ $found2 -eq 0 ]; then
      found1=0
      print_error "Error: Did not find package: $name_needed"
    fi
  done
  if [ $found1 -ne 1 ]; then
    print_error "Error: Some packages are missing."
    return 1
  fi

  print "All packages have been found in the repo."

  # download needed packages

  for idx in "${!found_pkg_names[@]}"
  do
    local name=${found_pkg_names[$idx]}
    local pkg_idx=${found_pkg_idx[$idx]}
    local url="$repo_root/${pkg_fnames[$pkg_idx]}"
    local file="$SNIPS_DIR/$name.deb"
    if [ $REUSE_DEB_FILES -eq 1 ] && [ -f "$file" ]; then
      print "No need to download package $name because it exists."
    else
      print "Downloading package $name from $url."
      if ! download "$url" "$file"; then
        print_error "Error: Cannot download package $name from $url."
        return 1
      fi
    fi
  done

  print "All deb packages were downloaded."

  # extract deb packages

  for name in "${found_pkg_names[@]}"
  do
    local deb_file="$SNIPS_DIR/$name.deb"
    local dir="$SNIPS_DIR/deb-$name"
    
    print "Extracting $deb_file to $dir dir."
    
    if ! mkdir -p "$dir"; then
      print_error "Error: Cannot make $dir."
      return 1
    fi
    
    cd "$dir"
    
    if ! ar x "$deb_file"; then
      print_error "Error: Cannot ar $deb_file."
      cd "$work_dir"
      return 1
    fi
    
    local data_file_xz="$dir/data.tar.xz"
    local data_file_tar="$dir/data.tar"
    
    print "Extracting $data_file_xz to $dir dir."
    
    if ! xz -f -d "$data_file_xz"; then
      print_error "Error: Cannot extract $data_file_xz."
      cd "$work_dir"
      return 1
    fi
    
    if ! tar -xf "$data_file_tar" --directory "$dir"; then
      print_error "Error: Cannot extract $data_file_tar."
      cd "$work_dir"
      return 1
    fi
    
    cd "$work_dir"
  done
  
  print "All deb packages were extracted."
}

clean_pkgs() {
  for name in "${pkgs_to_install_names[@]}"
  do
    local deb_file="$SNIPS_DIR/$name.deb"
    local dir="$SNIPS_DIR/deb-$name"
    if [ $KEEP_DEBS -eq 0 ] && [ -f "$deb_file" ]; then
      print "Deleting file: $deb_file"
      rm "$deb_file"
    fi
    if [ -d "$dir" ]; then
      print "Deleting dir: $dir"
      rm -R "$dir"
    fi
  done
  pkgs_to_install_names=()
  pkgs_to_install_vers=()
}

check_files() {
  local error=0
  for file in "${files_to_install[@]}"
  do
    if [ ! -f "$SNIPS_DIR/$(basename "$file")" ]; then
      print_error "Error: File $file does not exist."
      error=1
    fi
  done
  files_to_install=()
  return $error
}

# parse options

while getopts ":Vkhrcad:" options; do
case "${options}" in
  h)
    usage
    exit 0
    ;;
  V)
    print "$VERSION"
    exit 0
    ;;
  d)
    SNIPS_DIR="${OPTARG}"
    ;;
  c)
    CHECK_ONLY=1
    ;;
  a)
    INSTALL_SNIPS=0
    INSTALL_DEBIAN_MAIN=0
    INSTALL_DEBIAN_NONFREE=0
    INSTALL_ASSISTANT=1
    PATCH_PICO2WAVE=0
    ;;
  k)
    KEEP_DEBS=1
    KEEP_ASSISTANT=1
    ;;
  r)
    REUSE_DEB_FILES=1
    ;;
  :)
    exit_abnormal "Error: Option -$OPTARG requires an argument." 1
    ;;
  ?)
    exit_abnormal "Error: Unknown option -$OPTARG." 1
    ;;
  esac
done

# check if script is executed on SFOS ARM device

cpu_arch=$(uname -p) # machine cpu arch
print "CPU arch is $cpu_arch."

if [ $cpu_arch == "aarch64" ] || [[ $cpu_arch == *arm* ]]; then
  sfos_arm=1
else
  sfos_arm=0
fi

# check needed shell commands

needed_commands=( curl sed ar tar gzip basename readlink dirname base64 xz )
if [ $sfos_arm -ne 1 ]; then
  needed_commands+=( bspatch )
fi

error=0
for cmd in "${needed_commands[@]}"
do
  if ! [ -x "$(command -v "$cmd")" ]; then
    if [ $cmd == "ar" ]; then
      print_error "Error: $cmd is required but missing. Install binutils package."
    elif [ $cmd == "curl" ]; then
      print_error "Error: $cmd is required but missing. Install curl package."
    else
      print_error "Error: $cmd is required but not installed."
    fi
    error=1
  fi
done
if [ $error -ne 0 ]; then
  exit_abnormal
fi

# check user dir, if not provided default to <snipek cache>/snips

if [ -z "$SNIPS_DIR" ] && [ $sfos_arm -eq 1 ]; then
  print "Using default directory for download: $SNIPS_DIR_DEFAULT."
  if [ ! -d "$SNIPS_DIR_DEFAULT" ]; then
    if [ $CHECK_ONLY -eq 1 ]; then
      exit_abnormal "Error: Directory $SNIPS_DIR_DEFAULT does not exist." 1
    fi
    print "Creating $SNIPS_DIR_DEFAULT dir."
    if ! mkdir -p "$SNIPS_DIR_DEFAULT"; then
      exit_abnormal "Error: Cannot create $SNIPS_DIR_DEFAULT dir."
    fi
  fi
  SNIPS_DIR="$SNIPS_DIR_DEFAULT"
elif [ ! -d "$SNIPS_DIR" ]; then
  if [ -z "$SNIPS_DIR" ]; then
    exit_abnormal "Error: Directory was not set." 1
  else
    exit_abnormal "Error: Directory $SNIPS_DIR does not exist." 1
  fi
else
  SNIPS_DIR="$(readlink -f "$SNIPS_DIR")"
fi

# download binaries

if [ $ARCH == "armhf" ]; then
  lib_dir="arm-linux-gnueabihf"
elif [ $ARCH == "i386" ]; then
  lib_dir="i386-linux-gnu"
elif [ $ARCH == "amd64" ]; then
  lib_dir="amd64-linux-gnu"
else
  exit_abnormal "Error: Unknown architecture."
fi

# check only option

if [ $CHECK_ONLY -eq 1 ]; then
  print "Check if all needed files are present."
  
  files_to_install+=("$SNIPS_DIR/libsnips_kaldi.so")
  files_to_install+=("$SNIPS_DIR/snips-asr")
  files_to_install+=("$SNIPS_DIR/snips-dialogue")
  files_to_install+=("$SNIPS_DIR/snips-hotword")
  files_to_install+=("$SNIPS_DIR/snips-nlu")
  files_to_install+=("$SNIPS_DIR/snips-tts")
  
  files_to_install+=("$SNIPS_DIR/libgfortran.so.3")
  files_to_install+=("$SNIPS_DIR/mosquitto")
  files_to_install+=("$SNIPS_DIR/libstdc++.so.6")
  files_to_install+=("$SNIPS_DIR/libwrap.so.0")
  files_to_install+=("$SNIPS_DIR/libcrypto.so.1.1")
  files_to_install+=("$SNIPS_DIR/libssl.so.1.1")
  files_to_install+=("$SNIPS_DIR/libwebsockets.so.8")
  files_to_install+=("$SNIPS_DIR/libnsl.so.1")
  files_to_install+=("$SNIPS_DIR/libev.so.4")
  files_to_install+=("$SNIPS_DIR/libuv.so.1")
  files_to_install+=("$SNIPS_DIR/libatlas.so.3")
  files_to_install+=("$SNIPS_DIR/libcblas.so.3")
  files_to_install+=("$SNIPS_DIR/libf77blas.so.3")
  files_to_install+=("$SNIPS_DIR/liblapack_atlas.so.3")
  
  files_to_install+=("$SNIPS_DIR/libttspico.so.0")
  files_to_install+=("$SNIPS_DIR/pico2wave")
  
  if ! check_files; then
    exit_abnormal "Error: Some binaries are missing."
  fi
  
  if [ ! -f "$SNIPS_DIR/pico/lang/en-US_ta.bin" ]; then
    exit_abnormal "Error: File $SNIPS_DIR/pico/lang/en-US_ta.bin does not exist."
  fi
  
  if [ ! -f "$SNIPS_DIR/assistant/assistant.json" ]; then
    exit_abnormal "Error: File $SNIPS_DIR/assistant/assistant.json does not exist."
  fi
  
  files_to_install=()
  print "Done. All needed files are present in $SNIPS_DIR dir."
  exit 0
fi

print "Ready to start download for $ARCH cpu arch to $SNIPS_DIR directory."

if [ $INSTALL_SNIPS -ne 0 ]; then  # Snips installation
  print "Downloading Snips binaries..."

  pkgs_to_install_names+=(snips-hotword)
  pkgs_to_install_vers+=("0.64.0")
  pkgs_to_install_names+=(snips-asr)
  pkgs_to_install_vers+=("0.64.0")
  pkgs_to_install_names+=(snips-nlu)
  pkgs_to_install_vers+=("0.64.0")
  pkgs_to_install_names+=(snips-tts)
  pkgs_to_install_vers+=("0.64.0")
  pkgs_to_install_names+=(snips-dialogue)
  pkgs_to_install_vers+=("0.64.0")
  pkgs_to_install_names+=(snips-platform-common)
  pkgs_to_install_vers+=("0.64.0")
  pkgs_to_install_names+=(snips-kaldi-atlas)
  pkgs_to_install_vers+=("0.26.1")
  
  packages_url=$SNIPS_REPO_ROOT/dists/$SNIPS_DIST/main/binary-$ARCH/Packages.gz
  
  if ! download_and_extract_pkgs $SNIPS_REPO_ROOT $packages_url; then
    exit_abnormal
  fi
  
  files_to_install+=("$SNIPS_DIR/deb-snips-kaldi-atlas/usr/lib/libsnips_kaldi.so")
  files_to_install+=("$SNIPS_DIR/deb-snips-asr/usr/bin/snips-asr")
  files_to_install+=("$SNIPS_DIR/deb-snips-dialogue/usr/bin/snips-dialogue")
  files_to_install+=("$SNIPS_DIR/deb-snips-hotword/usr/bin/snips-hotword")
  files_to_install+=("$SNIPS_DIR/deb-snips-nlu/usr/bin/snips-nlu")
  files_to_install+=("$SNIPS_DIR/deb-snips-tts/usr/bin/snips-tts")
  for file in "${files_to_install[@]}"
  do
    cp --dereference "$file" "$SNIPS_DIR/"
  done
  
  clean_pkgs
  
  if ! check_files; then
    exit_abnormal "Error: Some binaries are missing."
  fi
  
  files_to_install=()
fi

if [ $INSTALL_DEBIAN_MAIN -ne 0 ]; then  # Debian main installation
  print "Downloading Debian main binaries..."

  pkgs_to_install_names+=(libgfortran3)
  pkgs_to_install_vers+=("6.3.0-18")
  pkgs_to_install_names+=(mosquitto)
  pkgs_to_install_vers+=("1.4.10-3")
  pkgs_to_install_names+=("libstdc++6")
  pkgs_to_install_vers+=("6.3.0-18")
  pkgs_to_install_names+=(libwrap0)
  pkgs_to_install_vers+=("7.6.q-26")
  pkgs_to_install_names+=(libssl1.1)
  pkgs_to_install_vers+=("1.1.0k-1")
  pkgs_to_install_names+=(libwebsockets8)
  pkgs_to_install_vers+=("2.0.3-2")
  pkgs_to_install_names+=(libc6)
  pkgs_to_install_vers+=("2.24-11")
  pkgs_to_install_names+=(libev4)
  pkgs_to_install_vers+=("1:4.22-1")
  pkgs_to_install_names+=(libuv1)
  pkgs_to_install_vers+=("1.9.1-3")
  pkgs_to_install_names+=(libatlas3-base)
  pkgs_to_install_vers+=("3.10.3-1")
  if [ $PATCH_PICO2WAVE -eq 1 ] && [ $ARCH == "armhf" ]; then  # needed to patch pico2wave
    pkgs_to_install_names+=(bsdiff)
    pkgs_to_install_vers+=("4.3-19")
    pkgs_to_install_names+=("libbz2-1.0")
    pkgs_to_install_vers+=("1.0.6-8.1")
  fi
  
  packages_url=$DEBIAN_REPO_ROOT/dists/$DEBIAN_DIST/main/binary-$ARCH/Packages.gz
  
  if ! download_and_extract_pkgs $DEBIAN_REPO_ROOT $packages_url; then
    exit_abnormal
  fi

  files_to_install+=("$SNIPS_DIR/deb-libgfortran3/usr/lib/$lib_dir/libgfortran.so.3")
  files_to_install+=("$SNIPS_DIR/deb-mosquitto/usr/sbin/mosquitto")
  files_to_install+=("$SNIPS_DIR/deb-libstdc++6/usr/lib/$lib_dir/libstdc++.so.6")
  files_to_install+=("$SNIPS_DIR/deb-libwrap0/lib/$lib_dir/libwrap.so.0")
  files_to_install+=("$SNIPS_DIR/deb-libssl1.1/usr/lib/$lib_dir/libcrypto.so.1.1")
  files_to_install+=("$SNIPS_DIR/deb-libssl1.1/usr/lib/$lib_dir/libssl.so.1.1")
  files_to_install+=("$SNIPS_DIR/deb-libwebsockets8/usr/lib/$lib_dir/libwebsockets.so.8")
  files_to_install+=("$SNIPS_DIR/deb-libc6/lib/$lib_dir/libnsl.so.1")
  files_to_install+=("$SNIPS_DIR/deb-libev4/usr/lib/$lib_dir/libev.so.4")
  files_to_install+=("$SNIPS_DIR/deb-libuv1/usr/lib/$lib_dir/libuv.so.1")
  files_to_install+=("$SNIPS_DIR/deb-libatlas3-base/usr/lib/libatlas.so.3")
  files_to_install+=("$SNIPS_DIR/deb-libatlas3-base/usr/lib/libcblas.so.3")
  files_to_install+=("$SNIPS_DIR/deb-libatlas3-base/usr/lib/libf77blas.so.3")
  files_to_install+=("$SNIPS_DIR/deb-libatlas3-base/usr/lib/liblapack_atlas.so.3")
  if [ $PATCH_PICO2WAVE -eq 1 ] && [ $ARCH == "armhf" ]; then
    files_to_install+=("$SNIPS_DIR/deb-bsdiff/usr/bin/bspatch")
    files_to_install+=("$SNIPS_DIR/deb-libbz2-1.0/lib/$lib_dir/libbz2.so.1.0")
  fi

  for file in "${files_to_install[@]}"
  do
    cp --dereference "$file" "$SNIPS_DIR/"
  done

  clean_pkgs
  
  if ! check_files; then
    exit_abnormal "Error: Some binaries are missing."
  fi
  
  files_to_install=()
fi

if [ $INSTALL_DEBIAN_NONFREE -ne 0 ]; then  # Debian non-free installation
  print "Downloading Debian non-free binaries..."

  pkgs_to_install_names+=(libttspico0)
  pkgs_to_install_vers+=("1.0")
  pkgs_to_install_names+=(libttspico-utils)
  pkgs_to_install_vers+=("1.0")
  pkgs_to_install_names+=(libttspico-data)
  pkgs_to_install_vers+=("1.0")

  packages_url=$DEBIAN_REPO_ROOT/dists/$DEBIAN_DIST/non-free/binary-$ARCH/Packages.gz
  
  if ! download_and_extract_pkgs $DEBIAN_REPO_ROOT $packages_url; then
    exit_abnormal
  fi

  files_to_install+=("$SNIPS_DIR/deb-libttspico0/usr/lib/$lib_dir/libttspico.so.0")
  files_to_install+=("$SNIPS_DIR/deb-libttspico-utils/usr/bin/pico2wave")
  for file in "${files_to_install[@]}"
  do
    cp --dereference "$file" "$SNIPS_DIR/"
  done
  cp -R "$SNIPS_DIR/deb-libttspico-data/usr/share/pico" "$SNIPS_DIR/"
  
  clean_pkgs
  
  if ! check_files; then
    exit_abnormal "Error: Some binaries are missing."
  fi
  if [ ! -f "$SNIPS_DIR/pico/lang/en-US_ta.bin" ]; then
    exit_abnormal "Error: File $SNIPS_DIR/pico/lang/en-US_ta.bin does not exist."
  fi
  files_to_install=()
fi

if [ $INSTALL_ASSISTANT -ne 0 ]; then  # Assistant installation
  print "Downloading assistant file..."
  if ! download_and_extract_assistant; then
    exit_abnormal
  fi
fi

# Patch pico2wave only for armhf because only armhf patch is available

if [ $PATCH_PICO2WAVE -eq 1 ] && [ $ARCH == "armhf" ]; then
  print "Applying patch on pico2wave."
  
  pico2wave_file="$SNIPS_DIR/pico2wave"
  pico2wave_patched="$SNIPS_DIR/pico2wave_patched"
  pico2wave_patch_file="$SNIPS_DIR/pico2wave.patch"
  bspatch_cmd="bspatch"

  if [ $sfos_arm -eq 1 ]; then # script executed on arm device, so using own bspatch
    bspatch_cmd="$SNIPS_DIR/bspatch"
    if ! [ -x "$(command -v "$bspatch_cmd")" ]; then
      exit_abnormal "Error: Cannot patch pico2wave because $bspatch_cmd is required but not available."
    fi
  fi
  
  if ! echo "$PICO2WAVE_PATCH" | base64 -d -- > "$pico2wave_patch_file"; then
    exit_abnormal "Error: Cannot create pico2wave patch file."
  fi
  
  if [ $sfos_arm -eq 1 ]; then
    export LD_LIBRARY_PATH=$SNIPS_DIR
  fi
  
  if ! "$bspatch_cmd" "$pico2wave_file" "$pico2wave_patched" "$pico2wave_patch_file"; then
    rm "$pico2wave_patch_file"
    exit_abnormal "Error: Cannot apply patch on pico2wave."
  fi
  
  if [ $sfos_arm -eq 1 ]; then
    export -n LD_LIBRARY_PATH
  fi

  rm "$pico2wave_patch_file"
  rm "$pico2wave_file"
  
  if ! mv "$pico2wave_patched" "$pico2wave_file"; then
    exit_abnormal "Error: Cannot apply patch on pico2wave."
  fi
  
  chmod a+x "$pico2wave_file"
  
  if [ ! -f "$pico2wave_file" ]; then
    exit_abnormal "Error: Cannot apply patch on pico2wave."
  fi
  
  print "Pico2wave patched successfully."
fi

print "Done. All files were successfully downloaded to $SNIPS_DIR."
