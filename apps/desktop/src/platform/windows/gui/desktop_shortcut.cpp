#include "platform/windows/gui/desktop_shortcut.h"

#include <objbase.h>
#include <shlobj.h>
#include <windows.h>

#include <cstdio>
#include <cwchar>
#include <string>

namespace crossdesk::platform {
namespace {

std::wstring Utf8ToWide(const std::string& text) {
  if (text.empty()) return {};
  const int length = MultiByteToWideChar(CP_UTF8, 0, text.data(),
                                         static_cast<int>(text.size()), nullptr,
                                         0);
  if (length <= 0) return {};
  std::wstring wide(static_cast<size_t>(length), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                      wide.data(), length);
  return wide;
}

std::string HresultError(HRESULT result) {
  char buffer[16] = {};
  std::snprintf(buffer, sizeof(buffer), "0x%08lX",
                static_cast<unsigned long>(result));
  return buffer;
}

// Replaces characters illegal in Windows file names, and trims trailing
// spaces/dots (also illegal at the end of a name).
std::wstring SanitizeLinkName(const std::string& display_name,
                              const std::string& fallback) {
  std::wstring name = Utf8ToWide(display_name);
  if (name.empty()) name = Utf8ToWide(fallback);
  constexpr wchar_t kIllegal[] = L"<>:\"/\\|?*";
  for (wchar_t& ch : name) {
    if (ch < 0x20 || wcschr(kIllegal, ch) != nullptr) ch = L'_';
  }
  while (!name.empty() && (name.back() == L' ' || name.back() == L'.'))
    name.pop_back();
  if (name.empty()) name = Utf8ToWide(fallback);
  return name;
}

class ComGuard {
 public:
  ComGuard() : result_(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)) {}
  ~ComGuard() {
    if (SUCCEEDED(result_)) CoUninitialize();
  }
  bool ok() const { return SUCCEEDED(result_); }
  std::string error() const { return HresultError(result_); }

 private:
  HRESULT result_;
};

}  // namespace

bool CreateDesktopConnectShortcut(const std::string& peer_id,
                                  const std::string& display_name,
                                  std::string& error) {
  error.clear();
  if (peer_id.empty() || peer_id.size() > 64) {
    error = "Invalid peer id for desktop shortcut";
    return false;
  }

  wchar_t desktop_path[MAX_PATH] = {};
  if (FAILED(SHGetFolderPathW(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr,
                              SHGFP_TYPE_CURRENT, desktop_path))) {
    error = "Cannot locate the desktop folder: " +
            std::to_string(GetLastError());
    return false;
  }
  wchar_t exe_path[MAX_PATH] = {};
  if (GetModuleFileNameW(nullptr, exe_path, MAX_PATH) == 0) {
    error = "Cannot locate the CrossDesk executable: " +
            std::to_string(GetLastError());
    return false;
  }

  ComGuard com;
  if (!com.ok()) {
    error = "Cannot initialize COM: " + com.error();
    return false;
  }

  IShellLinkW* link = nullptr;
  if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                              IID_IShellLinkW,
                              reinterpret_cast<void**>(&link)))) {
    error = "Cannot create shell link";
    return false;
  }
  link->SetPath(exe_path);
  const std::wstring arguments = L"--connect " + Utf8ToWide(peer_id);
  link->SetArguments(arguments.c_str());
  link->SetDescription(Utf8ToWide("Connect to " + peer_id).c_str());

  IPersistFile* file = nullptr;
  const HRESULT query =
      link->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&file));
  link->Release();
  if (FAILED(query)) {
    error = "Cannot access the shell link file interface: " +
            HresultError(query);
    return false;
  }
  const std::wstring link_path =
      std::wstring(desktop_path) + L"\\" +
      SanitizeLinkName(display_name, peer_id) + L".lnk";
  const HRESULT saved = file->Save(link_path.c_str(), TRUE);
  file->Release();
  if (FAILED(saved)) {
    error = "Cannot save the desktop shortcut: " + HresultError(saved);
    return false;
  }
  return true;
}

}  // namespace crossdesk::platform
