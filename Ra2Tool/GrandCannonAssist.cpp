#include <windows.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <BuildingClass.h>
#include <EventClass.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <MapClass.h>
#include <SessionClass.h>
#include <TargetClass.h>
#include "Utils.h"
#include "Config.h"
#include "Ra2Helper.h"

namespace {

std::unordered_set<DWORD> knownOwnGrandCannons;
std::unordered_set<DWORD> knownEnemyGrandCannons;
std::unordered_map<DWORD, DWORD> lastEngineerRepairTicks;
bool grandCannonKnownSetReady = false;
DWORD lastNoEngineerMessageTick = 0;

struct EngineerRepairTask {
	InfantryClass* Engineer = nullptr;
	BuildingClass* Target = nullptr;
	DWORD LastMissionTick = 0;
	DWORD LastActionTick = 0;
};

std::unordered_map<DWORD, EngineerRepairTask> engineerRepairTasks;

int AbsInt(int value) {
	return value < 0 ? -value : value;
}

DWORD GetBuildingKey(BuildingClass* building) {
	if (!building) {
		return 0;
	}

	return building->UniqueID ? building->UniqueID : reinterpret_cast<DWORD>(building);
}

bool IsLiveBuilding(BuildingClass* building) {
	return building
		&& building->Type
		&& building->Owner
		&& building->IsAlive
		&& building->IsOnMap
		&& !building->InLimbo
		&& building->ActuallyPlacedOnMap;
}

bool IsGrandCannon(BuildingClass* building) {
	__try {
		return IsLiveBuilding(building)
			&& building->Type->ArrayIndex == Config::getGrandCannonBuildingType();
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

bool IsGrandCannonReady(BuildingClass* building) {
	__try {
		return IsGrandCannon(building)
			&& building->BState != static_cast<int>(BStateType::Construction)
			&& building->Health > 0;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

bool IsEnemyHouse(HouseClass* house) {
	__try {
		auto current = HouseClass::CurrentPlayer;
		return current
			&& house
			&& house != current
			&& !house->Defeated
			&& !house->IsGameOver
			&& !house->IsNeutral()
			&& !current->IsAlliedWith(house);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

bool IsOwnGrandCannon(BuildingClass* building) {
	return IsGrandCannon(building) && building->Owner == HouseClass::CurrentPlayer;
}

bool IsEnemyGrandCannon(BuildingClass* building) {
	return IsGrandCannon(building) && IsEnemyHouse(building->Owner);
}

int DistanceBetween(AbstractClass* a, AbstractClass* b) {
	if (!a || !b) {
		return INT_MAX;
	}

	return a->DistanceFrom(b);
}

int GetBuildingArrayCount() {
	__try {
		return BuildingClass::Array.Count;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return 0;
	}
}

BuildingClass* GetBuildingArrayItem(int index) {
	__try {
		return BuildingClass::Array.GetItem(index);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return nullptr;
	}
}

int GetHouseBuildingCount(HouseClass* house) {
	__try {
		return house ? house->Buildings.Count : 0;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return 0;
	}
}

BuildingClass* GetHouseBuildingItem(HouseClass* house, int index) {
	__try {
		return house ? house->Buildings.GetItem(index) : nullptr;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return nullptr;
	}
}

bool IsInAttackRange(BuildingClass* source, BuildingClass* target) {
	if (!source || !target) {
		return false;
	}

	__try {
		if (source->IsCloseEnoughToAttack(target)) {
			return true;
		}

		const int range = source->GetWeaponRange(0);
		return range > 0 && DistanceBetween(source, target) <= range;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

std::vector<BuildingClass*> GetOwnGrandCannons() {
	std::vector<BuildingClass*> items;

	auto current = HouseClass::CurrentPlayer;
	if (!current) {
		return items;
	}

	const int count = GetHouseBuildingCount(current);
	for (int i = 0; i < count; ++i) {
		auto building = GetHouseBuildingItem(current, i);
		if (IsOwnGrandCannon(building)) {
			items.push_back(building);
		}
	}

	return items;
}

std::vector<BuildingClass*> GetEnemyGrandCannons() {
	std::vector<BuildingClass*> items;

	const int count = GetBuildingArrayCount();
	for (int i = 0; i < count; ++i) {
		auto building = GetBuildingArrayItem(i);
		if (IsEnemyGrandCannon(building)) {
			items.push_back(building);
		}
	}

	return items;
}

BuildingClass* FindNearestEnemyGrandCannonInRange(BuildingClass* source) {
	BuildingClass* best = nullptr;
	int bestDistance = INT_MAX;

	for (auto enemy : GetEnemyGrandCannons()) {
		if (!IsInAttackRange(source, enemy)) {
			continue;
		}

		const int distance = DistanceBetween(source, enemy);
		if (distance < bestDistance) {
			best = enemy;
			bestDistance = distance;
		}
	}

	return best;
}

int HealthPercent(BuildingClass* building) {
	__try {
		if (!building || !building->Type || building->Type->Strength <= 0) {
			return 100;
		}

		const int health = std::max(0, building->Health);
		return std::min(100, (health * 100) / building->Type->Strength);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return 100;
	}
}

bool IsOwnEngineer(InfantryClass* infantry) {
	__try {
		return infantry
			&& infantry->Owner == HouseClass::CurrentPlayer
			&& infantry->IsAlive
			&& infantry->IsOnMap
			&& !infantry->InLimbo
			&& infantry->Type
			&& (infantry->Type->Engineer || infantry->IsEngineer());
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

InfantryClass* FindNearestEngineer(BuildingClass* target) {
	InfantryClass* best = nullptr;
	int bestDistance = INT_MAX;

	__try {
		for (int i = 0; i < InfantryClass::Array.Count; ++i) {
			InfantryClass* infantry = InfantryClass::Array.GetItem(i);
			if (!IsOwnEngineer(infantry)) {
				continue;
			}

			const int distance = DistanceBetween(infantry, target);
			if (distance < bestDistance) {
				best = infantry;
				bestDistance = distance;
			}
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return nullptr;
	}

	return best;
}

void NotifyNoEngineer() {
	const DWORD now = GetTickCount();
	if (now - lastNoEngineerMessageTick < 5000) {
		return;
	}

	lastNoEngineerMessageTick = now;
	PrintGameMessage(L"当前没有工程师，无法执行巨炮抢修");
	Utils::Log("GrandCannonAssist engineer repair failed: no engineer.");
}

void IssueEngineerMission(InfantryClass* engineer, BuildingClass* target, Mission mission, const char* reason) {
	if (!engineer || !target || engineer->Owner != HouseClass::CurrentPlayer) {
		return;
	}

	__try {
		EventClass event(
			HouseClass::CurrentPlayer->ArrayIndex,
			TargetClass(engineer),
			mission,
			TargetClass(target),
			TargetClass(target),
			TargetClass());
		EventClass::OutList.Add(event);
		Utils::LogFormat(
			"GrandCannonAssist engineer mission: engineer=%08X target=%08X mission=%d reason=%s hp=%d%%",
			engineer->UniqueID,
			target->UniqueID,
			static_cast<int>(mission),
			reason ? reason : "",
			HealthPercent(target));
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Utils::Log("GrandCannonAssist engineer mission failed.");
	}
}

bool IssueEngineerRepairAction(InfantryClass* engineer, BuildingClass* target, const char* reason) {
	if (!engineer || !target || engineer->Owner != HouseClass::CurrentPlayer) {
		return false;
	}

	__try {
		Action action = engineer->MouseOverObject(target, false);
		if (action == Action::None || action == Action::NoRepair || action == Action::NoGRepair) {
			action = Action::GRepair;
		}

		bool result = engineer->ObjectClickedAction(action, target, false);
		if (!result && action != Action::GRepair) {
			result = engineer->ObjectClickedAction(Action::GRepair, target, false);
		}
		if (!result && action != Action::Repair) {
			result = engineer->ObjectClickedAction(Action::Repair, target, false);
		}
		if (!result && action != Action::Capture) {
			result = engineer->ObjectClickedAction(Action::Capture, target, false);
		}

		Utils::LogFormat(
			"GrandCannonAssist engineer action: engineer=%08X target=%08X action=%d reason=%s result=%d hp=%d%%",
			engineer->UniqueID,
			target->UniqueID,
			static_cast<int>(action),
			reason ? reason : "",
			result ? 1 : 0,
			HealthPercent(target));
		return result;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Utils::Log("GrandCannonAssist engineer action failed.");
		return false;
	}
}

bool IssueEngineerClickedMission(InfantryClass* engineer, BuildingClass* target, Mission mission, const char* reason) {
	if (!engineer || !target || engineer->Owner != HouseClass::CurrentPlayer) {
		return false;
	}

	__try {
		const CellStruct targetCell = target->GetMapCoords();
		CellClass* cell = MapClass::Instance.TryGetCellAt(targetCell);
		const bool result = engineer->ClickedMission(mission, target, cell, cell);
		Utils::LogFormat(
			"GrandCannonAssist engineer clicked mission: engineer=%08X target=%08X mission=%d reason=%s result=%d hp=%d%%",
			engineer->UniqueID,
			target->UniqueID,
			static_cast<int>(mission),
			reason ? reason : "",
			result ? 1 : 0,
			HealthPercent(target));
		return result;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Utils::Log("GrandCannonAssist engineer clicked mission failed.");
		return false;
	}
}

void IssueEngineerRepairCommand(InfantryClass* engineer, BuildingClass* target, const char* reason) {
	IssueEngineerRepairAction(engineer, target, reason);
	IssueEngineerClickedMission(engineer, target, Mission::Capture, reason);
	IssueEngineerMission(engineer, target, Mission::Capture, reason);
}

void IssueEngineerRepair(InfantryClass* engineer, BuildingClass* target) {
	IssueEngineerRepairCommand(engineer, target, "start");
	const DWORD key = GetBuildingKey(target);
	if (key) {
		EngineerRepairTask task;
		task.Engineer = engineer;
		task.Target = target;
		task.LastMissionTick = GetTickCount();
		task.LastActionTick = task.LastMissionTick;
		engineerRepairTasks[key] = task;
	}
}

bool IsEngineerNearTarget(InfantryClass* engineer, BuildingClass* target) {
	if (!engineer || !target) {
		return false;
	}

	__try {
		const CellStruct engineerCell = engineer->GetMapCoords();
		const CellStruct targetCell = target->GetMapCoords();
		if (AbsInt(static_cast<int>(engineerCell.X) - static_cast<int>(targetCell.X)) <= 3
			&& AbsInt(static_cast<int>(engineerCell.Y) - static_cast<int>(targetCell.Y)) <= 3) {
			return true;
		}

		return DistanceBetween(engineer, target) <= 1024;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

void TickEngineerRepairTasks() {
	const DWORD now = GetTickCount();
	std::vector<DWORD> finished;

	for (auto& item : engineerRepairTasks) {
		EngineerRepairTask& task = item.second;
		if (!IsOwnEngineer(task.Engineer) || !IsOwnGrandCannon(task.Target) || !task.Target->IsAlive) {
			finished.push_back(item.first);
			continue;
		}

		if (task.Target->Health >= task.Target->Type->Strength) {
			finished.push_back(item.first);
			continue;
		}

		if (IsEngineerNearTarget(task.Engineer, task.Target)) {
			if (now - task.LastActionTick >= 800) {
				IssueEngineerRepairAction(task.Engineer, task.Target, "near-action");
				IssueEngineerClickedMission(task.Engineer, task.Target, Mission::Capture, "near-capture");
				task.LastActionTick = now;
			}

			if (now - task.LastMissionTick >= 1200) {
				IssueEngineerMission(task.Engineer, task.Target, Mission::Capture, "near-capture");
				task.LastMissionTick = now;
			}
			continue;
		}

		if (now - task.LastMissionTick >= 2500) {
			IssueEngineerRepairCommand(task.Engineer, task.Target, "retry");
			task.LastMissionTick = now;
			task.LastActionTick = now;
		}
	}

	for (DWORD key : finished) {
		engineerRepairTasks.erase(key);
	}
}

void TryEngineerRepairGrandCannon(BuildingClass* cannon) {
	if (!Config::isGrandCannonEngineerRepairEnabled() || !IsOwnGrandCannon(cannon)) {
		return;
	}

	const int threshold = Config::getGrandCannonEngineerRepairThreshold();
	const int healthPercent = HealthPercent(cannon);
	if (healthPercent <= 0 || healthPercent >= threshold) {
		return;
	}

	const DWORD now = GetTickCount();
	const DWORD key = GetBuildingKey(cannon);
	if (engineerRepairTasks.find(key) != engineerRepairTasks.end()) {
		return;
	}

	auto last = lastEngineerRepairTicks.find(key);
	if (last != lastEngineerRepairTicks.end() && now - last->second < 3000) {
		return;
	}

	InfantryClass* engineer = FindNearestEngineer(cannon);
	if (!engineer) {
		NotifyNoEngineer();
		lastEngineerRepairTicks[key] = now;
		return;
	}

	IssueEngineerRepair(engineer, cannon);
	lastEngineerRepairTicks[key] = now;
}

void IssueAttack(BuildingClass* source, BuildingClass* target, const char* reason) {
	if (!source || !target || !source->Owner || source->Owner != HouseClass::CurrentPlayer) {
		return;
	}

	__try {
		EventClass event(
			HouseClass::CurrentPlayer->ArrayIndex,
			TargetClass(source),
			Mission::Attack,
			TargetClass(target),
			TargetClass(target),
			TargetClass());
		EventClass::OutList.Add(event);

		Utils::LogFormat(
			"GrandCannonAssist attack: source=%08X target=%08X reason=%s",
			source->UniqueID,
			target->UniqueID,
			reason ? reason : "");
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Utils::Log("GrandCannonAssist attack failed.");
	}
}

void TurnGrandCannon(BuildingClass* source) {
	if (!source || !Config::isGrandCannonAutoTurnEnabled()) {
		return;
	}

	int degrees = Config::getGrandCannonTurnAngle();
	if (degrees == 0) {
		return;
	}

	degrees %= 360;
	if (degrees < 0) {
		degrees += 360;
	}

	__try {
		if (SessionClass::IsMultiplayer()) {
			Utils::LogFormat(
				"GrandCannonAssist turn skipped in multiplayer: source=%08X angle=%d",
				source->UniqueID,
				degrees);
			return;
		}

		const int raw = (degrees * 65536) / 360;
		const DirStruct facing(raw);
		source->PrimaryFacing.SetDesired(facing);
		source->SecondaryFacing.SetDesired(facing);
		//source->BarrelFacing.SetDesired(facing);
		source->TurretIsRotating = TRUE;
		Utils::LogFormat("GrandCannonAssist turn: source=%08X angle=%d raw=%d",
			source->UniqueID, degrees, raw);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Utils::Log("GrandCannonAssist turn failed.");
	}
}

void HandleOwnGrandCannonReady(BuildingClass* source) {
	auto enemy = FindNearestEnemyGrandCannonInRange(source);
	if (enemy) {
		IssueAttack(source, enemy, "own-ready");
		return;
	}

	TurnGrandCannon(source);
}

void HandleEnemyGrandCannonReady(BuildingClass* target) {
	for (auto own : GetOwnGrandCannons()) {
		if (IsInAttackRange(own, target)) {
			IssueAttack(own, target, "enemy-ready");
		}
	}
}

void ScanGrandCannons(bool forceActions) {
	if (!Config::isGrandCannonAssistEnabled() && !forceActions) {
		return;
	}

	std::unordered_set<DWORD> currentOwn;
	std::unordered_set<DWORD> currentEnemy;
	std::vector<BuildingClass*> newOwn;
	std::vector<BuildingClass*> newEnemy;

	const int count = GetBuildingArrayCount();
	for (int i = 0; i < count; ++i) {
		auto building = GetBuildingArrayItem(i);
		if (!IsGrandCannonReady(building)) {
			continue;
		}

		const DWORD key = GetBuildingKey(building);
		if (!key) {
			continue;
		}

		if (IsOwnGrandCannon(building)) {
			currentOwn.insert(key);
			if ((grandCannonKnownSetReady || forceActions)
				&& knownOwnGrandCannons.find(key) == knownOwnGrandCannons.end()) {
				newOwn.push_back(building);
			}
		} else if (IsEnemyGrandCannon(building)) {
			currentEnemy.insert(key);
			if ((grandCannonKnownSetReady || forceActions)
				&& knownEnemyGrandCannons.find(key) == knownEnemyGrandCannons.end()) {
				newEnemy.push_back(building);
			}
		}
	}

	knownOwnGrandCannons.swap(currentOwn);
	knownEnemyGrandCannons.swap(currentEnemy);

	if (!grandCannonKnownSetReady && !forceActions) {
		grandCannonKnownSetReady = true;
		return;
	}

	grandCannonKnownSetReady = true;

	for (auto building : newOwn) {
		HandleOwnGrandCannonReady(building);
	}

	for (auto building : newEnemy) {
		HandleEnemyGrandCannonReady(building);
	}

	TickEngineerRepairTasks();
	for (auto building : GetOwnGrandCannons()) {
		TryEngineerRepairGrandCannon(building);
	}
}

}

void InitGrandCannonAssist() {
	knownOwnGrandCannons.clear();
	knownEnemyGrandCannons.clear();
	lastEngineerRepairTicks.clear();
	engineerRepairTasks.clear();
	grandCannonKnownSetReady = false;
	lastNoEngineerMessageTick = 0;
}

void TickGrandCannonAssist() {
	ScanGrandCannons(false);
}

void ForceGrandCannonScan() {
	Utils::Log("GrandCannonAssist manual scan.");
	ScanGrandCannons(true);
}
