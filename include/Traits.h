#pragma once

namespace Traits {
	static inline constexpr auto rulesSkipToken = "NONE"sv;
	static const srell::regex re_RulesSplitter(R"(\/)", srell::regex_constants::optimize);

	// NOTE this way of storing traits is not very efficient, because of the little overlap between the form types but
	// it's the easiest to implement
	struct Data {
		std::optional<bool> isEnchanted;

		template <typename T>
		bool Passes(const T* form) const;

		Data operator+(const Data& other) const {
			Data result = *this;
			result += other;
			return result;
		}

		Data& operator+=(const Data& other) {
			// no overwrite if this object has a value
			if (!isEnchanted.has_value()) {
				isEnchanted = other.isEnchanted;
			}
			return *this;
		}
	};

	namespace Factory {
		Data ParseTraits(const std::string& raw);
	}
}