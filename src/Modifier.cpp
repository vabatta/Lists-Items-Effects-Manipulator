#include "Modifier.h"

namespace Modifier {
	template <typename M, typename V>
	static inline void ApplyModifier(const M& modifier, V& value) {
		if (modifier.has_value()) {
			switch (modifier.value().first) {
				case Type::ABS:
					value = modifier.value().second;
					break;

				case Type::ADD:
					value += modifier.value().second;
					break;

				case Type::SUB:
					value -= modifier.value().second;
					break;
			}
		}
	}

	template <typename T>
	static std::optional<modnum_t<T>> Factory::ParseModifierNumber(const std::string& raw) {
		if (raw == rulesSkipToken || raw.empty()) {
			return std::nullopt;
		}

		srell::smatch matches;
		re_ModifierNumber.match(raw, matches);

		if (matches.size() >= 3) {
			std::string sign = matches[1].str();
			std::string value = matches[2].str();

			Type signType;
			if (sign.empty()) {
				signType = Type::ABS;
			} else if (sign == "+") {
				signType = Type::ADD;
			} else if (sign == "-") {
				signType = Type::SUB;
			}

			try {
				if constexpr (std::is_floating_point_v<T>) {
					return std::make_pair(signType, static_cast<T>(std::stof(value)));
				} else if constexpr (std::is_integral_v<T>) {
					return std::make_pair(signType, static_cast<T>(std::stoi(value)));
				}
			} catch (const std::exception&) {
				ERROR("Failed to parse modifier number: {}", raw);
			}
		}

		return std::nullopt;
	}

	template <>
	std::optional<Data<LIEM::RuleType::ARMOR>> Factory::ParseData<LIEM::RuleType::ARMOR>(const std::string& raw) {
		Data<LIEM::RuleType::ARMOR> result = {};

		// value,weight,rating
		std::vector<srell::ssub_match> rules;
		re_RulesSplitter.split(rules, raw);

		switch (rules.size()) {
			case 3: {
				result.rating = Factory::ParseModifierNumber<float>(rules[2].str());

				[[fallthrough]];
			}
			case 2: {
				result.weight = Factory::ParseModifierNumber<float>(rules[1].str());

				[[fallthrough]];
			}
			case 1: {
				result.value = Factory::ParseModifierNumber<int32_t>(rules[0].str());

				// at least one of the values must be present
				if (result.rating.has_value() || result.weight.has_value() || result.value.has_value()) {
					return result;
				}

				[[fallthrough]];
			}

			default:
				return std::nullopt;
		}
	}

	template <>
	std::optional<Data<LIEM::RuleType::AMMO>> Factory::ParseData<LIEM::RuleType::AMMO>(const std::string& raw) {
		Data<LIEM::RuleType::AMMO> result = {};

		// value,weight,damage
		std::vector<srell::ssub_match> rules;
		re_RulesSplitter.split(rules, raw);

		switch (rules.size()) {
		case 3: {
				result.damage = Factory::ParseModifierNumber<float>(rules[2].str());

				[[fallthrough]];
			}
		case 2: {
				result.weight = Factory::ParseModifierNumber<float>(rules[1].str());

				[[fallthrough]];
			}
		case 1: {
				result.value = Factory::ParseModifierNumber<int32_t>(rules[0].str());

				// at least one of the values must be present
				if (result.damage.has_value() || result.weight.has_value() || result.value.has_value()) {
					return result;
				}

				[[fallthrough]];
			}

			default:
				return std::nullopt;
		}
	}

	template <>
	std::optional<Data<LIEM::RuleType::WEAPON>> Factory::ParseData<LIEM::RuleType::WEAPON>(const std::string& raw) {
		Data<LIEM::RuleType::WEAPON> result = {};

		// value,weight,damage
		std::vector<srell::ssub_match> rules;
		re_RulesSplitter.split(rules, raw);

		switch (rules.size()) {
			case 7: {
				result.stagger = Factory::ParseModifierNumber<float>(rules[6].str());

				[[fallthrough]];
			}
			case 6: {
				result.reach = Factory::ParseModifierNumber<float>(rules[5].str());

				[[fallthrough]];
			}
			case 5: {
				result.speed = Factory::ParseModifierNumber<float>(rules[4].str());

				[[fallthrough]];
			}
			case 4: {
				result.critDamage = Factory::ParseModifierNumber<uint16_t>(rules[3].str());

				[[fallthrough]];
			}
			case 3: {
				result.damage = Factory::ParseModifierNumber<uint16_t>(rules[2].str());

				[[fallthrough]];
			}
			case 2: {
				result.weight = Factory::ParseModifierNumber<float>(rules[1].str());

				[[fallthrough]];
			}
			case 1: {
				result.value = Factory::ParseModifierNumber<int32_t>(rules[0].str());

				// at least one of the values must be present
				if (result.stagger.has_value() || result.reach.has_value() || result.speed.has_value() || result.critDamage.has_value() || result.damage.has_value() || result.weight.has_value() || result.value.has_value()) {
					return result;
				}

				[[fallthrough]];
			}

			default:
				return std::nullopt;
		}
	}

	void Data<LIEM::RuleType::ARMOR>::Apply(RE::TESObjectARMO* armor) const {
		ApplyModifier(this->value, armor->value);
		ApplyModifier(this->weight, armor->weight);
		// aromr rating is parsed as a float but the game uses a uint32_t
		if (this->rating.has_value()) {
			auto ratingInt = std::make_pair(this->rating.value().first, static_cast<uint32_t>(this->rating.value().second * 100u));
			ApplyModifier(std::make_optional(ratingInt), armor->armorRating);
		}
	}

	void Data<LIEM::RuleType::ARMOR>::Apply(std::vector<RE::TESObjectARMO*>& armors) const {
		for (auto& armor : armors) {
			this->Apply(armor);
		}
	}

	void Data<LIEM::RuleType::AMMO>::Apply(RE::TESAmmo* ammo) const {
		ApplyModifier(this->value, ammo->value);
		ApplyModifier(this->weight, ammo->weight);
		ApplyModifier(this->damage, ammo->data.damage);
	}

	void Data<LIEM::RuleType::AMMO>::Apply(std::vector<RE::TESAmmo*>& ammos) const {
		for (auto& ammo : ammos) {
			this->Apply(ammo);
		}
	}

	void Data<LIEM::RuleType::WEAPON>::Apply(RE::TESObjectWEAP* weapon) const {
		ApplyModifier(this->value, weapon->value);
		ApplyModifier(this->weight, weapon->weight);
		ApplyModifier(this->damage, weapon->attackDamage);
		ApplyModifier(this->critDamage, weapon->criticalData.damage);
		ApplyModifier(this->speed, weapon->weaponData.speed);
		ApplyModifier(this->reach, weapon->weaponData.reach);
		ApplyModifier(this->stagger, weapon->weaponData.staggerValue);
	}

	void Data<LIEM::RuleType::WEAPON>::Apply(std::vector<RE::TESObjectWEAP*>& weapons) const {
		for (auto& weapon : weapons) {
			this->Apply(weapon);
		}
	}
}