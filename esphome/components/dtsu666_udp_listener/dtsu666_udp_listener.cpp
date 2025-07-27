#include "dtsu666_udp_listener.h"

namespace esphome {
namespace dtsu666_udp_listener {

static const char *const TAG = "dtsu666_udp";

void ModbusUdpListener::setup() {
  this->udp_.begin(this->port_);
  ESP_LOGI(TAG, "Listening for UDP on port %u", this->port_);
}

void ModbusUdpListener::loop() {
  int packet_size = this->udp_.parsePacket();
  while (packet_size > 0) {
    uint8_t buf[700];
    size_t len = this->udp_.read(buf, sizeof(buf));
    // Log raw UDP packet data as hex string (first 64 bytes max)
    char hexbuf[3 * 64 + 1];
    size_t hexlen = len > 64 ? 64 : len;
    for (size_t i = 0; i < hexlen; i++) {
      sprintf(&hexbuf[i * 3], "%02X ", buf[i]);
    }
    hexbuf[hexlen * 3] = 0;
    ESP_LOGI(TAG, "Raw UDP packet (%u bytes): %s", (unsigned)len, hexbuf);
    this->parse_packet_(buf, len);
    packet_size = this->udp_.parsePacket();
  }
}

void ModbusUdpListener::dump_config() {
  ESP_LOGCONFIG(TAG, "DTU Modbus UDP listener");
  ESP_LOGCONFIG(TAG, "  Port: %u", this->port_);
  if (this->has_device_id_) ESP_LOGCONFIG(TAG, "  Device ID filter: %u", this->device_id_);
  LOG_SENSOR("  ", "Pt (kW)", this->pt_);
  LOG_SENSOR("  ", "Pa (kW)", this->pa_);
  LOG_SENSOR("  ", "Pb (kW)", this->pb_);
  LOG_SENSOR("  ", "Pc (kW)", this->pc_);
  LOG_SENSOR("  ", "Qt (kvar)", this->qt_);
  LOG_SENSOR("  ", "Qa (kvar)", this->qa_);
  LOG_SENSOR("  ", "Qb (kvar)", this->qb_);
  LOG_SENSOR("  ", "Qc (kvar)", this->qc_);
}

// Scan for: <id> 0x03 0x16 (22 data bytes) … (we ignore CRC in UDP payloads)
// Values are 11 x float16 (2 bytes, big-endian); we publish 4..11 (indexes 3..10) scaled by 0.1
bool ModbusUdpListener::parse_packet_(const uint8_t *data, size_t len) {
  // Minimum length: 1 (id) + 1 (func) + 1 (len) + 24 (data) = 27 bytes
  if (len < 57) {
    ESP_LOGW(TAG, "UDP packet too short (%u bytes)", (unsigned)len);
    return false;
  }

  uint8_t id = data[0];
  if (data[1] != 0x03 || data[2] != 0x20 || data[3] != 0x0C) {
    ESP_LOGW(TAG, "UDP packet does not match expected header");
    return false;
  }
  if (this->has_device_id_ && id != this->device_id_) {
    ESP_LOGW(TAG, "Device ID %u does not match filter %u", id, this->device_id_);
    return false;
  }

  float regs[11];
  const uint8_t *p = &data[6];
  for (int k = 0; k < 11; k++, p += 2) {
    regs[k] = be_float16_(p, 10.0f);  // scale by 10 if needed
    ESP_LOGI(TAG, "regs[%d] = %f", k, regs[k]);
  }

  if (this->pt_) this->pt_->publish_state(regs[3]);
  if (this->pa_) this->pa_->publish_state(regs[4]);
  if (this->pb_) this->pb_->publish_state(regs[5]);
  if (this->pc_) this->pc_->publish_state(regs[6]);
  if (this->qt_) this->qt_->publish_state(regs[7]);
  if (this->qa_) this->qa_->publish_state(regs[8]);
  if (this->qb_) this->qb_->publish_state(regs[9]);
  if (this->qc_) this->qc_->publish_state(regs[10]);

  ESP_LOGI(TAG, "Published sensor values from matched frame.");

  return true;  // handled one frame
}

}  // namespace dtsu666_udp_listener
}  // namespace esphome
