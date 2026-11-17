#pragma once

#include <stdint.h>
#include <string>

#include <MadDisplay/buffer.hpp>

namespace madweb {

// Host-only HTTP bridge for a readable MadDisplay buffer. The UI model stays
// in the normal Compositor; this class only renders, exposes pixels, and
// forwards normalized actions.
class MadDisplayWebDebugger {
 public:
  MadDisplayWebDebugger(mad::Buffer& buffer, mad::Compositor& compositor, uint16_t port = 4174);
  ~MadDisplayWebDebugger();

  MadDisplayWebDebugger(const MadDisplayWebDebugger&) = delete;
  MadDisplayWebDebugger& operator=(const MadDisplayWebDebugger&) = delete;

  bool start();
  void run();
  bool serveOnce();
  void stop();
  bool running() const { return serverSocket_ >= 0; }
  uint16_t port() const { return port_; }

 private:
  std::string page();
  std::string state();
  void handleClient(int client);
  static const mad::Action* actionFor(const std::string& name);
  static void sendResponse(int client, const std::string& body, const char* contentType);
  static void sendNotFound(int client);

  mad::Buffer& buffer_;
  mad::Compositor& compositor_;
  uint16_t port_;
  int serverSocket_ = -1;
};

}  // namespace madweb
