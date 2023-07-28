#pragma once

namespace Chance {
	static inline constexpr auto rulesSkipToken = "NONE"sv;

	struct Data {
		std::optional<uint32_t> chance = std::nullopt;

		template <typename T>
		bool Passes(const T* form) const;

		Data operator+(const Data& other) const {
			Data result = *this;
			result += other;
			return result;
		}

		Data& operator+=(const Data& other) {
			// no overwrite if this object has a value
			if (!chance.has_value()) {
				chance = other.chance;
			}
			return *this;
		}
	};

	namespace Factory {
		Data ParseChance(const std::string& raw);
	}
}