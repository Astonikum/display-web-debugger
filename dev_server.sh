#!/bin/sh
set -eu

root_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
server_binary=/tmp/mad_display_server
next_binary=/tmp/mad_display_server.next
stamp=/tmp/mad_display_server.watch.$$
server_pid=0

build() {
  echo "Building MadDisplay Web Debugger..."
  if c++ -std=c++11 -O2 \
      -I"$root_dir/../display-lib/src" -I"$root_dir/include" \
      "$root_dir/examples/main.cpp" "$root_dir/src/MadDisplayWebDebugger.cpp" \
      -o "$next_binary"; then
    mv "$next_binary" "$server_binary"
  else
    rm -f "$next_binary"
    return 1
  fi
}

stop_server() {
  if [ "$server_pid" -ne 0 ]; then
    kill -TERM "$server_pid" 2>/dev/null || true
    wait "$server_pid" 2>/dev/null || true
    server_pid=0
  fi
}

start_server() {
  "$server_binary" &
  server_pid=$!
}

cleanup() {
  stop_server
  rm -f "$stamp" "$next_binary"
}

trap cleanup INT TERM EXIT

build
touch "$stamp"
start_server

while :; do
  if [ -n "$(find "$root_dir/../display-lib/src" "$root_dir/include" \
      "$root_dir/src" "$root_dir/examples" -type f \
      \( -name '*.hpp' -o -name '*.cpp' -o -name '*.ino' \) \
      -newer "$stamp" -print -quit)" ]; then
    stop_server
    if build; then
      start_server
    else
      echo "Build failed; keeping the previous server binary."
    fi
    touch "$stamp"
  fi
  if [ "$server_pid" -eq 0 ] || ! kill -0 "$server_pid" 2>/dev/null; then
    start_server
  fi
  sleep 1
done
