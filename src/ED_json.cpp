#include "ED_json.h"

#include <esp_log.h>
#include <unordered_map>

namespace ED_JSON {

static inline const char *TAG = "ED_JSON";


JsonEncoder::JsonEncoder() { root = cJSON_CreateObject(); }

JsonEncoder::JsonEncoder(cJSON *existing) { root = existing; }

JsonEncoder::JsonEncoder(const JsonEncoder& other) {
  // ESP_LOGI(TAG,"in copy root nullptr? %s",other.root==nullptr?"yes":"no");

  root = cJSON_Duplicate(other.root, true); // deep copy
}

JsonEncoder& JsonEncoder::operator=(const JsonEncoder& other) {

  if (this != &other) {
    if (root) cJSON_Delete(root);
    root = cJSON_Duplicate(other.root, true);
  }
  return *this;
}
JsonEncoder::JsonEncoder(const std::string& jsonStr) {
  root = cJSON_Parse(jsonStr.c_str());
}

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

 std::string JsonEncoder::getCompactJson() {
  if (!root) return "{}";
  char* raw = cJSON_PrintUnformatted(root);
  std::string result = raw ? std::string(raw) : "{}";
  cJSON_free(raw);
  return result;
}





std::optional<int> JsonEncoder::getInt(const std::string& key) const {
  if (!root) return std::nullopt;
  cJSON* item = cJSON_GetObjectItem(root, key.c_str());
  if (item && cJSON_IsNumber(item)) {
    return static_cast<int>(item->valuedouble);
  }
  return std::nullopt;
}

std::optional<std::string> JsonEncoder::getString(const std::string& key) const {
  if (!root) return std::nullopt;
  cJSON* item = cJSON_GetObjectItem(root, key.c_str());
  if (item && cJSON_IsString(item)) {
    return std::string(item->valuestring);
  }
  return std::nullopt;
}
// std::string getCompactJson() {
//   return cJSON_PrintUnformatted(root);
// }

bool JsonEncoder::isValidJson() const {
    return root != nullptr && cJSON_IsObject(root);
}

bool JsonEncoder::isArray() const {
  // ESP_LOGI(TAG,"in isarray root nullptr? %s",root==nullptr?"yes":"no");
  // if(root)ESP_LOGI(TAG,"Step_5 %s",cJSON_Print(root)  );
    return root && cJSON_IsArray(root);
}

JsonEncoder JsonEncoder::getArrayItem(int index) const {
    if (!root || !cJSON_IsArray(root)) return JsonEncoder(nullptr);
    cJSON* item = cJSON_GetArrayItem(root, index);
    return item && cJSON_IsObject(item) ? JsonEncoder(cJSON_Duplicate(item, true)) : JsonEncoder(nullptr);
}

int JsonEncoder::getArraySize() const {
  return (root && cJSON_IsArray(root)) ? cJSON_GetArraySize(root) : 0;
}

void JsonEncoder::reset(const std::string& jsonStr) {
  if (root) {
    cJSON_Delete(root);
    root = nullptr;
  }
  root = cJSON_Parse(jsonStr.c_str());
}

void JsonEncoder::setJson(const std::string& jsonStr) {
    if (root) {
        cJSON_Delete(root);
        root = nullptr;
    }
    root = cJSON_Parse(jsonStr.c_str());
}
void JsonEncoder::wrapRootInArray() {
  if (!root) {
    ESP_LOGW(TAG, "Cannot wrap null root in array");
    return;
  }

  cJSON* array = cJSON_CreateArray();
  cJSON_AddItemToArray(array, root); // transfers ownership
  root = array;
}
JsonEncoder JsonEncoder::unwrapNestedArray() const {

  JsonEncoder current(*this);
  // in that case, the funtion would mistakenly return just the first element
  ESP_LOGI(TAG,"Step_unwrapper called with null root? %s",current.root==nullptr?"yes":"no");
  if(current.root) ESP_LOGI(TAG, "unwrap enters while with → %s", cJSON_PrintUnformatted(current.root));

  // Keep unwrapping while we have exactly one element AND that element is itself an array
  while (current.root && current.isArray() && current.getArraySize() == 1) {
    JsonEncoder child = current.getArrayItem(0);
    if (!child.isArray()) break;

    if(child.root) ESP_LOGI(TAG, "unwrap moving pointer to child → %s", cJSON_PrintUnformatted(child.root));
    current = std::move(child);
  }
   if(current.root) ESP_LOGI(TAG, "unwrap exit → %s", cJSON_PrintUnformatted(current.root));

  return current;
}

std::unordered_map<std::string, std::string> JsonEncoder::parseJsonToMap(const std::string& jsonStr) {
    std::unordered_map<std::string, std::string> result;

    cJSON* root = cJSON_Parse(jsonStr.c_str());
    if (!root || !cJSON_IsObject(root)) {
        ESP_LOGW("JSON", "Invalid JSON object: %s", jsonStr.c_str());
        cJSON_Delete(root);
        return result;
    }

    cJSON* item = nullptr;
    cJSON_ArrayForEach(item, root) {
        const char* key = item->string;
        if (!key) continue;

        if (cJSON_IsString(item)) {
            result[key] = item->valuestring;
        } else if (cJSON_IsNumber(item)) {
            result[key] = std::to_string(item->valuedouble);
        } else {
            // Optional: serialize nested objects/arrays as strings
            char* printed = cJSON_PrintUnformatted(item);
            result[key] = printed ? printed : "";
            cJSON_free(printed);
        }
    }

    cJSON_Delete(root);
    return result;
}


} // namespace ED_JSON