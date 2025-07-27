import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    DEVICE_CLASS_POWER,
    STATE_CLASS_MEASUREMENT,
)

CONF_ID = "id"
CONF_PORT = "port"
CONF_DEVICE_ID = "device_id"

AUTO_LOAD = ["sensor"]
DEPENDENCIES = ["wifi"]

ns = cg.global_ns.namespace("dtsu666_udp_listener")
ModbusUdpListener = ns.class_("ModbusUdpListener", cg.Component)

SENSORS = {
    "pt": ("W", DEVICE_CLASS_POWER),
    "pa": ("W", DEVICE_CLASS_POWER),
    "pb": ("W", DEVICE_CLASS_POWER),
    "pc": ("W", DEVICE_CLASS_POWER),
    "qt": ("var", None),
    "qa": ("var", None),
    "qb": ("var", None),
    "qc": ("var", None),
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(ModbusUdpListener),
        cv.Optional(CONF_PORT, default=2024): cv.port,
        cv.Optional(CONF_DEVICE_ID): cv.int_range(min=0, max=247),
        **{
            cv.Optional(k): sensor.sensor_schema(
                unit_of_measurement=u,
                accuracy_decimals=1,
                device_class=dc,
                state_class=STATE_CLASS_MEASUREMENT,
            )
            for k, (u, dc) in SENSORS.items()
        },
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_port(config[CONF_PORT]))
    if CONF_DEVICE_ID in config:
        cg.add(var.set_device_id(config[CONF_DEVICE_ID]))
    for k in SENSORS.keys():
        if k in config:
            sens = await sensor.new_sensor(config[k])
            getattr(var, f"set_{k}_sensor")(sens)
