#pragma once

namespace LIEM {
	enum class RuleType : std::uint32_t {
		ALIAS,

		ARMOR,
		AMMO,
		WEAPON
	};

	enum class SectionType : std::uint32_t { MODIFIERS, STRING_FILTERS, FORM_FILTERS, TRAITS, CHANCE, LOAD_CONDITIONS };

	const inline std::unordered_map<std::string_view, RuleType> StringToRuleType = {{"Alias"sv, RuleType::ALIAS},
																																									{"Armor"sv, RuleType::ARMOR},
																																									{"Ammo"sv, RuleType::AMMO},
																																									{"Weapon"sv, RuleType::WEAPON}};

	// po3 tweaks
	inline HMODULE tweaks;
	using _GetFormEditorID = const char* (*)(std::uint32_t);

	std::string GetEditorID(const RE::FormID a_formID);
	std::string GetEditorID(const RE::TESForm* a_form);
}