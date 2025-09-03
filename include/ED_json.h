#pragma once


#include <string>
#include <vector>
#include "cJSON.h"
#include <variant>
#include <optional>
#include <unordered_map>



namespace ED_JSON {

/// forward declarations to allow populate the pointer members of JsonConversionArgs
/// pointers are passed to avoid recompilation of classes implementing the interface
/// in case of changes in their structure
struct ED_JsonOptions{}; //used to pass reporting options request from the caller
struct ED_JsonData{};


    /*
std:monostate allows calls with empty variant such as:
    using MyVariantValue = std::variant<std::monostate, int, std::string>;
MyVariantValue empty_value; // Creates a std::monostate
my_interface_instance.processData(empty_value);
    */

    using JsonConversionArgs = std::variant<
    std::monostate //to allow empty variants
    ,ED_JsonOptions*
    ,ED_JsonData*
    // ,int   // verify if there are use cases justifying further types
    // ,double
    // ,std::string
    // ,std::vector<std::string>
>;



/**
 * @brief Interface which must be implemented by data types so that thwy can be transmitted via MQTT
 *
 */
class IJsonEncodable {
public:
    virtual ~IJsonEncodable() = default; // Good practice to include a virtual destructor
    virtual std::string toJson(const JsonConversionArgs& data) const = 0;
};




class JsonEncoder {
    public:
    // Define the variant type for dynamic JSON values
    using JsonValue = std::variant<
    JsonEncoder, // to allow recursive inclusions
    std::string,
        int,
        double,
        bool,
        std::vector<std::string>,
        std::nullptr_t
        >;

        // Constructor and destructor
        JsonEncoder();
        ~JsonEncoder(); //don't want it to shouw up in intellisense method list.
    /** wraps an existing cJSON object in the helper */
    explicit JsonEncoder(cJSON* existing);
    JsonEncoder(const JsonEncoder& other);
    JsonEncoder& operator=(const JsonEncoder& other) ;

    void add(const std::string &key, const JsonValue &value);
    /// @brief adds a json object.
    /// @param key
    /// @param nested the JsonEncoder object to add
    void addObject(const std::string &key, const JsonEncoder &nested);

    // Unified accessor method
    JsonValue getValue(const std::string& key) const;
    JsonEncoder getObject(const std::string &key) const;
    // returns the encoded Json string, formatted for easier readability
    std::string getJson();
//gets the compact unformatted Json string
    std::string getCompactJson();
    JsonEncoder(const std::string &jsonStr);
    std::optional<int> getInt(const std::string& key) const;
    std::optional<std::string> getString(const std::string &key) const;
    bool isValidJson() const;
    bool isArray() const;

    JsonEncoder getArrayItem(int index) const;
    int getArraySize() const;
    // resets by deleting the internal object
    void reset(const std::string &jsonStr);
    //changes che Json string parsed by the encoder
    void setJson(const std::string &jsonStr);
    //wraps a given json in [] turning into an array
    void wrapRootInArray();
    // unwraps a json from redundant array enclosures generated during transmissiom of data
    JsonEncoder unwrapNestedArray() const;
//parses a Json string into a map<string,string>
    static std::unordered_map<std::string, std::string> parseJsonToMap(const std::string &jsonStr);

private:
    cJSON *root=nullptr;

};



} //namespace ED_JSON