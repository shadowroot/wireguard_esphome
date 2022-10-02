#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/automation.h"
#include "esphome/core/helpers.h"
#include <string>
#include <IPAddress.h>

namespace esphome {
namespace wireguard {

enum WireguardComponentState {
  /** Nothing has been initialized yet. Internal AP, if configured, is disabled at this point. */
  WIFI_COMPONENT_STATE_OFF = 0,
  /** WiFi is in cooldown mode because something went wrong, scanning will begin after a short period of time. */
  WIFI_COMPONENT_STATE_COOLDOWN,
  /** WiFi is in STA-only mode and currently scanning for APs. */
  WIFI_COMPONENT_STATE_STA_SCANNING,
  /** WiFi is in STA(+AP) mode and currently connecting to an AP. */
  WIFI_COMPONENT_STATE_STA_CONNECTING,
  /** WiFi is in STA(+AP) mode and currently connecting to an AP a second time.
   *
   * This is required because for some reason ESPs don't like to connect to WiFi APs directly after
   * a scan.
   * */
  WIFI_COMPONENT_STATE_STA_CONNECTING_2,
  /** WiFi is in STA(+AP) mode and successfully connected. */
  WIFI_COMPONENT_STATE_STA_CONNECTED,
  /** WiFi is in AP-only mode and internal AP is already enabled. */
  WIFI_COMPONENT_STATE_AP,
};

/// Struct for setting static IPs in WireguardComponent.
struct ManualIP {
  IPAddress static_ip;
  IPAddress gateway;
  IPAddress subnet;
  IPAddress dns1;  ///< The first DNS server. 0.0.0.0 for default.
  IPAddress dns2;  ///< The second DNS server. 0.0.0.0 for default.
};


/// This component is responsible for managing the ESP WiFi interface.
class WireguardComponent : public Component {
 public:
  /// Construct a WireguardComponent.
  WireguardComponent();

  void set_sta(const WiFiAP &ap);
  void add_sta(const WiFiAP &ap);

  /** Setup an Access Point that should be created if no connection to a station can be made.
   *
   * This can also be used without set_sta(). Then the AP will always be active.
   *
   * If both STA and AP are defined, then both will be enabled at startup, but if a connection to a station
   * can be made, the AP will be turned off again.
   */
  void set_ap(const WiFiAP &ap);

  void start_scanning();
  void check_scanning_finished();
  void start_connecting(const WiFiAP &ap, bool two);
  void set_fast_connect(bool fast_connect);
  void set_ap_timeout(uint32_t ap_timeout) { ap_timeout_ = ap_timeout; }

  void check_connecting_finished();

  void retry_connect();

  bool can_proceed() override;

  void set_reboot_timeout(uint32_t reboot_timeout);

  bool is_connected();

  void set_power_save_mode(WiFiPowerSaveMode power_save);
  void set_output_power(float output_power) { output_power_ = output_power; }

  // ========== INTERNAL METHODS ==========
  // (In most use cases you won't need these)
  /// Setup WiFi interface.
  void setup() override;
  void dump_config() override;
  /// WIFI setup_priority.
  float get_setup_priority() const override;
  float get_loop_priority() const override;

  /// Reconnect WiFi if required.
  void loop() override;

  bool has_sta() const;
  bool has_ap() const;

  IPAddress get_ip_address();
  std::string get_use_address() const;
  void set_use_address(const std::string &use_address);

  const std::vector<WiFiScanResult> &get_scan_result() const { return scan_result_; }

  IPAddress wifi_soft_ap_ip();

  bool has_sta_priority(const bssid_t &bssid) {
    for (auto &it : this->sta_priorities_)
      if (it.bssid == bssid)
        return true;
    return false;
  }
  float get_sta_priority(const bssid_t bssid) {
    for (auto &it : this->sta_priorities_)
      if (it.bssid == bssid)
        return it.priority;
    return 0.0f;
  }
  void set_sta_priority(const bssid_t bssid, float priority) {
    for (auto &it : this->sta_priorities_)
      if (it.bssid == bssid) {
        it.priority = priority;
        return;
      }
    this->sta_priorities_.push_back(WiFiSTAPriority{
        .bssid = bssid,
        .priority = priority,
    });
  }

 protected:
  static std::string format_mac_addr(const uint8_t mac[6]);
  void setup_ap_config_();
  void print_connect_params_();

  bool wifi_mode_(optional<bool> sta, optional<bool> ap);
  bool wifi_sta_pre_setup_();
  bool wifi_apply_output_power_(float output_power);
  bool wifi_apply_power_save_();
  bool wifi_sta_ip_config_(optional<ManualIP> manual_ip);
  IPAddress wifi_sta_ip_();
  bool wifi_apply_hostname_();
  bool wifi_sta_connect_(WiFiAP ap);
  void wifi_pre_setup_();
  wl_status_t wifi_sta_status_();
  bool wifi_scan_start_();
  bool wifi_ap_ip_config_(optional<ManualIP> manual_ip);
  bool wifi_start_ap_(const WiFiAP &ap);
  bool wifi_disconnect_();

  bool is_captive_portal_active_();

#ifdef ARDUINO_ARCH_ESP8266
  static void wifi_event_callback(System_Event_t *event);
  void wifi_scan_done_callback_(void *arg, STATUS status);
  static void s_wifi_scan_done_callback(void *arg, STATUS status);
#endif

#ifdef ARDUINO_ARCH_ESP32
  void wifi_event_callback_(system_event_id_t event, system_event_info_t info);
  void wifi_scan_done_callback_();
#endif

  std::string use_address_;
  std::vector<WiFiAP> sta_;
  std::vector<WiFiSTAPriority> sta_priorities_;
  WiFiAP selected_ap_;
  bool fast_connect_{false};

  WiFiAP ap_;
  WireguardComponentState state_{WIFI_COMPONENT_STATE_OFF};
  uint32_t action_started_;
  uint8_t num_retried_{0};
  uint32_t last_connected_{0};
  uint32_t reboot_timeout_{};
  uint32_t ap_timeout_{};
  WiFiPowerSaveMode power_save_{WIFI_POWER_SAVE_NONE};
  bool error_from_callback_{false};
  std::vector<WiFiScanResult> scan_result_;
  bool scan_done_{false};
  bool ap_setup_{false};
  optional<float> output_power_;
};

extern WireguardComponent *global_wifi_component;

template<typename... Ts> class WiFiConnectedCondition : public Condition<Ts...> {
 public:
  bool check(Ts... x) override;
};

template<typename... Ts> bool WiFiConnectedCondition<Ts...>::check(Ts... x) {
  return global_wifi_component->is_connected();
}

}  // namespace wifi
}  // namespace esphome
