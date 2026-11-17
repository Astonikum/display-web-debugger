#include <MadDisplayWebDebugger/MadDisplayWebDebugger.hpp>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

namespace {

const char kIndexHtml[] = R"HTML(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>MadDisplay Web Debugger</title>
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Roboto+Mono:wght@400;500&display=swap" rel="stylesheet">
  <style>
    body { font-family: "Roboto Mono", ui-monospace, SFMono-Regular, Menlo, Consolas, monospace; }
    .controls { display: grid; grid-template-columns: repeat(3, 42px); gap: 2px; }
    button { width: 42px; height: 30px; padding: 4px; font: inherit; display: grid; place-items: center; }
    .icon { width: 20px; height: 20px; fill: currentColor; }
    .icon-sprite { position: absolute; width: 0; height: 0; overflow: hidden; }
    table { width: 500px; table-layout: fixed; border-collapse: collapse; margin-top: 4px; }
    th, td { border: 1px solid #999; padding: 2px 5px; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
    th { text-align: left; }
    #screenStatus th:first-child { width: 100px; }
    #screenStatus th:not(:first-child), #screenStatus td:not(:first-child) { width: 130px; }
  </style>
</head>
<body>
  <h1>MadDisplay Web Debugger</h1>
  <canvas id="screen" tabindex="0" aria-label="display pixel grid"></canvas>
  <svg class="icon-sprite" aria-hidden="true">
    <symbol id="icon-left" viewBox="0 0 256 256"><path d="M224,128a8,8,0,0,1-8,8H59.31l58.35,58.34a8,8,0,0,1-11.32,11.32l-72-72a8,8,0,0,1,0-11.32l72-72a8,8,0,0,1,11.32,11.32L59.31,120H216A8,8,0,0,1,224,128Z"/></symbol>
    <symbol id="icon-up" viewBox="0 0 256 256"><path d="M205.66,117.66a8,8,0,0,1-11.32,0L136,59.31V216a8,8,0,0,1-16,0V59.31L61.66,117.66a8,8,0,0,1-11.32-11.32l72-72a8,8,0,0,1,11.32,0l72,72A8,8,0,0,1,205.66,117.66Z"/></symbol>
    <symbol id="icon-right" viewBox="0 0 256 256"><path d="M221.66,133.66l-72,72a8,8,0,0,1-11.32-11.32L196.69,136H40a8,8,0,0,1,0-16H196.69L138.34,61.66a8,8,0,0,1,11.32-11.32l72,72A8,8,0,0,1,221.66,133.66Z"/></symbol>
    <symbol id="icon-previous" viewBox="0 0 256 256"><path d="M205.66,202.34a8,8,0,0,1-11.32,11.32l-80-80a8,8,0,0,1,0-11.32l80-80a8,8,0,0,1,11.32,11.32L131.31,128ZM51.31,128l74.35-74.34a8,8,0,0,0-11.32-11.32l-80,80a8,8,0,0,0,0,11.32l80,80a8,8,0,0,0,11.32-11.32Z"/></symbol>
    <symbol id="icon-next" viewBox="0 0 256 256"><path d="M141.66,133.66l-80,80a8,8,0,0,1-11.32-11.32L124.69,128,50.34,53.66A8,8,0,0,1,61.66,42.34l80,80A8,8,0,0,1,141.66,133.66Zm80-11.32-80-80a8,8,0,0,0-11.32,11.32L204.69,128l-74.35,74.34a8,8,0,0,0,11.32,11.32l80-80A8,8,0,0,0,221.66,122.34Z"/></symbol>
    <symbol id="icon-use" viewBox="0 0 256 256"><path d="M219.31,184a16,16,0,0,1,0,22.63l-12.68,12.68a16,16,0,0,1-22.63,0L132.7,168,115,214.09c0,.1-.08.21-.13.32a15.83,15.83,0,0,1-14.6,9.59l-.79,0a15.83,15.83,0,0,1-14.41-11L32.8,52.92A16,16,0,0,1,52.92,32.8L213,85.07a16,16,0,0,1,1.41,29.8l-.32.13L168,132.69ZM208,195.31,156.69,144a16,16,0,0,1,4.93-26l.32-.14,45.95-17.64L48,48l52.2,159.86,17.65-46c0-.11.08-.22.13-.33a15.83,15.83,0,0,1,11.69-9.34,16.72,16.72,0,0,1,3-.28,16,16,0,0,1,11.3,4.69L195.31,208Z"/></symbol>
    <symbol id="icon-back" viewBox="0 0 256 256"><path d="M232,144a64.07,64.07,0,0,1-64,64H80a8,8,0,0,1,0-16h88a48,48,0,0,0,0-96H51.31l34.35,34.34a8,8,0,0,1-11.32,11.32l-48-48a8,8,0,0,1,0-11.32l48-48A8,8,0,0,1,85.66,45.66L51.31,80H168A64.07,64.07,0,0,1,232,144Z"/></symbol>
    <symbol id="icon-down" viewBox="0 0 256 256"><path d="M205.66,149.66l-72,72a8,8,0,0,1-11.32,0l-72-72a8,8,0,0,1,11.32-11.32L120,196.69V40a8,8,0,0,1,16,0V196.69l58.34-58.35A8,8,0,0,1,205.66,149.66Z"/></symbol>
  </svg>
  <div class="controls" aria-label="Input actions">
    <button type="button" data-action="Left" aria-label="LEFT" title="LEFT"><svg class="icon"><use href="#icon-left"></use></svg></button>
    <button type="button" data-action="Up" aria-label="UP" title="UP"><svg class="icon"><use href="#icon-up"></use></svg></button>
    <button type="button" data-action="Right" aria-label="RIGHT" title="RIGHT"><svg class="icon"><use href="#icon-right"></use></svg></button>
    <button type="button" data-action="Previous" aria-label="PREVIOUS" title="PREVIOUS"><svg class="icon"><use href="#icon-previous"></use></svg></button>
    <button type="button" data-action="Use" aria-label="USE" title="USE"><svg class="icon"><use href="#icon-use"></use></svg></button>
    <button type="button" data-action="Next" aria-label="NEXT" title="NEXT"><svg class="icon"><use href="#icon-next"></use></svg></button>
    <button type="button" data-action="Back" aria-label="BACK" title="BACK"><svg class="icon"><use href="#icon-back"></use></svg></button>
    <button type="button" data-action="Down" aria-label="DOWN" title="DOWN"><svg class="icon"><use href="#icon-down"></use></svg></button>
    <span></span>
  </div>
  <table id="screenStatus">
    <thead>
      <tr><th>Metric</th><th>Target</th><th>Actual</th><th>Browser paint</th></tr>
    </thead>
    <tbody>
      <tr><th>FPS</th><td id="statusTargetFps">--</td><td id="statusActualFps">--</td><td id="statusBrowserFps">--</td></tr>
      <tr><th>Frame time</th><td id="statusTargetFrameTime">--</td><td id="statusActualFrameTime">--</td><td id="statusBrowserFrameTime">--</td></tr>
      <tr><th>Page</th><td id="statusPage" colspan="3">--</td></tr>
      <tr><th>Focus</th><td id="statusFocus" colspan="3">--</td></tr>
    </tbody>
  </table>
  <script>
    const canvas = document.getElementById('screen');
    const ctx = canvas.getContext('2d');
    ctx.imageSmoothingEnabled = false;
    const statusTargetFps = document.getElementById('statusTargetFps');
    const statusActualFps = document.getElementById('statusActualFps');
    const statusBrowserFps = document.getElementById('statusBrowserFps');
    const statusTargetFrameTime = document.getElementById('statusTargetFrameTime');
    const statusActualFrameTime = document.getElementById('statusActualFrameTime');
    const statusBrowserFrameTime = document.getElementById('statusBrowserFrameTime');
    const statusPage = document.getElementById('statusPage');
    const statusFocus = document.getElementById('statusFocus');
    let state = __STATE__;
    let image = null;
    let imageWidth = 0;
    let imageHeight = 0;
    let lastPixels = null;
    let refreshInFlight = false;
    let actionInFlight = false;
    let browserPreviousFrame = 0;
    let browserWindowStart = performance.now();
    let browserFrameCount = 0;
    let browserFps = 0;
    let browserFrameTime = 0;

    function draw(nextState) {
      state = nextState;
      if (imageWidth !== state.width || imageHeight !== state.height) {
        imageWidth = state.width;
        imageHeight = state.height;
        canvas.width = imageWidth;
        canvas.height = imageHeight;
        image = ctx.createImageData(imageWidth, imageHeight);
        lastPixels = null;
      }
      const imageKey = (state.pixelFormat || 'mono1') + (state.pixels || '');
      if (imageKey !== lastPixels) {
        const output = image.data;
        const packed = atob(state.pixels);
        for (let pixel = 0; pixel < imageWidth * imageHeight; pixel++) {
          const offset = pixel * 4;
          if (state.pixelFormat === 'rgb888') {
            output[offset] = packed.charCodeAt(pixel*3);
            output[offset+1] = packed.charCodeAt(pixel*3+1);
            output[offset+2] = packed.charCodeAt(pixel*3+2);
          } else {
            const value = packed.charCodeAt(pixel >> 3) & (1 << (pixel & 7)) ? 255 : 0;
            output[offset] = output[offset+1] = output[offset+2] = value;
          }
          output[offset+3] = 255;
        }
        lastPixels = imageKey;
        ctx.putImageData(image, 0, 0);
      }
      statusTargetFps.textContent = state.fps === -1 ? 'MAX' : state.fps;
      statusActualFps.textContent = `${state.actualFps} FPS`;
      statusTargetFrameTime.textContent = state.fps === -1 ? '—' : `${state.frameTimeMs} ms`;
      statusActualFrameTime.textContent = `${state.actualFrameTimeMs} ms`;
      statusPage.textContent = `Page${state.page + 1}`;
      statusFocus.textContent = state.focused ? `${state.focused.x},${state.focused.y} (${state.focused.w}×${state.focused.h})` : '—';
    }
    async function refresh() {
      if (refreshInFlight) return;
      refreshInFlight = true;
      try {
        const response = await fetch('/', {
          headers: { Accept: 'application/json' },
          cache: 'no-store'
        });
        if (response.ok) draw(await response.json());
      } catch (error) {
        // A debugger may be restarted while the page stays open. Keep the
        // polling loop alive and let the next frame reconnect.
      } finally {
        refreshInFlight = false;
      }
    }
    function scheduleRefresh() {
      if (state.fps === -1) {
        // Unlimited display mode must not inherit the browser's vsync cap.
        // Poll as soon as the previous request completes; the browser's own
        // paint FPS is measured independently below.
        setTimeout(async () => { await refresh(); scheduleRefresh(); }, 0);
      } else {
        setTimeout(async () => { await refresh(); scheduleRefresh(); }, Math.max(1, state.frameTimeMs));
      }
    }
    async function dispatch(action) {
      if (actionInFlight) return;
      actionInFlight = true;
      try {
        const response = await fetch('/', {
          method: 'POST',
          headers: {
            Accept: 'application/json',
            'Content-Type': 'application/x-www-form-urlencoded'
          },
          body: `action=${encodeURIComponent(action)}`
        });
        if (response.ok) draw(await response.json());
      } catch (error) {
        // Ignore a transient connection loss; the next poll will retry.
      } finally {
        actionInFlight = false;
      }
    }
    document.querySelectorAll('[data-action]').forEach(control => {
      control.addEventListener('click', () => dispatch(control.dataset.action));
    });
    document.addEventListener('keydown', event => {
      const keyMap = { ArrowUp: 'Up', ArrowDown: 'Down', ArrowLeft: 'Left', ArrowRight: 'Right', Enter: 'Use', ' ': 'Use', Escape: 'Back', Backspace: 'Back', n: 'Next', p: 'Previous' };
      const action = keyMap[event.key];
      if (action) { event.preventDefault(); dispatch(action); }
    });
    function measureBrowserFrame(now) {
      if (browserPreviousFrame) {
        browserFrameTime = now - browserPreviousFrame;
        browserFrameCount++;
      }
      if (now - browserWindowStart >= 500) {
        browserFps = browserFrameCount * 1000 / (now - browserWindowStart);
        browserFrameCount = 0;
        browserWindowStart = now;
      }
      browserPreviousFrame = now;
      statusBrowserFps.textContent = `${browserFps.toFixed(1)} FPS`;
      statusBrowserFrameTime.textContent = `${browserFrameTime.toFixed(1)} ms`;
      requestAnimationFrame(measureBrowserFrame);
    }
    draw(state);
    scheduleRefresh();
    requestAnimationFrame(measureBrowserFrame);
  </script>
</body>
</html>)HTML";

bool sendAll(int client, const char* data, size_t size) {
  size_t sent = 0;
  while (sent < size) {
#ifdef MSG_NOSIGNAL
    const int flags = MSG_NOSIGNAL;
#else
    const int flags = 0;
#endif
    const ssize_t count = send(client, data + sent, size - sent, flags);
    if (count <= 0) return false;
    sent += static_cast<size_t>(count);
  }
  return true;
}

bool sendAll(int client, const std::string& payload) {
  return sendAll(client, payload.data(), payload.size());
}

void appendInt(std::string& output, int32_t value) {
  char number[16];
  const int length = std::snprintf(number, sizeof(number), "%ld", static_cast<long>(value));
  if (length > 0) output.append(number, static_cast<size_t>(length));
}

void appendBase64(std::string& output, const uint8_t* bytes, size_t count) {
  static const char alphabet[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  output.reserve(output.size() + ((count + 2) / 3) * 4);
  for (size_t i = 0; i < count; i += 3) {
    const uint32_t value = (static_cast<uint32_t>(bytes[i]) << 16) |
        (i + 1 < count ? static_cast<uint32_t>(bytes[i + 1]) << 8 : 0) |
        (i + 2 < count ? bytes[i + 2] : 0);
    output += alphabet[(value >> 18) & 63];
    output += alphabet[(value >> 12) & 63];
    output += i + 1 < count ? alphabet[(value >> 6) & 63] : '=';
    output += i + 2 < count ? alphabet[value & 63] : '=';
  }
}

uint32_t monotonicMicros() {
  using Clock = std::chrono::steady_clock;
  const auto now = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now().time_since_epoch());
  return static_cast<uint32_t>(now.count());
}

}  // namespace

namespace madweb {

MadDisplayWebDebugger::MadDisplayWebDebugger(mad::Buffer& buffer, mad::Compositor& compositor, uint16_t port)
    : buffer_(buffer), compositor_(compositor), port_(port) {}

MadDisplayWebDebugger::~MadDisplayWebDebugger() { stop(); }

bool MadDisplayWebDebugger::start() {
  if (running()) return true;
  serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket_ < 0) return false;

  int reuse = 1;
  setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  address.sin_port = htons(port_);
  if (bind(serverSocket_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0 || listen(serverSocket_, 8) < 0) {
    stop();
    return false;
  }
  return true;
}

void MadDisplayWebDebugger::stop() {
  if (serverSocket_ >= 0) {
    close(serverSocket_);
    serverSocket_ = -1;
  }
}

void MadDisplayWebDebugger::run() {
  if (!start()) return;
  while (running()) serveOnce();
}

bool MadDisplayWebDebugger::serveOnce() {
  if (!start()) return false;
  const int client = accept(serverSocket_, nullptr, nullptr);
  if (client < 0) return false;
  int noDelay = 1;
  setsockopt(client, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof(noDelay));
#ifdef SO_NOSIGPIPE
  int noSigPipe = 1;
  setsockopt(client, SOL_SOCKET, SO_NOSIGPIPE, &noSigPipe, sizeof(noSigPipe));
#endif
  handleClient(client);
  close(client);
  return true;
}

std::string MadDisplayWebDebugger::state() {
  if (compositor_.currentPage()) compositor_.render(buffer_, monotonicMicros());
  else buffer_.clear();
  const mad::Size dimensions = buffer_.size();
  std::string json;
  const bool monochrome = buffer_.format() == mad::PixelFormat::Mono1;
  const size_t bytes = monochrome ? buffer_.byteCount() : size_t(dimensions.width)*dimensions.height*3;
  json.reserve(2048 + ((bytes + 2) / 3) * 4);
  json += "{\"width\":";
  appendInt(json, dimensions.width);
  json += ",\"height\":";
  appendInt(json, dimensions.height);
  json += ",\"fps\":";
  appendInt(json, compositor_.fps());
  json += ",\"frameTimeMs\":";
  appendInt(json, compositor_.frameTimeMs());
  json += ",\"actualFps\":";
  appendInt(json, compositor_.actualFps());
  json += ",\"actualFrameTimeMs\":";
  appendInt(json, compositor_.actualFrameTimeMs());
  json += ",\"page\":";
  appendInt(json, compositor_.currentPageId());
  json += ",\"focused\":";
  mad::Component* focused = compositor_.currentPage() ? compositor_.currentPage()->focusedComponent() : nullptr;
  if (!focused) json += "null";
  else {
    const mad::Rect rect = focused->bounds();
    json += "{\"x\":";
    appendInt(json, rect.x);
    json += ",\"y\":";
    appendInt(json, rect.y);
    json += ",\"w\":";
    appendInt(json, rect.width);
    json += ",\"h\":";
    appendInt(json, rect.height);
    json += '}';
  }
  json += monochrome ? ",\"pixelFormat\":\"mono1\",\"pixels\":\"" : ",\"pixelFormat\":\"rgb888\",\"pixels\":\"";
  if (monochrome) appendBase64(json,buffer_.pixels(),buffer_.byteCount());
  else {
    std::vector<uint8_t> rgb; rgb.reserve(bytes);
    for (int16_t y = 0; y < dimensions.height; ++y) for (int16_t x = 0; x < dimensions.width; ++x) {
      const auto color = buffer_.pixel(x,y); rgb.push_back(mad::red(color)); rgb.push_back(mad::green(color)); rgb.push_back(mad::blue(color));
    }
    appendBase64(json,rgb.data(),rgb.size());
  }
  json += '\"';
  json += '}';
  return json;
}

std::string MadDisplayWebDebugger::page() {
  std::string html(kIndexHtml);
  const std::string marker = "__STATE__";
  const size_t statePosition = html.find(marker);
  if (statePosition != std::string::npos) html.replace(statePosition, marker.size(), state());
  return html;
}

void MadDisplayWebDebugger::handleClient(int client) {
  char buffer[8192];
  const ssize_t received = recv(client, buffer, sizeof(buffer) - 1, 0);
  if (received <= 0) return;
  buffer[received] = '\0';
  const std::string request(buffer, static_cast<size_t>(received));
  const size_t lineEnd = request.find("\r\n");
  if (lineEnd == std::string::npos) return;
  const std::string line = request.substr(0, lineEnd);
  const size_t firstSpace = line.find(' ');
  const size_t secondSpace = line.find(' ', firstSpace + 1);
  if (firstSpace == std::string::npos || secondSpace == std::string::npos) return;
  const std::string method = line.substr(0, firstSpace);
  const std::string path = line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
  const size_t queryStart = path.find('?');
  const std::string route = queryStart == std::string::npos ? path : path.substr(0, queryStart);
  const bool wantsJson = request.find("Accept: application/json") != std::string::npos;
  if (method == "GET" && route == "/") {
    sendResponse(client, wantsJson ? state() : page(),
                 wantsJson ? "application/json" : "text/html; charset=utf-8");
  } else if (method == "POST" && route == "/") {
    const size_t bodyStart = request.find("\r\n\r\n");
    const std::string body = bodyStart == std::string::npos ? "" : request.substr(bodyStart + 4);
    const size_t actionStart = body.find("action=");
    const size_t actionOffset = actionStart == std::string::npos ? 0 : actionStart + 7;
    const size_t actionEnd = body.find('&', actionOffset);
    const std::string action = body.substr(actionOffset, actionEnd == std::string::npos ? std::string::npos : actionEnd - actionOffset);
    const mad::Action* selected = actionFor(action);
    if (selected) compositor_.dispatch(*selected);
    sendResponse(client, wantsJson ? state() : page(),
                 wantsJson ? "application/json" : "text/html; charset=utf-8");
  } else {
    sendNotFound(client);
  }
}

const mad::Action* MadDisplayWebDebugger::actionFor(const std::string& name) {
  static const mad::Action down = mad::Action::Down;
  static const mad::Action up = mad::Action::Up;
  static const mad::Action left = mad::Action::Left;
  static const mad::Action right = mad::Action::Right;
  static const mad::Action next = mad::Action::Next;
  static const mad::Action previous = mad::Action::Previous;
  static const mad::Action use = mad::Action::Use;
  static const mad::Action back = mad::Action::Back;
  if (name == "Down") return &down;
  if (name == "Up") return &up;
  if (name == "Left") return &left;
  if (name == "Right") return &right;
  if (name == "Next") return &next;
  if (name == "Previous") return &previous;
  if (name == "Use") return &use;
  if (name == "Back") return &back;
  return nullptr;
}

void MadDisplayWebDebugger::sendResponse(int client, const std::string& body, const char* contentType) {
  char header[256];
  const int length = std::snprintf(header, sizeof(header),
      "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nCache-Control: no-store\r\n"
      "Content-Length: %lu\r\nConnection: close\r\n\r\n",
      contentType, static_cast<unsigned long>(body.size()));
  if (length > 0) sendAll(client, header, static_cast<size_t>(length));
  sendAll(client, body);
}

void MadDisplayWebDebugger::sendNotFound(int client) {
  const std::string body = "Not found";
  char header[160];
  const int length = std::snprintf(header, sizeof(header),
      "HTTP/1.1 404 Not Found\r\nContent-Length: %lu\r\nConnection: close\r\n\r\n",
      static_cast<unsigned long>(body.size()));
  if (length > 0) sendAll(client, header, static_cast<size_t>(length));
  sendAll(client, body);
}

}  // namespace madweb
