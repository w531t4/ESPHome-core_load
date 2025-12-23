// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
// core_load.h — ESP-IDF, FreeRTOS runtime-stats based, per-core CPU load
#pragma once
#include "esphome.h"
extern "C" {
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}
#include <cstring>
#include <vector>

#if !CONFIG_FREERTOS_USE_TRACE_FACILITY
#error "CONFIG_FREERTOS_USE_TRACE_FACILITY must be enabled to use core_load component"
#endif
#if !CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
#error "CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS must be enabled to use core_load component"
#endif

namespace core_load {

class CoreLoadSensorsRTOS : public esphome::PollingComponent {
  public:
    CoreLoadSensorsRTOS() = default;

    void set_core0_sensor(esphome::sensor::Sensor *s) { core0_ = s; }
    void set_core1_sensor(esphome::sensor::Sensor *s) { core1_ = s; }

    void setup() override { last_wall_us_ = esp_timer_get_time(); }

    void update() override {
        // Snapshot all tasks (to find IDLE0 / IDLE1 run-time counters)
        UBaseType_t max_tasks = uxTaskGetNumberOfTasks() + 8;
        std::vector<TaskStatus_t> tasks(max_tasks);
        uint32_t unused_total = 0;
        UBaseType_t n =
            uxTaskGetSystemState(tasks.data(), tasks.size(), &unused_total);
        tasks.resize(n);

        // Find idle counters by name
        uint64_t idle_now[2] = {0, 0};
        bool found[2] = {false, false};
        for (const auto &t : tasks) {
            if (std::strcmp(t.pcTaskName, "IDLE0") == 0) {
                idle_now[0] = (uint64_t)t.ulRunTimeCounter;
                found[0] = true;
            } else if (std::strcmp(t.pcTaskName, "IDLE1") == 0) {
                idle_now[1] = (uint64_t)t.ulRunTimeCounter;
                found[1] = true;
            }
        }

        uint64_t now = esp_timer_get_time();
        uint64_t d_wall = (now >= last_wall_us_) ? (now - last_wall_us_) : 0;
        last_wall_us_ = now;

        // On the very first call, prev_ is 0 — publish after we have deltas
        if (d_wall == 0 || !found[0] || !found[1])
            return;

        publish_core_(0, idle_now[0], d_wall, core0_);
        publish_core_(1, idle_now[1], d_wall, core1_);
    }

  private:
    esphome::sensor::Sensor *core0_{nullptr};
    esphome::sensor::Sensor *core1_{nullptr};
    uint64_t last_wall_us_{0};
    uint64_t prev_idle_us_[2] = {0, 0};

    void publish_core_(int core, uint64_t idle_now, uint64_t d_wall,
                       esphome::sensor::Sensor *out) {
        if (!out)
            return;
        uint64_t prev = prev_idle_us_[core];
        prev_idle_us_[core] = idle_now;
        if (idle_now < prev)
            return; // wrap or first sample

        uint64_t d_idle = idle_now - prev;
        float util = (d_wall == 0)
                         ? 0.0f
                         : 100.0f * (1.0f - (float)d_idle / (float)d_wall);
        if (util < 0.f)
            util = 0.f;
        if (util > 100.f)
            util = 100.f;
        out->publish_state(std::round(util));
    }
};

} // namespace core_load
