#pragma once

#include "Defs.h"
#include "Modifier.h"
#include "StringFilters.h"
#include "FormFilters.h"
#include "Traits.h"

namespace Runner {
	static inline constexpr auto suffix = "_LIEM"sv;
	static const srell::regex re_Sections(R"(\|)", srell::regex_constants::optimize);

	static std::string Sanitize(const std::string& raw);

	static inline std::size_t Hashify(const std::string& raw);

	struct Appliers {
		StringFilters::Data stringFilters;
		FormFilters::Data formFilters;
		Traits::Data traits;
		// TODO: add other appliers here

		template <typename T>
		bool Passes(const T* form) const;
	};

	struct Data {
		// hash(appliers/alias name) -> appliers
		std::map<std::size_t, Appliers> appliers;

		// type -> vector<hash(mod)>
		std::map<LIEM::RuleType, std::vector<std::size_t>> order;
		// hash(mod) -> pair<modifier, ref<appliers>>
		std::unordered_map<std::size_t, std::pair<Modifier::Data<LIEM::RuleType::ARMOR>, std::reference_wrapper<Appliers>>> armors;
		std::unordered_map<std::size_t, std::pair<Modifier::Data<LIEM::RuleType::AMMO>, std::reference_wrapper<Appliers>>> ammos;
		std::unordered_map<std::size_t, std::pair<Modifier::Data<LIEM::RuleType::WEAPON>, std::reference_wrapper<Appliers>>> weapons;

		template <LIEM::RuleType R>
		constexpr auto& GetRules() {
			if constexpr (R == LIEM::RuleType::ARMOR) {
				return armors;
			} else if constexpr (R == LIEM::RuleType::AMMO) {
				return ammos;
			} else if constexpr (R == LIEM::RuleType::WEAPON) {
				return weapons;
			}
		}
	};

	std::vector<std::string> GetFiles();

	Data ProcessFiles(const std::vector<std::string>& fileList);
}
