// #region StdManifest
/**
 * @file main_Json.cpp
 * @brief
 *
 *
 * @author Emanuele Dolis (edoliscom@gmail.com)
 * @version GIT_VERSION: v1.0.0.0-0-dirty
 * @tagged as: SNTP-core
 * @commit hash: g5d100c9 [5d100c9e7fbf8030cd9e50ec7db3b7b6333dbee1]
 * @build ID: P20250828-124058-5d100c9
 *  @compiledSizeInfo begin

    .iram0.text      49 856    .dram0.data  5 144
    .flash.text     148 774    .dram0.bss   4 688
    .flash.appdesc      256    ――――――――――――――――――
    .flash.rodata    43 568    total        9 832
    ―――――――――――――――――――――――                      
    subtotal        242 454                      

    @compiledSizeInfo end
 * @date 2025-08-28
 */

static const char *TAG = "ESP_main_loop";

// #region BuildInfo
namespace ED_SYSINFO {
// compile time GIT status
struct GIT_fwInfo {
  static constexpr const char *GIT_VERSION = "v1.0.0.0-0-dirty";
  static constexpr const char *GIT_TAG = "SNTP-core";
  static constexpr const char *GIT_HASH = "g5d100c9";
  static constexpr const char *FULL_HASH =
      "5d100c9e7fbf8030cd9e50ec7db3b7b6333dbee1";
  static constexpr const char *BUILD_ID = "P20250828-122422-5d100c9";
};
} // namespace ED_SYSINFO
// #endregion
// #endregion

#include "ED_JSON.h"
#include "ED_sysInfo.h"
#include "ED_wifi.h"
// #include "esp_partition.h"
#include <string.h>

#ifdef DEBUG_BUILD
#endif

using namespace ED_JSON;
using namespace ED_SYSINFO;

extern "C" void app_main(void) {
#ifdef DEBUG_BUILD

#endif

  char buffer[18] = "";

  ED_JSON::JsonEncoder encoder;
  ED_JSON::JsonEncoder encoderMAC;

  for (const auto &pair : ESP_MACstorage::getMacMap()) {
    esp_mac_type_t type = pair.first;
    const MacAddress &mac = pair.second;

    char buffer[18];
    std::string macStr = std::string(mac.toString(buffer, sizeof(buffer)));

    encoderMAC.add(std::string(esp_mac_type_str[type]), macStr);
  }
  encoder.add("deviceMACs", encoderMAC);
  encoder.add("intKey", 42);
  encoder.add("boolKey", true);
  encoder.add("nullKey", nullptr);
  encoder.add("arrayKey", std::vector<std::string>{"item1", "item2", "item3"});

  // ED_wifi::WiFiService::subscribeToIPReady(launchSNTPalignment);
  // ED_wifi::WiFiService::launch();

  // Optional hardware setup
  // gpio_set_direction(LED_BUILTIN, GPIO_MODE_OUTPUT);

#ifdef DEBUG_BUILD

#endif

  while (true) {
    // shows the results of the first calls which will happen when network not
    // initialized, afterwards calls will get froper feedback

    vTaskDelay(3000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "JSON Output: %s", encoder.getJson().c_str());
  }
}
