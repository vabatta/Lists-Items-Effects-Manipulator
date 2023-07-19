#pragma once

#include "Defs.h"

namespace StringFilters {
	static inline constexpr auto rulesSkipToken = "NONE"sv;
	static const srell::regex re_RulesSplitter(R"(,)", srell::regex_constants::optimize);
	static const srell::regex re_RequirementsSplitter(R"(\+)", srell::regex_constants::optimize);

	struct Data {
		std::unordered_set<std::string> requirements;
		std::unordered_set<std::string> exclusions;
		std::unordered_set<std::string> matches;
		std::unordered_set<std::string> wildcards;

		template <typename T>
		bool Passes(const T* form) const;

		Data operator+(const Data& other) const {
			Data result = *this;
			result += other;
			return result;
		}

		Data& operator+=(const Data& other) {
			requirements.insert(other.requirements.begin(), other.requirements.end());
			exclusions.insert(other.exclusions.begin(), other.exclusions.end());
			matches.insert(other.matches.begin(), other.matches.end());
			wildcards.insert(other.wildcards.begin(), other.wildcards.end());
			return *this;
		}
	};

	namespace Factory {
		// [data, aliases]
		std::pair<Data, std::unordered_set<std::string>> ParseStringFilters(const std::string& raw);
	}
}