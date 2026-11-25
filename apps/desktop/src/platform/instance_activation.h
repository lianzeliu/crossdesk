/*
 * @Author: DI JUNKUN
 * @Date: 2026-10-17
 * Copyright (c) 2026 by DI JUNKUN, All Rights Reserved.
 */

#ifndef _INSTANCE_ACTIVATION_H_
#define _INSTANCE_ACTIVATION_H_

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdint>
#endif

#include <string>

namespace crossdesk::platform {

// Created only by the interactive GUI, on its event-loop thread. Requests can
// arrive during startup and remain pending until the UI is ready to show
// itself.
class InstanceActivationReceiver {
 public:
  explicit InstanceActivationReceiver(std::string application = "CrossDesk");
  ~InstanceActivationReceiver();
  InstanceActivationReceiver(const InstanceActivationReceiver&) = delete;
  InstanceActivationReceiver& operator=(const InstanceActivationReceiver&) =
      delete;

  // The caller must hold this application's client single-instance lease.
  bool Start(std::string& error);
  bool TakeRequest(std::string* activation_token = nullptr);

#ifdef _WIN32
  // A peer id forwarded by a second launch (desktop shortcut --connect).
  // Take it once; the UI validates it before connecting.
  bool TakeConnectRequest(std::string* peer_id);
#endif

 private:
  std::string application_;
#ifdef _WIN32
  static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wparam,
                                     LPARAM lparam);

  HWND window_ = nullptr;
  UINT activation_message_ = 0;
  bool requested_ = false;
  std::string pending_connect_id_;
#else
  int socket_ = -1;
  std::string socket_path_;
  std::uint64_t socket_device_ = 0;
  std::uint64_t socket_inode_ = 0;
#endif
};

// Targets the current user's GUI on the current desktop, including a daemon
// child. Waits briefly for startup/restart, without waiting on its UI thread.
bool RequestInstanceActivation(std::string& error,
                               const std::string& application = "CrossDesk");

#ifdef _WIN32
// Like RequestInstanceActivation, but also carries a peer id (desktop
// shortcut --connect) to the running instance via WM_COPYDATA. The id is
// validated by the receiver's UI before connecting.
bool RequestInstanceActivationWithConnect(std::string& error,
                                          const std::string& connect_id,
                                          const std::string& application = "CrossDesk");
#endif

}  // namespace crossdesk::platform

#endif
