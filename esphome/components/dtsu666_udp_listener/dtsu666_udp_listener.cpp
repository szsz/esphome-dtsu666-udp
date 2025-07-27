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
  LOG_SENSOR("  ", "Pt (W x0.1)", this->pt_);
  LOG_SENSOR("  ", "Pa (W x0.1)", this->pa_);
  LOG_SENSOR("  ", "Pb (W x0.1)", this->pb_);
  LOG_SENSOR("  ", "Pc (W x0.1)", this->pc_);
  LOG_SENSOR("  ", "Qt (var x0.1)", this->qt_);
  LOG_SENSOR("  ", "Qa (var x0.1)", this->qa_);
  LOG_SENSOR("  ", "Qb (var x0.1)", this->qb_);
  LOG_SENSOR("  ", "Qc (var x0.1)", this->qc_);
}

// Scan for: <id> 0x03 0x2C (44 data bytes) … (we ignore CRC in UDP payloads)
// Values are 11 x float32 BE; we publish 4..11 (indexes 3..10) scaled by 0.1
bool ModbusUdpListener::parse_packet_(const uint8_t *data, size_t len) {
  for (size_t i = 0; i + 3 + 44 <= len; i++) {
    if (data[i + 1] != 0x03 || data[i + 2] != 0x2C) continue;
    uint8_t id = data[i];
    if (this->has_device_id_ && id != this->device_id_) continue;

    float regs[11];
    const uint8_t *p = &data[i + 3];
    for (int k = 0; k < 11; k++, p += 4) regs[k] = be_float_(p);

    constexpr float SCALE = 0.1f;
    if (this->pt_) this->pt_->publish_state(regs[3] * SCALE);
    if (this->pa_) this->pa_->publish_state(regs[4] * SCALE);
    if (this->pb_) this->pb_->publish_state(regs[5] * SCALE);
    if (this->pc_) this->pc_->publish_state(regs[6] * SCALE);
    if (this->qt_) this->qt_->publish_state(regs[7] * SCALE);
    if (this->qa_) this->qa_->publish_state(regs[8] * SCALE);
    if (this->qb_) this->qb_->publish_state(regs[9] * SCALE);
    if (this->qc_) this->qc_->publish_state(regs[10] * SCALE);

    return true;  // handled one frame
  }
  return false;
}

}  // namespace dtsu666_udp_listener
}  // namespace esphome
