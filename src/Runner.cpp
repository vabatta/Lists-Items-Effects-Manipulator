#include "Runner.h"

#include <SimpleIni.h>

namespace Runner {
	static std::string Sanitize(const std::string& raw) {
		auto newValue = raw;

		// formID hypen
		/*if (!newValue.contains('~')) {
			string::replace_first_instance(newValue, " - ", "~");
		}*/

		// strip spaces between " | "
		static const srell::regex re_bar(R"(\s*\|\s*)", srell::regex_constants::optimize);
		newValue = srell::regex_replace(newValue, re_bar, "|");

		// strip spaces between " , "
		static const srell::regex re_comma(R"(\s*,\s*)", srell::regex_constants::optimize);
		newValue = srell::regex_replace(newValue, re_comma, ",");

		// convert 00012345 formIDs to 0x12345
		static const srell::regex re_formID(R"(\b00+([0-9a-fA-F]{1,6})\b)", srell::regex_constants::optimize);
		newValue = srell::regex_replace(newValue, re_formID, "0x$1");

		// strip leading zeros
		static const srell::regex re_zeros(R"((0x00+)([0-9a-fA-F]+))", srell::regex_constants::optimize);
		newValue = srell::regex_replace(newValue, re_zeros, "0x$2");

		// NOT to hyphen
		// string::replace_all(newValue, "NOT ", "-");

		return newValue;
	}

	static inline std::size_t Hashify(const std::string& raw) {
		std::string toHash = raw;
		std::ranges::transform(toHash, toHash.begin(), [](char c) { return static_cast<char>(std::tolower(c)); });
		std::size_t hashed = std::hash<std::string>{}(toHash);

		return hashed;
	}

	std::vector<std::string> GetFiles() {
		std::vector<std::string> fileList;
		const std::string dataPath = "Data/";

		std::filesystem::path dataDir(dataPath);

		for (const auto& entry : std::filesystem::directory_iterator(dataDir)) {
			if (entry.path().extension() == ".ini" && entry.path().stem().string().ends_with(suffix)) {
				fileList.emplace_back(entry.path().string());
			}
		}

		return fileList;
	}

	std::size_t MapAppliers(const std::vector<srell::ssub_match>& matches, Data& result) {
		std::string raw = std::accumulate(matches.begin() + 1, matches.end(), std::string(),
																			[](const std::string& acc, const srell::ssub_match& match) {
																				return acc.empty() ? match.str() : acc + "|" + match.str();
																			});
		std::size_t appliersHash = Hashify(raw);

		if (result.appliers.contains(appliersHash)) {
			DEBUG("Reusing existing appliers: {}", appliersHash);
		} else {
			Appliers appliers;
			StringFilters::Aliases aliases;

			if (static_cast<uint32_t>(LIEM::SectionType::STRING_FILTERS) < matches.size()) {
				auto pair = StringFilters::Factory::ParseStringFilters(matches[1].str());
				appliers.stringFilters = pair.first;
				aliases = pair.second;
				TRACE("Created SF for: {}", matches[1].str());
			}
			if (static_cast<uint32_t>(LIEM::SectionType::FORM_FILTERS) < matches.size()) {
				appliers.formFilters = FormFilters::Factory::ParseFormFilters(matches[2].str());
				TRACE("Created FF for: {}", matches[2].str());
			}
			// TODO other appliers here

			result.appliers.emplace(appliersHash, appliers);
			DEBUG("Created appliers: {} -> {}", appliersHash, raw);

			for (auto& alias : aliases) {
				std::size_t mergeAppliersHash = Hashify(alias);

				if (!result.appliers.contains(mergeAppliersHash)) {
					WARN("Alias appliers not found: {}", alias);
					continue;
				}

				result.appliers[appliersHash].stringFilters += result.appliers[mergeAppliersHash].stringFilters;
				result.appliers[appliersHash].formFilters += result.appliers[mergeAppliersHash].formFilters;
				// TODO other appliers here

				DEBUG("Merged appliers: {} -> {}", mergeAppliersHash, appliersHash);
			}
		}

		return appliersHash;
	};

	template <LIEM::RuleType R>
	std::size_t MapModifier(const std::vector<srell::ssub_match>& matches, const std::size_t& appliersHash,
													Data& result) {
		std::size_t modifiersHash = Hashify(matches[0].str());

		auto modifier = Modifier::Factory::ParseData<R>(matches[0].str());
		if (modifier.has_value()) {
			auto pair = std::make_pair(modifier.value(), std::ref(result.appliers[appliersHash]));
			result.GetRules<R>().emplace(modifiersHash, pair);
			result.order[R].emplace_back(modifiersHash);
			DEBUG("Created modifier: {} -> {}", modifiersHash, matches[0].str());
		} else {
			WARN("Invalid modifier discarded: {}", matches[0].str());
		}

		return modifiersHash;
	};

	Data ProcessFiles(const std::vector<std::string>& fileList) {
		Data result;

		for (const auto& filePath : fileList) {
			CSimpleIniA ini;
			ini.SetUnicode();
			ini.SetMultiKey();

			TRACE("Processing file: {}", filePath);

			auto rc = ini.LoadFile(filePath.c_str());
			if (rc < 0) {
				ERROR("Error loading file: {}", filePath);
				continue;
			}

			auto values = ini.GetSection("");

			if (values->empty()) {
				WARN("File is empty: {}", filePath);
				continue;
			}

			// use to resave cleaned up entries
			std::multimap<CSimpleIniA::Entry, std::pair<std::string, std::string>, CSimpleIniA::Entry::LoadOrder>
					oldFormatMap;

			for (const auto& [key, entry] : *values) {
				DEBUG("Processing entry: {} = {}", key.pItem, entry);

				auto sanitized = Sanitize(entry);

				if (sanitized.empty()) {
					WARN("Entry is empty: {} = {}", key.pItem, entry);
					continue;
				}

				if (!sanitized.empty() && sanitized != entry) {
					oldFormatMap.emplace(key, std::make_pair(entry, sanitized));
				}

				// TODO lookup for the full string is worth?
				// std::string fullToHash = sanitized;
				// std::ranges::transform(fullToHash, fullToHash.begin(), [](char c) { return
				// static_cast<char>(std::tolower(c)); }); std::size_t fullHash = std::hash<std::string>{}(fullToHash);

				// check if it's a valid rule
				std::optional<LIEM::RuleType> ruleType = std::nullopt;
				auto it = LIEM::StringToRuleType.find(key.pItem);
				if (it != LIEM::StringToRuleType.end()) {
					ruleType = it->second;
				}
				if (!ruleType.has_value()) {
					WARN("Unknown rule: {}", key.pItem);
					continue;
				}

				// split our entry: RuleType = Modifiers|StringFilters
				std::vector<srell::ssub_match> matches;
				re_Sections.split(matches, sanitized);

				// create appliers
				std::size_t appliersHash = MapAppliers(matches, result);

				// we know it's a valid rule, so we can safely use value()
				if (ruleType.value() == LIEM::RuleType::ALIAS) {
					// TODO add / merge the path to the alias name to prevent bleed between mods (strips data/)
					std::size_t aliasNameHash = Hashify(matches[0].str());

					if (!result.appliers.contains(aliasNameHash)) {
						result.appliers.emplace(aliasNameHash, result.appliers[appliersHash]);
						DEBUG("Created alias: {} -> {}", aliasNameHash, appliersHash);
					}
					// if alias already exists, check if points to the same appliers
					else {
						WARN("Alias duplicate discarded: {}", matches[0].str());
						continue;
					}
				} else if (ruleType.value() == LIEM::RuleType::ARMOR) {
					MapModifier<LIEM::RuleType::ARMOR>(matches, appliersHash, result);
				} else if (ruleType.value() == LIEM::RuleType::AMMO) {
					MapModifier<LIEM::RuleType::AMMO>(matches, appliersHash, result);
				} else if (ruleType.value() == LIEM::RuleType::WEAPON) {
					MapModifier<LIEM::RuleType::WEAPON>(matches, appliersHash, result);
				}
				// TODO add more rules here
				else {
					WARN("Unimplemented key: {}", key.pItem);
					continue;
				}
			}

			// resave cleaned up entries
			if (!oldFormatMap.empty()) {
				INFO("Sanitizing {} entries", oldFormatMap.size());

				for (const auto& [key, entry] : oldFormatMap) {
					auto& [original, sanitized] = entry;
					ini.DeleteValue("", key.pItem, original.c_str());
					ini.SetValue("", key.pItem, sanitized.c_str(), key.pComment, false);
				}

				(void)ini.SaveFile(filePath.c_str());
			}

			ini.Reset();
		}

		return result;
	}

	template <typename T>
	bool Appliers::Passes(const T* form) const {
		return this->stringFilters.Passes(form) && this->formFilters.Passes(form);
	};

	// forces compilation
	template bool Appliers::Passes<RE::TESObjectARMO>(const RE::TESObjectARMO* form) const;
	template bool Appliers::Passes<RE::TESObjectWEAP>(const RE::TESObjectWEAP* form) const;
	template bool Appliers::Passes<RE::TESAmmo>(const RE::TESAmmo* form) const;
}
