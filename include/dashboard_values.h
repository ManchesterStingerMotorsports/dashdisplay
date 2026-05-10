#ifndef DASHBOARD_VALUES_H
#define DASHBOARD_VALUES_H

#include "shareddata.h"
#include <memory>

void init_dashboard_values();
void update_dashboard_values(std::shared_ptr<SharedData> shared_data);

#endif
