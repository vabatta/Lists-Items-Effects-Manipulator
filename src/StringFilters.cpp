#include "StringFilters.h"

namespace StringFilters {
	std::pair<Data, std::unordered_set<std::string>> Factory::ParseStringFilters(const std::string& raw) {
		Data result = {};
		std::unordered_set<std::string> aliases = {};

		if (raw == rulesSkipToken || raw.empty()) {
			return std::make_pair(result, aliases);
		}

		std::vector<srell::ssub_match> rules;
		re_RulesSplitter.split(rules, raw);

		for (auto& rule : rules) {
			// requirement
			if (rule.str().contains("+"sv)) {
				std::string localCopy(rule.str());
				std::vector<srell::ssub_match> strings;
				re_RequirementsSplitter.split(strings, localCopy);
				result.requirements.insert(strings.begin(), strings.end());
			}
			// exclusion
			else if (rule.str().at(0) == '-') {
				// TODO maybe split on - here too?
				result.exclusions.emplace(rule.str().substr(1));
			}
			// wildcard
			else if (rule.str().at(0) == '*') {
				result.wildcards.emplace(rule.str().substr(1));
			}
			// alias
 			else if (rule.str().at(0) == '@') {
 				aliases.emplace(rule.str().substr(1));
 			}
			// match
			else {
				result.matches.emplace(rule.str());
			}
		}

		return std::make_pair(result, aliases);
	}

	template <typename T>
	bool Data::Passes(const T* form) const {
		auto to_lower{std::ranges::views::transform(static_cast<int (*)(int)>(std::tolower))};
		// by default we assume the form passes
		bool requirementsMet = true;
		bool exclusionsMet = true;
		bool matchesMet = true;
		bool wildcardsMet = true;

		/*if constexpr (std::derived_from<T, RE::TESForm>) {
			auto tesForm = static_cast<const RE::TESForm*>(form);
			DEBUG("{} = {} : {}", tesForm->GetObjectTypeName(), tesForm->GetFormEditorID(), tesForm->GetName())
		}*/

		if (!requirements.empty()) {
			requirementsMet = std::all_of(requirements.begin(), requirements.end(), [&](const auto& requirement) {
				bool result = false;
				TRACE("SF requirement: {}", requirement);

				if constexpr (std::derived_from<T, RE::BGSKeywordForm>) {
					// check keywords
					auto keywordForm = static_cast<const RE::BGSKeywordForm*>(form);
					result |= keywordForm->HasKeywordString(requirement);
					TRACE("SF hasKeyword = {}", result);
				}

				if constexpr (std::derived_from<T, RE::TESForm>) {
					auto tesForm = static_cast<const RE::TESForm*>(form);
					// check editorID
					std::string editorID(LIEM::GetEditorID(tesForm)); //(tesForm->GetFormEditorID());
					result |= std::ranges::equal(editorID | to_lower, requirement | to_lower);
					TRACE("SF isFormEditorID = {}", result);
					// check name
					std::string name(tesForm->GetName());
					result |= std::ranges::equal(name | to_lower, requirement | to_lower);
					TRACE("SF isName = {}", result);
				}

				return result;
			});
		}

		if (!exclusions.empty()) {
			exclusionsMet = std::none_of(exclusions.begin(), exclusions.end(), [&](const auto& exclusion) {
				bool result = false;
				TRACE("SF exclusion: {}", exclusion);

				if constexpr (std::derived_from<T, RE::BGSKeywordForm>) {
					// check keywords
					auto keywordForm = static_cast<const RE::BGSKeywordForm*>(form);
					result |= keywordForm->HasKeywordString(exclusion);
					TRACE("SF hasKeyword = {}", result);
				}

				if constexpr (std::derived_from<T, RE::TESForm>) {
					auto tesForm = static_cast<const RE::TESForm*>(form);
					// check editorID
					std::string editorID(LIEM::GetEditorID(tesForm)); //(tesForm->GetFormEditorID());
					result |= std::ranges::equal(editorID | to_lower, exclusion | to_lower);
					TRACE("SF isFormEditorID = {}", result);
					// check name
					std::string name(tesForm->GetName());
					result |= std::ranges::equal(name | to_lower, exclusion | to_lower);
					TRACE("SF isName = {}", result);
				}

				return result;
			});
		}

		if (!matches.empty()) {
			matchesMet = std::any_of(matches.begin(), matches.end(), [&](const auto& match) {
				bool result = false;
				TRACE("SF match: {}", match);

				if constexpr (std::derived_from<T, RE::BGSKeywordForm>) {
					// check keywords
					auto keywordForm = static_cast<const RE::BGSKeywordForm*>(form);
					result |= keywordForm->HasKeywordString(match);
					TRACE("SF hasKeyword = {}", result);
				}

				if constexpr (std::derived_from<T, RE::TESForm>) {
					auto tesForm = static_cast<const RE::TESForm*>(form);
					// check editorID
					std::string editorID(LIEM::GetEditorID(tesForm)); //(tesForm->GetFormEditorID());
					result |= std::ranges::equal(editorID | to_lower, match | to_lower);
					TRACE("SF isFormEditorID = {}", result);
					// check name
					std::string name(tesForm->GetName());
					result |= std::ranges::equal(name | to_lower, match | to_lower);
					TRACE("SF isName = {}", result);
				}

				return result;
			});
		}

		if (!wildcards.empty()) {
			wildcardsMet = std::any_of(wildcards.begin(), wildcards.end(), [&](const auto& wildcard) {
				bool result = false;
				TRACE("SF wildcard: {}", wildcard);

				if constexpr (std::derived_from<T, RE::BGSKeywordForm>) {
					// check keywords
					auto keywordForm = static_cast<const RE::BGSKeywordForm*>(form);
					result |= keywordForm->ContainsKeywordString(wildcard);
					TRACE("SF containsKeyword = {}", result);
				}

				if constexpr (std::derived_from<T, RE::TESForm>) {
					auto tesForm = static_cast<const RE::TESForm*>(form);
					// check editorID
					std::string editorID(LIEM::GetEditorID(tesForm)); //(tesForm->GetFormEditorID());
					result |= std::search(editorID.begin(), editorID.end(), wildcard.begin(), wildcard.end(),
													[](char c1, char c2) { return std::tolower(c1) == std::tolower(c2); }) != editorID.end();
					TRACE("SF containsFormEditorID = {}", result);
					// check name
					std::string name(tesForm->GetName());
					result |= std::search(name.begin(), name.end(), wildcard.begin(), wildcard.end(),
																[](char c1, char c2) { return std::tolower(c1) == std::tolower(c2); }) != name.end();
					TRACE("SF containsName = {}", result);
				}

				return result;
			});
		}

		return requirementsMet && exclusionsMet && matchesMet && wildcardsMet;
	}

	// forces compilation
	template bool Data::Passes<RE::TESObjectARMO>(const RE::TESObjectARMO* form) const;
	template bool Data::Passes<RE::TESObjectWEAP>(const RE::TESObjectWEAP* form) const;
	template bool Data::Passes<RE::TESAmmo>(const RE::TESAmmo* form) const;
}
