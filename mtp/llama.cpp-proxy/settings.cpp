#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// Assuming the header provided is in "Settings.h"
#include "Settings.h"

class JsonParser {
private:
    std::ifstream& is;

    void skip_whitespace() {
        while (is.peek() != EOF && (is.peek() == ' ' || is.peek() == '\n' || is.peek() == '\r' || is.peek() == '\t')) {
            is.get();
        }
    }

    void expect(char c) {
        skip_whitespace();
        if (is.get() != c) {
            throw std::runtime_error("Expected character");
        }
    }

    void expect(const std::string& s) {
        skip_whitespace();
        std::string actual(s.length(), ' ');
        is.read(&actual[0], s.length());
        if (actual != s) {
            throw std::runtime_error("Expected string: " + s);
        }
    }

    std::string parse_string() {
        skip_whitespace();
        if (is.get() != '"') throw std::runtime_error("Expected quote");
        std::string s;
        char c;
        while (is.get(c) && c != '"') {
            s += c;
        }
        return s;
    }

public:
    JsonParser(std::ifstream& input) : is(input) {}

    void parse_value(Settings::Replace& replace, Settings::When& when, std::string& current_key) {
        skip_whitespace();
        char peek = is.peek();

        if (peek == '"') {
            std::string val = parse_string();
            // Map JSON keys to struct members
            if (current_key == "from") replace.from = val; // Temporary scope check
            else if (current_key == "to") replace.to = val;
            else if (current_key == "from-value") replace.to_value = val;
            else if (current_key == "to-value") replace.to_value = val;
        }
        else if (peek == '{') {
            // Object handling is managed by the parent caller's context
        }
        else if (peek == '[') {
            // Array handling
        }
    }

    // Recursive Descent for Settings structure
    void parse_settings(Settings& s) {
        expect('{');
        while (is.peek() != '}' && !is.eof()) {
            std::string key = parse_string();
            expect(':');
            skip_whitespace();

            if (key == "forward to") {
                s.forward_to = parse_string();
            }
            else if (key == "replace") {
                expect('[');
                while (is.peek() != ']' && !is.eof()) {
                    s.replace_rules.push_back(parse_replace_item());
                    skip_whitespace();
                    if (is.peek() == ',') expect(',');
                }
                expect(']');
            }

            skip_whitespace();
            if (is.peek() == ',')
                expect(',');
        }
        expect('}');
    }

    Settings::Replace parse_replace_item() {
        Settings::Replace r;
        expect('{');
        while (is.peek() != '}' && !is.eof()) {
            std::string key = parse_string();
            expect(':');
            skip_whitespace();

            if (key == "from") r.from = parse_string();
            else if (key == "to") r.to = parse_string();
            else if (key == "from-value") r.from_value = parse_string();
            else if (key == "to-value") r.to_value = parse_string();
            else if (key == "when") {
                expect('[');
                while (is.peek() != ']' && !is.eof()) {
                    r.when.push_back(parse_when_item());
                    skip_whitespace();
                    if (is.peek() == ',') expect(',');
                }
                expect(']');
            }
            else {
                parse_string(); // skip unknown
            }

            skip_whitespace();
            if (is.peek() == ',') expect(',');
        }
        expect('}');
        return r;
    }

    Settings::When parse_when_item() {
        Settings::When w;
        expect('{');
        while (is.peek() != '}' && !is.eof()) {
            std::string key = parse_string();
            expect(':');
            skip_whitespace();

            if (key == "arg") w.arg = parse_string();
            else if (key == "value" || key == "vlaue") w.value = parse_string(); // Handling user typo
            else parse_string();

            skip_whitespace();
            if (is.peek() == ',') expect(',');
        }
        expect('}');
        return w;
    }
};

// Implementation of the Settings Constructor
Settings::Settings(char const* path_to_json) {
    std::ifstream file(path_to_json);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    JsonParser parser(file);
    parser.parse_settings(*this);
}
