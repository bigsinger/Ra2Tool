#include <windows.h>
#include <cmath>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <BuildingClass.h>
#include <EventClass.h>
#include <HouseClass.h>
#include <MapClass.h>
#include <SessionClass.h>
#include <TargetClass.h>
#include "Utils.h"
#include "Config.h"
#include "Ra2Helper.h"

namespace {

std::unordered_set<DWORD> knownOwnGrandCannons;
std::unordered_set<DWORD> knownEnemyGrandCannons;
bool grandCannonKnownSetReady = false;

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
			const int range = source->GetWeaponRange(0);
			int distanceCells = range > 0 ? range / 256 : 8;
			distanceCells = std::min(std::max(distanceCells, 3), 32);

			const double radians = static_cast<double>(degrees) * 3.14159265358979323846 / 180.0;
			const int dx = static_cast<int>(std::round(std::sin(radians) * distanceCells));
			const int dy = static_cast<int>(std::round(-std::cos(radians) * distanceCells));
			const CellStruct origin = source->GetMapCoords();
			CellStruct targetCell {
				static_cast<short>(origin.X + dx),
				static_cast<short>(origin.Y + dy)
			};

			if (!MapClass::Instance.TryGetCellAt(targetCell)) {
				targetCell = origin;
			}

			EventClass event(
				HouseClass::CurrentPlayer->ArrayIndex,
				TargetClass(source),
				Mission::Attack,
				TargetClass(targetCell),
				TargetClass(targetCell),
				TargetClass());
			EventClass::OutList.Add(event);

			Utils::LogFormat(
				"GrandCannonAssist turn event: source=%08X angle=%d cell=(%d,%d)",
				source->UniqueID,
				degrees,
				targetCell.X,
				targetCell.Y);
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
		if (!IsGrandCannon(building)) {
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
}

}

void InitGrandCannonAssist() {
	knownOwnGrandCannons.clear();
	knownEnemyGrandCannons.clear();
	grandCannonKnownSetReady = false;
}

void TickGrandCannonAssist() {
	ScanGrandCannons(false);
}

void ForceGrandCannonScan() {
	Utils::Log("GrandCannonAssist manual scan.");
	ScanGrandCannons(true);
}
