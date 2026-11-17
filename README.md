# MadDisplay Web Debugger (legacy)

The native UI editor is now [Mad Studio](https://github.com/Astonikum/display-studio). This
HTTP bridge remains available for lightweight device/browser monitoring.

Native browser debugger for the sibling [`MadDisplay`](https://github.com/Astonikum/display-lib) library.

## Run

From this directory:

```bash
pio run -e native_server
./.pio/build/native_server/program
```

Or compile and run directly:

```bash
c++ -std=c++11 -O2 -I../display-lib/src -Iinclude \
  examples/main.cpp src/MadDisplayWebDebugger.cpp -o /tmp/mad_display_server
/tmp/mad_display_server
```

For development, use the watcher; it rebuilds and restarts the server after
changes in the Arduino library or debugger sources:

```bash
./dev_server.sh
```

Open `http://127.0.0.1:4174/`.

Monochrome buffers keep the packed `mono1` wire payload. Colored/palette buffers
are sent as canonical `rgb888` pixels with an explicit `pixelFormat` field; the
browser uses that field rather than interpreting every frame as a bitmask.
