#pragma once

#include "Settings.h"
#include <string>
#include <optional>
#include <list>

class CommandProcessor {
public:
    /**
     * @brief Initializes the processor with command line arguments and settings.
     * @param argc Number of arguments
     * @param argv Array of argument strings
     * @param settings The rules loaded from JSON
     */
    CommandProcessor(int argc, char** argv, const Settings& settings);

    /**
     * @brief Applies replacement rules and reconstructs the command string.
     * @return A string containing the modified command line.
     */
    std::string get_command();

private:
    Settings m_settings;
    std::list<std::pair<std::string, std::optional<std::string>>> m_args;
};
