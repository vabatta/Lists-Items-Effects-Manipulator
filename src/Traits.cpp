#include "Traits.h"

namespace Traits {
	Data Factory::ParseTraits(const std::string& raw) {
		Data result = {};

		if (raw == rulesSkipToken || raw.empty()) {
			return result;
		}

		std::vector<srell::ssub_match> rules;
		re_RulesSplitter.split(rules, raw);

		for (auto& rule : rules) {
			if (rule.str().empty()) {
				continue;
			}
			// if starts with '-' then it's a negative rule
			bool isNegative = rule.str().at(0) == '-';
			char trait = std::toupper(rule.str().size() == 1 ? rule.str().at(0) : rule.str().at(1));

			switch (trait) {
				case 'E':
					result.isEnchanted = std::make_optional(!isNegative);
					break;

				default:
					TRACE("TR skipped unknown trait: {}", trait);
					break;
			}
		}

		return result;
	}

	template <typename T>
	bool Data::Passes(const T* form) const {
		bool result = true;

		if constexpr (std::derived_from<T, RE::TESEnchantableForm>) {
			auto enchantableForm = static_cast<const RE::TESEnchantableForm*>(form);
			if (isEnchanted.has_value()) {
				result &= isEnchanted.value() ? enchantableForm->formEnchanting != nullptr
																			: enchantableForm->formEnchanting == nullptr;
				TRACE("TR form is enchanted: {}", result);
			}
		}

		return result;
	}

	// forces compilation
	template bool Data::Passes<RE::TESObjectARMO>(const RE::TESObjectARMO* form) const;
	template bool Data::Passes<RE::TESObjectWEAP>(const RE::TESObjectWEAP* form) const;
	template bool Data::Passes<RE::TESAmmo>(const RE::TESAmmo* form) const;
}