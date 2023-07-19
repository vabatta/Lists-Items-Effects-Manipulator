#pragma once

#include "Defs.h"

namespace Modifier {
	enum class Type {
		ABS,	// no sign
		ADD,	// prefixed +
		SUB		// prefixed -
	};

	template<typename N>
	using modnum_t = std::pair<Type, N>;

	static inline constexpr auto rulesSkipToken = "NONE"sv;
	static const srell::regex re_RulesSplitter(R"(,)", srell::regex_constants::optimize);

	static const srell::regex re_ModifierNumber(R"(([+-])?(\d*(\.\d+)?))", srell::regex_constants::optimize);

	template <typename M, typename V>
	static inline void ApplyModifier(const M& modifier, V& value);

	template <LIEM::RuleType>
	struct Data {};

	template <>
	struct Data<LIEM::RuleType::ARMOR> {
		std::optional<modnum_t<int32_t>> value = std::nullopt;
		std::optional<modnum_t<float>> weight = std::nullopt;
		std::optional<modnum_t<float>> rating = std::nullopt;

		void Apply(RE::TESObjectARMO* armor) const;
		void Apply(std::vector<RE::TESObjectARMO*>& armors) const;
	};

	template<>
	struct Data<LIEM::RuleType::AMMO> {
		std::optional<modnum_t<int32_t>> value = std::nullopt;
		std::optional<modnum_t<float>> weight = std::nullopt;
		std::optional<modnum_t<float>> damage = std::nullopt;

		void Apply(RE::TESAmmo* ammo) const;
		void Apply(std::vector<RE::TESAmmo*>& ammos) const;
	};

	template<>
	struct Data<LIEM::RuleType::WEAPON> {
		std::optional<modnum_t<int32_t>> value = std::nullopt;
		std::optional<modnum_t<float>> weight = std::nullopt;
		std::optional<modnum_t<uint16_t>> damage = std::nullopt;
		std::optional<modnum_t<uint16_t>> critDamage = std::nullopt;
		std::optional<modnum_t<float>> speed = std::nullopt;
		std::optional<modnum_t<float>> reach = std::nullopt;
		std::optional<modnum_t<float>> stagger = std::nullopt;

		void Apply(RE::TESObjectWEAP* weapon) const;
		void Apply(std::vector<RE::TESObjectWEAP*>& weapons) const;
	};

	using DataVariant = std::variant<
		Data<LIEM::RuleType::ARMOR>,
		Data<LIEM::RuleType::AMMO>,
		Data<LIEM::RuleType::WEAPON>
	>;

	namespace Factory {
		template <typename T>
		static std::optional<modnum_t<T>> ParseModifierNumber(const std::string& raw);

		template <LIEM::RuleType K>
		std::optional<Data<K>> ParseData(const std::string& raw);
	}
}
