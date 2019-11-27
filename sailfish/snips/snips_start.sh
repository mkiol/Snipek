#!/usr/bin/env bash

# Script that starts essential Snips components on Sailfish OS
# Copyright (c) Michal Kosciesza <michal@mkiol.net>
#
# Project website: https://github.com/mkiol/Snipek
#
# Usage:
#
# Start Snips that is downloaded to default dir:
# $ ./snips_start.sh
#
# Start Snips that is downloaded to specific dir:
# $ ./snips_start.sh -d <dir>
#
# Stop Snips:
# $ ./snips_start.sh -k
#
# Check if Snips is running:
# $ ./snips_start.sh -c
#
# Display usage help:
# $ ./snips_start.sh -h
#

VERSION=1.0.0

SNIPS_DIR_DEFAULT=/home/nemo/.cache/harbour-snipek/harbour-snipek/snips
SNIPS_DIR=$SNIPS_DIR_DEFAULT

########################

STOP=0
VERBOSE=0
CHECK_RUNNING=0

print() {
  echo "$1"
}

print_error() {
  echo "$1" 1>&2
}

usage() {
  print "Usage: $0 [OPTION]..."
  print "Start Snips processes."
  print ""
  print "Options:"
  print "  -d <DIR>     directory where Snips is installed (default is $SNIPS_DIR_DEFAULT)"
  print "  -k           stop instead start"
  print "  -c           check if Snips is already running instead start"
  print "  -v           sets the verbosity"
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
    usage
  fi
  exit 1
}

check_processes() {
  local processes=($(ps -e | sed -r 's/.*[0-9][0-9]:[0-9][0-9]:[0-9][0-9] ([^ ]+).*$/\1/g;t;d'))
  local error=0
  for needed_process in "${snips_processes[@]}"
  do
    local found=0
    for process in "${processes[@]}"
    do
      if [ "$process" == "$needed_process" ]; then
        print "Process $process is running."
        found=1
        break
      fi
    done
    if [ $found -eq 0 ]; then
      print_error "Process $needed_process is not running."
      error=1
    fi
  done
  return $error
}

kill_processes() {
  for process in "${snips_processes[@]}"
  do
    if killall -9 "$process"; then
      print "Process $process stopped."
    fi
  done
}

check_all_files() {
  local files_to_install=()
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
  
  local error=0
  for file in "${files_to_install[@]}"
  do
    if [ ! -f "$file" ]; then
      print_error "Error: File $file does not exist."
      error=1
    fi
  done
  
  if [ ! -f "$SNIPS_DIR/pico/lang/en-US_ta.bin" ]; then
    print_error "Error: File $SNIPS_DIR/pico/lang/en-US_ta.bin does not exist."
    error=1
  fi
  
  if [ ! -f "$SNIPS_DIR/assistant/assistant.json" ]; then
    exit_abnormal "Error: File $SNIPS_DIR/assistant/assistant.json does not exist."
  fi
  
  return $error
}

# parse options

while getopts ":Vhcvkd:" options; do
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
  k)
    STOP=1
    ;;
  c)
    CHECK_RUNNING=1
    ;;
  v)
    VERBOSE=1
    ;;
  :)
    exit_abnormal "Error: Option -$OPTARG requires an argument." 1
    ;;
  ?)
    exit_abnormal "Error: Unknown option -$OPTARG." 1
    ;;
  esac
done

# check needed shell commands

needed_commands=( killall readlink )
error=0
for cmd in "${needed_commands[@]}"
do
  if ! [ -x "$(command -v "$cmd")" ]; then
    print_error "Error: $cmd is required but not installed."
    error=1
  fi
done
if [ $error -ne 0 ]; then
  exit_abnormal
fi

# check snips dir, if not provided default to <snipek cache>/snips

if [ ! -d "$SNIPS_DIR" ]; then
  exit_abnormal "Error: Directory $SNIPS_DIR does not exist." 1
else
  SNIPS_DIR=$(readlink -f "$SNIPS_DIR")
fi

print "Using directory: $SNIPS_DIR."

declare -a snips_processes
snips_processes+=("snips-asr")
snips_processes+=("snips-tts")
snips_processes+=("snips-dialogue")
snips_processes+=("snips-nlu")
snips_processes+=("snips-hotword")
snips_processes+=("snips-asr")
snips_processes+=("mosquitto")

# check if Snips is running

if [ $CHECK_RUNNING -eq 1 ]; then
  print "Check if all Snips processes are running."
  if ! check_processes; then
    exit_abnormal "Error: Some Snips processes are stopped."
  fi
  print "Done. All Snips processes are running."
  exit 0
fi

# export required viariables

export LD_LIBRARY_PATH=$SNIPS_DIR
export PATH=$PATH:$SNIPS_DIR

# stop

print "Stopping Snips processes..."
kill_processes

if [ $STOP -eq 0 ]; then
  print "Check if all needed files are present in $SNIPS_DIR directory."
  
  if ! check_all_files; then
    exit_abnormal "Error: Some needed files are missing."
  fi

  print "Starting Snips..."
  
  if ! "$SNIPS_DIR/mosquitto" -d; then
    exit_abnormal "Error: Cannot start MQTT."
  fi
  
  assistant_dir="$SNIPS_DIR/assistant"
  pushd "$SNIPS_DIR"
  if [ $VERBOSE -eq 1 ]; then
    opts="-v"
    "$SNIPS_DIR/snips-hotword" $opts --assistant "$assistant_dir" --user-dir "$SNIPS_DIR" &
    "$SNIPS_DIR/snips-nlu" $opts --assistant "$assistant_dir" --user-dir "$SNIPS_DIR" &
    "$SNIPS_DIR/snips-dialogue" $opts --assistant "$assistant_dir" --user-dir "$SNIPS_DIR" &
    "$SNIPS_DIR/snips-tts" $opts --assistant "$assistant_dir" --user-dir "$SNIPS_DIR" --provider picotts &
    "$SNIPS_DIR/snips-asr" $opts --assistant "$assistant_dir" --user-dir "$SNIPS_DIR" &
  else
    "$SNIPS_DIR/snips-hotword" --assistant "$assistant_dir" --user-dir "$SNIPS_DIR" > /dev/null 2>&1 &
    "$SNIPS_DIR/snips-nlu" --assistant "$assistant_dir" --user-dir "$SNIPS_DIR" > /dev/null 2>&1 &
    "$SNIPS_DIR/snips-dialogue" --assistant "$assistant_dir" --user-dir "$SNIPS_DIR" > /dev/null 2>&1 &
    "$SNIPS_DIR/snips-tts" --assistant "$assistant_dir" --user-dir "$SNIPS_DIR" --provider picotts > /dev/null 2>&1 &
    "$SNIPS_DIR/snips-asr" --assistant "$assistant_dir" --user-dir "$SNIPS_DIR" > /dev/null 2>&1 &
  fi
  popd
  
  print "Check if all Snips processes are running."
  
  if ! check_processes; then
    print_error "Error: Snips cannot be started."
    kill_processes
    exit_abnormal
  fi
  
  print "Done. Snips successfuly started."
else
  print "Done. Snips successfuly stopped."
fi

