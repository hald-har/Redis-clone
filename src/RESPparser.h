
#include "parsedElement.h"
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <algorithm>

#pragma once

class Parser{

public:
    // Parses a RESP string and populates the given ParsedElement object
    void parse( char* input, ParsedElement& element) {
        std::istringstream stream(input);

        char prefix ;
        stream >> prefix ;

        switch(prefix){
            case '+': // Simple String
                parseSimpleString(stream, element);
                break;
            case ':': // Integer
                parseInteger(stream, element);
                break;
            case '$': // Bulk String
                parseBulkString(stream, element);
                break;
            case '*': // Array
                parseArray(stream, element);
                break;
            default:
                throw std::invalid_argument("Unknown RESP type prefix");
        }
    }


     // Helper methods for parsing each RESP type

     //Parse Simple String
     void parseSimpleString(std::istringstream& stream , ParsedElement& element){
       std::string value ;
       std::getline(stream , value , '\r') ; // Read until \r
       stream.get() ;                        // Consume the \n

       element.type = ParsedElement::Type::SIMPLE_STRING;
       element = ParsedElement(value) ;
     }

     //Parse INTEGER
     void parseInteger(std::istringstream& stream , ParsedElement& element){
       int value;
        stream >> value;
        stream.get();                     // Consume the \n

        element.type = ParsedElement::Type::INTEGER;
        element = ParsedElement(value);   // Set the ParsedElement
     }

    //Parse Bulk String
     void parseBulkString(std::istringstream& stream, ParsedElement& element){
       int len ;
        stream >> len ;

        stream.get() ; //consume '\r'
        stream.get() ; //consume '\n'

        if( len == -1 ){
          element = ParsedElement(); // Null bulk string
          return;
        }

        std::string value(len , '\0') ;
        stream.read( &value[0] , len ) ;

        stream.get() ;
        stream.get() ;

        element.type = ParsedElement::Type::BULK_STRING ;
        element = ParsedElement(value , -1) ;
     }

     //Parse ARRAY
     void parseArray(std::istringstream& stream , ParsedElement& element){
       int len ;
       stream >> len ;

       stream.get() ; //consume '\r'
       stream.get() ; //consume '\n'

       std::vector<ParsedElement> arrayElements;

       for( int i = 0 ; i<len ; i++){
         char prefix = stream.get() ;
         ParsedElement subElement;
            switch (prefix) {
                case '+':
                    parseSimpleString(stream, subElement);
                    break;
                case ':':
                    parseInteger(stream, subElement);
                    break;
                case '$':
                    parseBulkString(stream, subElement);
                    break;
                case '*':
                    parseArray(stream, subElement); // Recursive call for nested arrays
                    break;
                default:
                    throw std::invalid_argument("Unknown RESP type in array - here");
            }
            arrayElements.push_back(subElement);
       }

       element.type = ParsedElement::Type::ARRAY;
       element = ParsedElement(arrayElements) ;
     }


     std::pair<std::string ,std::vector<string>>extractFromArray(ParsedElement& element){
       auto array = element.getArrayValue() ;

       if( array.empty() ){
         throw std::invalid_argument("Array must not be empty.");
       }

       if (array[0].getType() != ParsedElement::Type::BULK_STRING) {
         throw std::invalid_argument("First element of the array must be a bulk string command.");
       }

        std::string command = array[0].getStringValue();

        // Normalize the command to lowercase
        std::transform(command.begin(), command.end(), command.begin(), ::tolower) ;

        // Remaining elements as arguments
        std::vector<std::string> arguments;
      for(size_t i = 1; i < array.size(); ++i){
        if (array[i].getType() == ParsedElement::Type::BULK_STRING || array[i].getType() == ParsedElement::Type::SIMPLE_STRING) {
            arguments.push_back(array[i].getStringValue());
        } else if (array[i].getType() == ParsedElement::Type::INTEGER) {
            arguments.push_back(std::to_string(array[i].getIntValue()));
        } else {
            throw std::invalid_argument("Unsupported argument type.");
        }
    }

    return {command, arguments};

     }


    std::pair<std::string, std::vector<std::string>> extractFromString(ParsedElement &element) {
    if (element.getType() != ParsedElement::Type::BULK_STRING && element.getType() != ParsedElement::Type::SIMPLE_STRING) {
        throw std::invalid_argument("Element must be a bulk or simple string.");
    }

    // Command is the string itself
    std::string command = element.getStringValue();

    // Normalize the command to lowercase
    std::transform(command.begin(), command.end(), command.begin(), ::tolower) ;

    // No arguments
    return {command, {}};
}



     std::pair<std::string, std::vector<std::string>> extractCommandAndArguments(ParsedElement& element){
       switch( element.getType() ){
         case ParsedElement::Type::ARRAY:
              return extractFromArray(element) ;
         case ParsedElement::Type::BULK_STRING:
         case ParsedElement::Type::SIMPLE_STRING:
              return extractFromString(element) ;
         default:
             throw std::invalid_argument("Unsupported type for command extraction.");
         }
     }

};
