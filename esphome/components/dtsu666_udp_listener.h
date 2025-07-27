#pragma once
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/log.h"
#include <WiFiUdp.h>

namespace esphome {
namespace dtsu666_udp_listener {

class ModbusUdpListener : public Component {
 public:
  void set_port(uint16_t port) { port_ = port; }
  void set_device_id(uint8_t id) { device_id_ = id; has_device_id_ = true; }

  void set_pt_sensor(sensor::Sensor *s) { pt_ = s; }
  void set_pa_sensor(sensor::Sensor *s) { pa_ = s; }
  void set_pb_sensor(sensor::Sensor *s) { pb_ = s; }
  void set_pc_sensor(sensor::Sensor *s) { pc_ = s; }
  void set_qt_sensor(sensor::Sensor *s) { qt_ = s; }
  void set_qa_sensor(sensor::Sensor *s) { qa_ = s; }
  void set_qb_sensor(sensor::Sensor *s) { qb_ = s; }
  void set_qc_sensor(sensor::Sensor *s) { qc_ = s; }

  void setup() override;
  void loop() override;
  void dump_config() override;

 protected:
  bool parse_packet_(const uint8_t *data, size_t len);
  static float be_float_(const uint8_t *p) {
    uint32_t u = (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
                 (uint32_t(p[2]) << 8) | uint32_t(p[3]);
    float f;
    memcpy(&f, &u, sizeof(f));
    return f;
  }

  WiFiUDP udp_;
  uint16_t port_{2024};
  bool has_device_id_{false};
  uint8_t device_id_{1};

  sensor::Sensor *pt_{nullptr};
  sensor::Sensor *pa_{nullptr};
  sensor::Sensor *pb_{nullptr};
  sensor::Sensor *pc_{nullptr};
  sensor::Sensor *qt_{nullptr};
  sensor::Sensor *qa_{nullptr};
  sensor::Sensor *qb_{nullptr};
  sensor::Sensor *qc_{nullptr};
};

}  // namespace dtsu666_udp_listener
}  // namespace esphome
