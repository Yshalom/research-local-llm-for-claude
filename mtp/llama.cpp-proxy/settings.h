#pragma once

#include <string>
#include <list>
#include <optional>

struct Settings
{
public:
	struct When
	{
	public:
		std::optional<std::string> arg;
		std::optional<std::string> value;
	};

	struct Replace
	{
	public:
		std::optional<std::string> from = std::nullopt;
		std::optional<std::string> to = std::nullopt;
		std::optional<std::string> from_value = std::nullopt;
		std::optional<std::string> to_value = std::nullopt;
		std::list<When> when;
	};

// -----------------------------------------
	/// <summary>
	/// The path to the program whom the proxy is for.
	/// </summary>
	std::string forward_to;

	std::list<Replace> replace_rules;

	Settings(char const* path_to_json);
};
