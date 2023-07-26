#include "MergeMapperPluginAPI.h"
#include "Runner.h"

namespace LIEM {
	void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
		switch (a_msg->type) {
			case SKSE::MessagingInterface::kPostLoad:
				tweaks = GetModuleHandle(L"po3_Tweaks");
				INFO("po3_tweaks detected : {}", tweaks != nullptr);
				break;

			case SKSE::MessagingInterface::kPostPostLoad:
				MergeMapperPluginAPI::GetMergeMapperInterface001();
				if (g_mergeMapperInterface) {
					INFO("MergeMapper detected : {}", g_mergeMapperInterface->GetBuildNumber());
				} else
					INFO("MergeMapper detected : false");
				break;

			case SKSE::MessagingInterface::kDataLoaded:
				std::vector<std::string> fileList = Runner::GetFiles();
				INFO("Loaded {} files", fileList.size());

				Runner::Data map = Runner::ProcessFiles(fileList);
				INFO("Processed [{}] appliers+aliases, [{}] armors, [{}] weapons, [{}] ammos", map.appliers.size(),
						 map.armors.size(), map.weapons.size(), map.ammos.size());

				RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton();

				if (!map.armors.empty()) {
					const auto startTime = std::chrono::steady_clock::now();
					std::size_t total = 0;

					auto& armors = handler->GetFormArray<RE::TESObjectARMO>();
					for (auto const& armor : armors) {
						std::vector<std::size_t> appliedMods;
						TRACE("Processing armor: {:X} {} ({})", armor->GetFormID(), GetEditorID(armor), armor->GetName());

						for (auto& hash : map.order[LIEM::RuleType::ARMOR]) {
							auto& [mod, applierRef] = map.armors.at(hash);
							auto& applier = applierRef.get();

							auto applierPass = applier.Passes(armor);
							TRACE("Processed mod: {} = {}", hash, applierPass);
							if (applierPass) {
								mod.Apply(armor);
								appliedMods.emplace_back(hash);
							}
						}

						if (!appliedMods.empty()) {
							DEBUG("Processed armor mods [{}]: {:X} {} ({})", appliedMods.size(), armor->GetFormID(),
										GetEditorID(armor), armor->GetName());
							total += appliedMods.size();
							for (auto const& mod : appliedMods) {
								DEBUG("Applied mod: {}", mod);
							}
						}
					}
					const auto endTime = std::chrono::steady_clock::now();
					const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
					INFO("Processed armors: applied [{}] in {}ms", total, elapsedTime.count());
				}

				if (!map.ammos.empty()) {
					const auto startTime = std::chrono::steady_clock::now();
					std::size_t total = 0;

					auto& ammos = handler->GetFormArray<RE::TESAmmo>();
					for (auto const& ammo : ammos) {
						std::vector<std::size_t> appliedMods;
						TRACE("Processing ammo: {:X} {} ({})", ammo->GetFormID(), GetEditorID(ammo), ammo->GetName());

						for (auto& hash : map.order[LIEM::RuleType::AMMO]) {
							auto& [mod, applierRef] = map.ammos.at(hash);
							auto& applier = applierRef.get();

							auto applierPass = applier.Passes(ammo);
							TRACE("Processed mod: {} = {}", hash, applierPass);
							if (applierPass) {
								mod.Apply(ammo);
								appliedMods.emplace_back(hash);
							}
						}

						if (!appliedMods.empty()) {
							DEBUG("Processed ammo mods [{}]: {:X} {} ({})", appliedMods.size(), ammo->GetFormID(), GetEditorID(ammo),
										ammo->GetName());
							total += appliedMods.size();
							for (auto const& mod : appliedMods) {
								DEBUG("Applied mod: {}", mod);
							}
						}
					}
					const auto endTime = std::chrono::steady_clock::now();
					const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
					INFO("Processed ammos: applied [{}] in {}ms", total, elapsedTime.count());
				}

				if (!map.weapons.empty()) {
					const auto startTime = std::chrono::steady_clock::now();
					std::size_t total = 0;

					auto& weapons = handler->GetFormArray<RE::TESObjectWEAP>();
					for (auto const& weapon : weapons) {
						std::vector<std::size_t> appliedMods;
						TRACE("Processing weapon: {:X} {} ({})", weapon->GetFormID(), GetEditorID(weapon), weapon->GetName());

						for (auto& hash : map.order[LIEM::RuleType::WEAPON]) {
							auto& [mod, applierRef] = map.weapons.at(hash);
							auto& applier = applierRef.get();

							auto applierPass = applier.Passes(weapon);
							TRACE("Processed mod: {} = {}", hash, applierPass);
							if (applierPass) {
								mod.Apply(weapon);
								appliedMods.emplace_back(hash);
							}
						}

						if (!appliedMods.empty()) {
							DEBUG("Processed weapon mods [{}]: {:X} {} ({})", appliedMods.size(), weapon->GetFormID(),
										GetEditorID(weapon), weapon->GetName());
							total += appliedMods.size();
							for (auto const& mod : appliedMods) {
								DEBUG("Applied mod: {}", mod);
							}
						}
					}
					const auto endTime = std::chrono::steady_clock::now();
					const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
					INFO("Processed weapons: applied [{}] in {}ms", total, elapsedTime.count());
				}
				break;
		}
	}

	DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse) {
		DKUtil::Logger::Init(Plugin::NAME, REL::Module::get().version().string());

		// REL::Module::reset();
		SKSE::Init(a_skse);

		INFO("{} v{} loaded", Plugin::NAME, Plugin::Version);

		// DKUtil::Logger::SetLevel(spdlog::level::debug);

		auto* messaging = SKSE::GetMessagingInterface();
		if (!messaging->RegisterListener(MessageHandler)) {
			return false;
		}

		return true;
	}
}