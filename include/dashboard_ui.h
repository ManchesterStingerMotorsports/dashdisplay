#ifndef DASHBOARD_UI_H
#define DASHBOARD_UI_H

#include "shareddata.h"
#include <memory>

#ifdef _WIN32
#include <functional>

using PreviewInputPoll = std::function<void(SharedData&)>;
int run_dashboard_ui(std::shared_ptr<SharedData> shared_data, const PreviewInputPoll& poll_preview_input = {});
#else
int run_dashboard_ui(std::shared_ptr<SharedData> shared_data);
#endif

#endif
