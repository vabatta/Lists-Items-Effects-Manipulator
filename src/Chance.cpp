#include "Chance.h"

#include "Defs.h"

namespace Chance {
	Data Factory::ParseChance(const std::string& raw) {
		Data result;

		if (raw == rulesSkipToken || raw.empty()) {
			return result;
		}

		try {
			int32_t parsed = std::stoi(raw);
			if (parsed >= 0 && parsed < 100) {
				result.chance = std::make_optional(static_cast<uint32_t>(parsed));
			} else {
				throw std::exception("Chance out of range");
			}
		} catch (const std::exception&) {
			WARN("Chance skipped as invalid: {}", raw);
		}

		return result;
	}

	template <typename T>
	bool Data::Passes(const T* form) const {
		uint32_t chanceValue = chance.value_or(100);

		switch (chanceValue) {
			case 100:
				return true;
			case 0:
				return false;
			default:
				if constexpr (std::derived_from<T, RE::TESForm>) {
					auto tesForm = static_cast<const RE::TESForm*>(form);
					// deterministic random based on formID + editorID
					std::string hashString = std::to_string(tesForm->GetFormID()) + LIEM::GetEditorID(tesForm);
					std::size_t seed = std::hash<std::string>{}(hashString);
					std::seed_seq seedSequence{seed};
					std::mt19937_64 rng(seedSequence);
					uint32_t random = rng() % 100;
					bool passes = random <= chanceValue;
					TRACE("CH passed: {} ({} <= {})", passes, random, chanceValue);
					return passes;
				} else {
					static_assert(!std::derived_from<T, RE::TESForm>, "Chance needs a TESForm derived argument");
				}
		}
	}

	// forces compilation
	template bool Data::Passes<RE::TESObjectARMO>(const RE::TESObjectARMO* form) const;
	template bool Data::Passes<RE::TESObjectWEAP>(const RE::TESObjectWEAP* form) const;
	template bool Data::Passes<RE::TESAmmo>(const RE::TESAmmo* form) const;
}

