#include "command_processor.h"
#include <sstream>

CommandProcessor::CommandProcessor(int argc, char** argv, const Settings& settings)
    : m_settings(settings)
{
    // We start from i = 1 to skip the program name (the proxy itself)
    for (int i = 1; i < argc; ++i) {
        std::pair<std::string, std::optional<std::string>> arg{ argv[i], std::nullopt };

        // Check if the current argument is a flag (starts with '-')
        if (!arg.first.empty() && arg.first[0] == '-') {
            // Check if the next argument exists and is a value (not a flag)
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                arg.second = argv[i + 1];
                i++;
            }
        }

        m_args.push_back(arg);
    }

    auto m_args_copy = m_args;

    // Apply semantic replacement rules
    for (const auto& rule : m_settings.replace_rules) {
        // If there's no when conditions at the rule, perform the rule!
        bool all_when_conditions_met = (rule.when.size() == 0);
        for (const auto& condition : rule.when) {
            if (!condition.arg && !condition.value)
                break;
            // Check if the argument exists and the value matches exactly
            for (auto arg : m_args_copy) {
                // If the argument is empty, search only for the value
                // If the value is empty, search only for the argument
                if ((!condition.arg || condition.arg.value() == arg.first)
                    && (!condition.value || (arg.second && condition.value.value() == arg.second.value())))
                    all_when_conditions_met = true;
            }
        }

        if (all_when_conditions_met) {
            for (auto iarg = m_args.begin(); iarg != m_args.end(); ++iarg) {                
                // If there's a from, replace the argument (and its value)
                if (rule.from && iarg->first == rule.from.value())
                {
                    // If there's no from-value, always replace
                    if (!rule.from_value || (iarg->second && iarg->second.value() == rule.from_value.value())) {
                        if (rule.to)
                            iarg->first = rule.to.value();
                        if (rule.to_value && iarg->second)
                            iarg->second = rule.to_value.value();
                    }
                }
            }            
        }
    }
}

std::string CommandProcessor::get_command() {
    std::stringstream ss;

    // 1. Start with the target program
    ss << m_settings.forward_to;

    // 2. Reconstruct the argument list
    for (auto& const arg : m_args) {
        if (arg.first != "")
            ss << " " << arg.first;
        if (arg.second && arg.second.value() != "")
            ss << " " << arg.second.value();
    }

    return ss.str();
}
