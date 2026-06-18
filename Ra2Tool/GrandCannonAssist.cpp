#include <windows.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <BuildingClass.h>
#include <EventClass.h>
#include <HouseClass.h>
#include <InfantryClass.h>
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

void IssueEngineerRepair(InfantryClass* engineer, BuildingClass* target) {
	if (!engineer || !target || engineer->Owner != HouseClass::CurrentPlayer) {
		return;
	}

	__try {
		EventClass event(
			HouseClass::CurrentPlayer->ArrayIndex,
			TargetClass(engineer),
			Mission::Repair,
			TargetClass(target),
			TargetClass(target),
			TargetClass());
		EventClass::OutList.Add(event);
		Utils::LogFormat(
			"GrandCannonAssist engineer repair: engineer=%08X target=%08X hp=%d%%",
			engineer->UniqueID,
			target->UniqueID,
			HealthPercent(target));
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Utils::Log("GrandCannonAssist engineer repair failed.");
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

	for (auto building : GetOwnGrandCannons()) {
		TryEngineerRepairGrandCannon(building);
	}
}

}

void InitGrandCannonAssist() {
	knownOwnGrandCannons.clear();
	knownEnemyGrandCannons.clear();
	lastEngineerRepairTicks.clear();
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
