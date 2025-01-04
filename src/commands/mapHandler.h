#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#pragma once

class MapHandler {
private:
    std::unordered_map<std::string, std::string> data;

public:
    // Modified setter - args contains only arguments (key value)
    std::string set(const std::vector<std::string>& args) {
        // Check if we have correct number of arguments (key value)
        if (args.size() < 2) {
            throw std::invalid_argument("ERR wrong number of arguments for 'set' command");
        }

        const std::string& key = args[0];    // First argument is key
        const std::string& value = args[1];   // Second argument is value

        if (key.empty()) {
            throw std::invalid_argument("ERR key cannot be empty");
        }
        if (value.empty()) {
            throw std::invalid_argument("ERR value cannot be empty");
        }

        data[key] = value;
        return "+OK\r\n";
    }

    // Modified getter - args contains only arguments (key)
    std::string get(const std::vector<std::string>& args) {
        // Check if we have correct number of arguments (key)
        if (args.empty()) {
            throw std::invalid_argument("ERR wrong number of arguments for 'get' command");
        }

        const std::string& key = args[0];    // First argument is key

        if (key.empty()) {
            throw std::invalid_argument("ERR key cannot be empty");
        }

        auto it = data.find(key);
        if (it != data.end()) {
            const std::string& value = it->second;
            return "$" + std::to_string(value.length()) + "\r\n" + value + "\r\n";
        }
        return "$-1\r\n";  // Redis null reply
    }
};
