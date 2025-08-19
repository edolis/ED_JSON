# ESP-IDF Heapless JSON Stream Parser

This library provides a heapless JSON stream parser for ESP-IDF, allowing you to parse JSON streams without using dynamic memory allocation (heap).

## Features

- Parse JSON streams without using heap memory
- Minimal memory footprint
- Simple integration into ESP-IDF projects
- Supports parsing JSON objects and arrays
- Handles parsing of JSON events from an event source stream

## Usage

1. Include the library in your ESP-IDF project:

   ```bash
   git clone https://github.com/your-username/esp-idf-heapless-json-stream-parser.git
   ```

2. Include the library in your component makefile:

   ```makefile
   COMPONENT_SRCDIRS += path/to/esp-idf-heapless-json-stream-parser
   ```

3. Initialize the JSON stream parser with a callback function to handle parsed JSON objects or arrays:

   ```cpp
   #include "esp_heapless_json_stream_parser.h"

   json_stream_parser jparser([](std::string path, json_val_t value) {
       // Handle parsed JSON tokens here
   });
   ```

4. Initialize the event source stream parser with callbacks for events and data:

   ```cpp
   #include "event_source_stream_parser.h"

   event_source_stream_parser eparser("event", "data", [](std::string event) -> on_evts_data_char_cb_t {
       // Handle event parsing and return a data callback
       return [&](char c) {
           jparser.parse(c); // Parse JSON data from the event
       };
   });
   ```

5. Parse JSON data as it becomes available in your stream input:

   ```cpp
   // Example: Parsing JSON data from an event source stream
   size_t s = strlen(event_source_str);
   int i = 0;

   while (i < s) {
       eparser.parse(event_source_str[i]);
       i++;
   }
   ```

6. Cleanup the parser handle when finished:

   ```cpp
   // No cleanup necessary for heapless JSON stream parser
   ```
   
## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
