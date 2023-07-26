#include "FormFilters.h"
#include "Defs.h"

#include "MergeMapperPluginAPI.h"

namespace FormFilters {
	inline static Descriptor Factory::ParseDescriptor(const std::string& raw) {
		srell::smatch matches;
		re_FormFilter.match(raw, matches);

		auto& formId = matches[1];
		auto& editorOrMod = matches[2];
		// editorID
		if (!editorOrMod.str().empty() && formId.str().empty()) {
			return editorOrMod.str();
		}
		// 0x000001
		else if (!formId.str().empty() && editorOrMod.str().empty()) {
			return std::stoul(formId.str(), 0, 16);
		}
		// 0x000001~MyMod.esp
		else {
			return std::make_pair(std::stoul(formId.str(), 0, 16), editorOrMod.str());
		}
	}

	Data Factory::ParseFormFilters(const std::string& raw) {
		Data result = {};

		if (raw == rulesSkipToken || raw.empty()) {
			return result;
		}

		std::vector<srell::ssub_match> rules;
		re_RulesSplitter.split(rules, raw);

		for (auto& rule : rules) {
			// requirement
			if (rule.str().contains("+"sv)) {
				std::string localCopy(rule.str());
				std::vector<srell::ssub_match> requirements;
				re_RequirementsSplitter.split(requirements, localCopy);

				for (auto& requirement : requirements) {
					result.requirements.emplace(Factory::ParseDescriptor(requirement.str()));
				}
			}
			// exclusion
			else if (rule.str().at(0) == '-') {
				// TODO maybe split on - here too?
				result.exclusions.emplace(Factory::ParseDescriptor(rule.str()));
			}
			// match
			else {
				result.matches.emplace(Factory::ParseDescriptor(rule.str()));
			}
		}

		return result;
	}

	static const RE::TESForm* GetFormFromDescriptor(const Descriptor& desc) {
		if (std::holds_alternative<std::string>(desc)) {
			auto& editorID = std::get<std::string>(desc);
			return RE::TESForm::LookupByEditorID(editorID);
		} else if (std::holds_alternative<RE::FormID>(desc)) {
			auto& formID = std::get<RE::FormID>(desc);
			// REVIEW should we use the merge mapper here too?
			return RE::TESForm::LookupByID(formID);
		} else if (std::holds_alternative<std::pair<RE::FormID, std::string>>(desc)) {
			RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton();
			auto& [formID, modName] = std::get<std::pair<RE::FormID, std::string>>(desc);
			if (g_mergeMapperInterface) {
				const auto& [newModName, newFormID] = g_mergeMapperInterface->GetNewFormID(modName.c_str(), formID);
				return handler->LookupForm(newFormID, newModName);
			} else {
				return handler->LookupForm(formID, modName);
			}
		}

		return nullptr;
	}

	static const std::string DescriptorToString(const Descriptor& desc) {
		if (std::holds_alternative<std::string>(desc)) {
			return std::get<std::string>(desc);
		} else if (std::holds_alternative<RE::FormID>(desc)) {
			auto& formID = std::get<RE::FormID>(desc);
			return std::format("0x{:X}", formID);
		} else if (std::holds_alternative<std::pair<RE::FormID, std::string>>(desc)) {
			auto& [formID, modName] = std::get<std::pair<RE::FormID, std::string>>(desc);
			return std::format("0x{:X}~{}", formID, modName);
		}

		return "unknown descriptor";
	}

	template <typename T>
	bool Data::Passes(const T* form) const {
		auto to_lower{std::ranges::views::transform(static_cast<int (*)(int)>(std::tolower))};
		// by default we assume the form passes
		bool requirementsMet = true;
		bool exclusionsMet = true;
		bool matchesMet = true;

		std::function<bool(const RE::TESForm*, uint32_t guard)> checkFormInDescriptorRecursive = [&](const RE::TESForm* descForm, uint32_t guard) -> bool {
			if (descForm == nullptr) {
				WARN("FF form skipped as not found");
				return false;
			}

			// stop guard to prevent infinite recursion (should never happen)
			if (guard > 10) {
				WARN("FF form skipped as recursion depth exceeded");
				return false;
			}

			bool result = false;
			if (descForm->Is(RE::FormType::LeveledItem)) {
				auto lvli = static_cast<const RE::TESLevItem*>(descForm);
				result |= std::find_if(lvli->entries.begin(), lvli->entries.end(), [&](const RE::LEVELED_OBJECT& entry) {
										return checkFormInDescriptorRecursive(entry.form, guard + 1);
									}) != lvli->entries.end();
				TRACE("FF form is in leveled list: {}", result);
			} else if (descForm->Is(RE::FormType::Outfit)) {
				auto otft = static_cast<const RE::BGSOutfit*>(descForm);
				result |= std::find_if(otft->outfitItems.begin(), otft->outfitItems.end(), [&](const RE::TESForm* item) {
										return checkFormInDescriptorRecursive(item, guard + 1);
									}) != otft->outfitItems.end();
				TRACE("FF form is in outfit: {}", result);
			} else if (descForm->Is(RE::FormType::FormList)) {
				auto flst = static_cast<const RE::BGSListForm*>(descForm);
				result |= std::find_if(flst->forms.begin(), flst->forms.end(), [&](const RE::TESForm* formInList) {
										return checkFormInDescriptorRecursive(formInList, guard + 1);
									}) != flst->forms.end();
				TRACE("FF form is in form list: {}", result);
			} else {
				// just ignore if not any of the above and treat as form
				result |= descForm->formID == form->formID;
				//TRACE("FF form check: {}", result);
			}

			return result;
		};

		auto checkFormInDescriptor = [&](const Descriptor& desc) -> bool {
			bool result = false;
			TRACE("FF: {}", DescriptorToString(desc));

			const RE::TESForm* descForm = GetFormFromDescriptor(desc);
			result = checkFormInDescriptorRecursive(descForm, 0);
			TRACE("FF form check done: {} -> {}", LIEM::GetEditorID(descForm), result);
			return result;
		};

		if (!requirements.empty()) {
			requirementsMet = std::all_of(requirements.begin(), requirements.end(), checkFormInDescriptor);
		}

		if (!exclusions.empty()) {
			exclusionsMet = std::none_of(exclusions.begin(), exclusions.end(), checkFormInDescriptor);
		}

		if (!matches.empty()) {
			matchesMet = std::any_of(matches.begin(), matches.end(), checkFormInDescriptor);
		}

		return requirementsMet && exclusionsMet && matchesMet;
	}

	// forces compilation
	template bool Data::Passes<RE::TESObjectARMO>(const RE::TESObjectARMO* form) const;
	template bool Data::Passes<RE::TESObjectWEAP>(const RE::TESObjectWEAP* form) const;
	template bool Data::Passes<RE::TESAmmo>(const RE::TESAmmo* form) const;
}