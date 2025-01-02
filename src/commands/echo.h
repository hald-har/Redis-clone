#include <sstream>
#include <string>
#include <vector>

#pragma once

class EchoHandler{
public:

  EchoHandler(std::vector<std::string>& args) : arguments(args) {}
  std::string getResponse() {
        return encodeRESP();
    }

private:

  std::vector<std::string> arguments ;
  std::string encodeRESP() {
        std::ostringstream resp;
        resp << "*" << arguments.size() << "\r\n";

        for (std::string& arg : arguments) {
            resp << "$" << arg.size() << "\r\n" << arg << "\r\n";
        }

        return resp.str();
    }

};
