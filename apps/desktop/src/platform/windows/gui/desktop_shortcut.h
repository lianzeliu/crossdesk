#pragma once

#include <string>

namespace crossdesk::platform {

// Creates a .lnk on the current user's desktop that launches this program
// with `--connect <peer_id>`. The link name comes from display_name (or the
// peer id when it is empty), with characters illegal in file names replaced
// by '_'. Returns false and fills error on failure.
bool CreateDesktopConnectShortcut(const std::string& peer_id,
                                  const std::string& display_name,
                                  std::string& error);

}  // namespace crossdesk::platform
