#pragma once


#include <string>
#include <vector>
#include "cJSON.h"
#include <variant>



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

    void add(const std::string &key, const JsonValue &value);
    /// @brief adds a json object.
    /// @param key
    /// @param nested the JsonEncoder object to add
    void addObject(const std::string &key, const JsonEncoder &nested);

    // Unified accessor method
    JsonValue getValue(const std::string& key) const;
    JsonEncoder getObject(const std::string &key) const;
    // returns the encoded Json string
    std::string getJson();

    private:
    cJSON* root;
};



} //namespace ED_JSON