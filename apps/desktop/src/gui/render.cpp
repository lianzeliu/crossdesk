#include "render.h"

#include <memory>

#include "application/gui_application.h"

namespace crossdesk {

Render::Render() : application_(std::make_unique<GuiApplication>()) {}

Render::~Render() = default;

int Render::Run() { return application_->Run(); }

void Render::SetPendingConnectId(const std::string& id) {
  if (!id.empty()) application_->SetPendingConnectId(id);
}
#ifdef _WIN32
int Render::RunUnattended(void* stop_event) {
  return application_->RunUnattended(stop_event);
}
#endif

} // namespace crossdesk
