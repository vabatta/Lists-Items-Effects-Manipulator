#include "Runner.h"

namespace LIEM {
	void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
		switch (a_msg->type) {
			case SKSE::MessagingInterface::kPostLoad:
				tweaks = GetModuleHandle(L"po3_Tweaks");
				INFO("powerofthree's Tweaks (po3_tweaks) detected : {}", tweaks != nullptr);
				break;

			case SKSE::MessagingInterface::kDataLoaded:
				std::vector<std::string> fileList = Runner::GetFiles();
				INFO("Loaded {} files", fileList.size());

				Runner::Data map = Runner::ProcessFiles(fileList);
				INFO("Processed [{}] appliers+aliases, [{}] armors, [{}] weapons, [{}] ammos", map.appliers.size(),
						 map.armors.size(), map.weapons.size(), map.ammos.size());

				RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton();

				if (!map.armors.empty()) {
					auto& armors = handler->GetFormArray<RE::TESObjectARMO>();
					for (auto const& armor : armors) {
						std::vector<std::size_t> appliedMods;
						TRACE("Processing armor: {:X} {} ({})", armor->GetFormID(), GetEditorID(armor), armor->GetName());

						for (auto& [hash, rule] : map.armors) {
							auto& mod = rule.first;
							auto& applier = rule.second.get();

							auto applierPass = applier.Passes(armor);
							TRACE("Processed applier: {} = {}", hash, applierPass);
							if (applierPass) {
								mod.Apply(armor);
								appliedMods.emplace_back(hash);
							}
						}

						if (!appliedMods.empty()) {
							DEBUG("Processed armor mods [{}]: {:X} {} ({})", appliedMods.size(), armor->GetFormID(), GetEditorID(armor),
										armor->GetName());
							for (auto& hash : appliedMods) {
								TRACE("Applied mod: {}", hash);
							}
						}
					}
				}

				if (!map.ammos.empty()) {
					auto& ammos = handler->GetFormArray<RE::TESAmmo>();
					for (auto const& ammo : ammos) {
						std::vector<std::size_t> appliedMods;
						TRACE("Processing ammo: {:X} {} ({})", ammo->GetFormID(), GetEditorID(ammo), ammo->GetName());

						for (auto& [hash, rule] : map.ammos) {
							auto& mod = rule.first;
							auto& applier = rule.second.get();

							auto applierPass = applier.Passes(ammo);
							TRACE("Processed applier: {} = {}", hash, applierPass);
							if (applierPass) {
								mod.Apply(ammo);
								appliedMods.emplace_back(hash);
							}
						}

						if (!appliedMods.empty()) {
							DEBUG("Processed ammo mods [{}]: {:X} {} ({})", appliedMods.size(), ammo->GetFormID(), GetEditorID(ammo),
										ammo->GetName());
							for (auto& hash : appliedMods) {
								TRACE("Applied mod: {}", hash);
							}
						}
					}
				}

				if (!map.weapons.empty()) {
					auto& weapons = handler->GetFormArray<RE::TESObjectWEAP>();
					for (auto const& weapon : weapons) {
						std::vector<std::size_t> appliedMods;
						TRACE("Processing weapon: {:X} {} ({})", weapon->GetFormID(), GetEditorID(weapon), weapon->GetName());

						for (auto& [hash, rule] : map.weapons) {
							auto& mod = rule.first;
							auto& applier = rule.second.get();

							auto applierPass = applier.Passes(weapon);
							TRACE("Processed applier: {} = {}", hash, applierPass);
							if (applierPass) {
								mod.Apply(weapon);
								appliedMods.emplace_back(hash);
							}
						}

						if (!appliedMods.empty()) {
							DEBUG("Processed weapon mods [{}]: {:X} {} ({})", appliedMods.size(), weapon->GetFormID(), GetEditorID(weapon),
										weapon->GetName());
							for (auto& hash : appliedMods) {
								TRACE("Applied mod: {}", hash);
							}
						}
					}
				}
				break;
		}
	}

	DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse) {
		DKUtil::Logger::Init(Plugin::NAME, REL::Module::get().version().string());

		// REL::Module::reset();
		SKSE::Init(a_skse);

		INFO("{} v{} loaded", Plugin::NAME, Plugin::Version);

		auto* messaging = SKSE::GetMessagingInterface();
		if (!messaging->RegisterListener(MessageHandler)) {
			return false;
		}

		return true;
	}
}