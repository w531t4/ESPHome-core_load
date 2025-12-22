# SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
# SPDX-License-Identifier: MIT

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, CONF_UPDATE_INTERVAL, UNIT_PERCENT, ICON_CHIP

core_load_ns = cg.esphome_ns.namespace("core_load")
CoreLoadSensorsRTOS = core_load_ns.class_("CoreLoadSensorsRTOS", cg.PollingComponent)

CONF_CORE0 = "core0"
CONF_CORE1 = "core1"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(CoreLoadSensorsRTOS),
    cv.Required(CONF_CORE0): sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT, accuracy_decimals=0, icon=ICON_CHIP),
    cv.Required(CONF_CORE1): sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT, accuracy_decimals=0, icon=ICON_CHIP),
}).extend(cv.polling_component_schema("1s"))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    s0 = await sensor.new_sensor(config[CONF_CORE0])
    s1 = await sensor.new_sensor(config[CONF_CORE1])
    cg.add(var.set_core0_sensor(s0))
    cg.add(var.set_core1_sensor(s1))
