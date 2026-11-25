/*
 * @Author: DI JUNKUN
 * @Date: 2026-09-04
 * Copyright (c) 2026 by DI JUNKUN, All Rights Reserved.
 */

#ifndef _RENDER_H_
#define _RENDER_H_

#include <memory>
#include <string>

namespace crossdesk {

class GuiApplication;

// Stable application-facing facade. The Slint application and feature
// controllers remain private implementation details.
class Render {
public:
  Render();
  ~Render();

  Render(const Render &) = delete;
  Render &operator=(const Render &) = delete;

  int Run();
  // Connect to this peer once the signal server is up (see
  // GuiApplication::SetPendingConnectId). No-op when empty.
  void SetPendingConnectId(const std::string& id);
#ifdef _WIN32
  int RunUnattended(void* stop_event);
#endif

private:
  std::unique_ptr<GuiApplication> application_;
};

} // namespace crossdesk

#endif
