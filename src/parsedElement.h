#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

#pragma once

using namespace std;

class ParsedElement {
public:
    // Enum to define possible types
    enum class Type {
        SIMPLE_STRING,
        BULK_STRING,
        INTEGER,
        ARRAY
    };

    // Data members for storing different types of values
    Type type = Type::SIMPLE_STRING;
    string stringValue = "";
    int intValue = 0;
    vector<ParsedElement> arrayValue ;


    // Default constructor
    ParsedElement() = default;

    // Constructor for SIMPLE_STRING
    ParsedElement( string& value) : type(Type::SIMPLE_STRING), stringValue(value) {}

    //constructor for BULK_STRING
    ParsedElement( string& value , int val ) : type(Type::BULK_STRING),stringValue(value) {}

    // Constructor for INTEGER
    ParsedElement(int value) : type(Type::INTEGER), intValue(value) {}

    // Constructor for ARRAY
    ParsedElement(vector<ParsedElement>& value) : type(Type::ARRAY), arrayValue(value) {}

    // Getters for each type
    Type getType() { return type; }

    string getStringValue(){
        if (type != Type::SIMPLE_STRING  && type != Type::BULK_STRING ) {
            throw std::invalid_argument("Type mismatch: Expected SIMPLE_STRING ");
        }
        return stringValue;
    }

    int getIntValue(){
        if (type != Type::INTEGER) {
            throw std::invalid_argument("Type mismatch: Expected Integer");
        }
        return intValue;
    }

    vector<ParsedElement>& getArrayValue(){
        if (type != Type::ARRAY) {
            throw std::invalid_argument("Type mismatch: Expected array ");
        }
        return arrayValue;
    }
};
