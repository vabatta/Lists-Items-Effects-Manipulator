#pragma once

namespace FormFilters {
	static inline constexpr auto rulesSkipToken = "NONE"sv;
	static const srell::regex re_RulesSplitter(R"(,)", srell::regex_constants::optimize);
	static const srell::regex re_RequirementsSplitter(R"(\+)", srell::regex_constants::optimize);
	static const srell::regex re_FormFilter(R"((0x[0-9a-fA-F]+)?~?(.*))", srell::regex_constants::optimize);

	using Descriptor = std::variant<std::string,												// editorID
																	RE::FormID,													// formID
																	std::pair<RE::FormID, std::string>	// formID~modName
																	>;

	struct DescriptorHash {
		inline std::size_t operator()(const Descriptor& v) const {
			if (std::holds_alternative<std::string>(v)) {
				return std::hash<std::string>{}(std::get<std::string>(v));
			}
			else if (std::holds_alternative<RE::FormID>(v)) {
				return std::hash<RE::FormID>{}(std::get<RE::FormID>(v));
			}
			else {
				auto& a = std::get<std::pair<RE::FormID, std::string>>(v);
				return std::hash<RE::FormID>{}(a.first) + std::hash<std::string>{}(a.second);
			}
		}
	};

	struct Data {
		std::unordered_set<Descriptor, DescriptorHash> requirements;
		std::unordered_set<Descriptor, DescriptorHash> exclusions;
		std::unordered_set<Descriptor, DescriptorHash> matches;

		template <typename T>
		bool Passes(const T* form) const;

		Data operator+(const Data& other) const {
			Data result = *this;
			result += other;
			return result;
		}

		Data& operator+=(const Data& other) {
			requirements.insert(other.requirements.begin(), other.requirements.end());
			exclusions.insert(other.exclusions.begin(), other.exclusions.end());
			matches.insert(other.matches.begin(), other.matches.end());
			return *this;
		}
	};

	static const RE::TESForm* GetFormFromDescriptor(const Descriptor& desc);
	static const std::string DescriptorToString(const Descriptor& desc);

	namespace Factory{
		inline static Descriptor ParseDescriptor(const std::string& raw);

		Data ParseFormFilters(const std::string& raw);
	}
}