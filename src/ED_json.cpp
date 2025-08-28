#include "ED_json.h"

#include <esp_log.h>

namespace ED_JSON {

static inline const char *TAG = "ED_JSON";


JsonEncoder::JsonEncoder() { root = cJSON_CreateObject(); }

JsonEncoder::JsonEncoder(cJSON *existing) { root = existing; }

JsonEncoder::~JsonEncoder() {
  if (root) {
    cJSON_Delete(root);
  }
}
void JsonEncoder::add(const std::string &key, const JsonValue &value) {
  if (!root) {
    ESP_LOGW(TAG, "Attempted to add to null root");
    return;
  }

  std::visit(
      [&](auto &&val) {
        using T = std::decay_t<decltype(val)>;

        if constexpr (std::is_same_v<T, JsonEncoder>) {
          const JsonEncoder& nested = std::get<JsonEncoder>(value);
        addObject(key, nested);  
        } else
        if constexpr (std::is_same_v<T, std::string>) {
          cJSON_AddStringToObject(root, key.c_str(), val.c_str());
        } else if constexpr (std::is_same_v<T, int> ||
                             std::is_same_v<T, double>) {
          cJSON_AddNumberToObject(root, key.c_str(), static_cast<double>(val));
        } else if constexpr (std::is_same_v<T, bool>) {
          cJSON_AddBoolToObject(root, key.c_str(), val);
        } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
          cJSON *array = cJSON_CreateArray();
          for (const auto &item : val) {
            cJSON_AddItemToArray(array, cJSON_CreateString(item.c_str()));
          }
          cJSON_AddItemToObject(root, key.c_str(), array);
        } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
          cJSON_AddNullToObject(root, key.c_str());
        } else {
          ESP_LOGE(TAG, "Unsupported JsonValue type for key: %s", key.c_str());
          return; // or handle gracefully
        }
      },
      value);
}

void JsonEncoder::addObject(const std::string &key, const JsonEncoder &nested) {
  if (!root) {
    ESP_LOGW(TAG, "Attempted to add to null root");
    return;
  }

  cJSON_AddItemToObject(root, key.c_str(), cJSON_Duplicate(nested.root, true));
}

JsonEncoder::JsonValue JsonEncoder::getValue(const std::string &key) const {
  if (!root)
    return nullptr;

  cJSON *item = cJSON_GetObjectItem(root, key.c_str());
  if (!item)
    return nullptr;

  if (cJSON_IsString(item)) {
    return std::string(item->valuestring);
  } else if (cJSON_IsNumber(item)) {
    // You can choose to return int or double based on your use case
    double val = item->valuedouble;
    if (val == static_cast<int>(val)) {
      return static_cast<int>(val);
    } else {
      return val;
    }
  } else if (cJSON_IsBool(item)) {
    return static_cast<bool>(cJSON_IsTrue(item));

  } else if (cJSON_IsArray(item)) {
    std::vector<std::string> result;
    cJSON *subItem = nullptr;
    cJSON_ArrayForEach(subItem, item) {
      if (cJSON_IsString(subItem)) {
        result.push_back(subItem->valuestring);
      }
    }
    return result;
  }

  return nullptr;
}

JsonEncoder JsonEncoder::getObject(const std::string &key) const {
  cJSON *item = cJSON_GetObjectItem(root, key.c_str());
  if (item && cJSON_IsObject(item)) {
    return JsonEncoder(cJSON_Duplicate(item, true));
  }

  ESP_LOGE("JsonEncoder", "Key not found or not an object: %s", key.c_str());
  return JsonEncoder(nullptr); // or a default-constructed JsonEncoder
}

 std::string JsonEncoder::getJson(){return cJSON_Print(root);};

} // namespace ED_JSON