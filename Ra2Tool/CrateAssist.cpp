#include <windows.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <MapClass.h>
#include <TacticalClass.h>
#include <FootClass.h>
#include <EventClass.h>
#include <HouseClass.h>
#include <MessageListClass.h>
#include <SessionClass.h>
#include <WaypointPathClass.h>
#include "Utils.h"
#include "Config.h"
#include "Ra2Header.h"
#include "Ra2Helper.h"

namespace {

struct CrateTarget {
	CellStruct Cell;
	int TimeLeft;
};

struct PendingPickupState {
	bool Active = false;
	bool Charging = false;
	std::vector<FootClass*> Units;
	std::vector<CellStruct> ApproachCells;
	CellStruct CrateCell = CellStruct::Empty;
	DWORD DueTick = 0;
	DWORD LastChargeTick = 0;
	DWORD ChargeUntilTick = 0;
};

struct RouteCrateState {
	bool Capturing = false;
	bool Running = false;
	std::vector<CellStruct> Route;
	std::vector<FootClass*> Units;
	size_t NextRouteIndex = 0;
	DWORD LastSampleTick = 0;
	DWORD LastRouteMoveTick = 0;
	DWORD LastCrateScanTick = 0;
};

PendingPickupState g_pendingPickup;
RouteCrateState g_routeCrate;

int AbsInt(int value) {
	return value < 0 ? -value : value;
}

int CellDistance2(const CellStruct& a, const CellStruct& b) {
	const int dx = static_cast<int>(a.X) - static_cast<int>(b.X);
	const int dy = static_cast<int>(a.Y) - static_cast<int>(b.Y);
	return dx * dx + dy * dy;
}

bool SameCell(const CellStruct& a, const CellStruct& b) {
	return a.X == b.X && a.Y == b.Y;
}


bool TryGetCurrentHouseIndex(int* outHouseIndex) {
	__try {
		if (!HouseClass::CurrentPlayer) {
			return false;
		}

		*outHouseIndex = HouseClass::CurrentPlayer->ArrayIndex;
		return true;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

int TryGetSelectedCount() {
	__try {
		return *reinterpret_cast<int*>(0x00A8ECC8);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return 0;
	}
}

TechnoClass* TryGetSelectedTechno(int index) {
	__try {
		auto selected = *reinterpret_cast<TechnoClass***>(0x00A8ECBC);
		return selected ? selected[index] : nullptr;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return nullptr;
	}
}

FootClass* TryAsFoot(TechnoClass* techno) {
	__try {
		if (!techno) {
			return nullptr;
		}

		const AbstractType type = techno->WhatAmI();
		if (type != AbstractType::Unit
			&& type != AbstractType::Infantry
			&& type != AbstractType::Aircraft) {
			return nullptr;
		}

		auto foot = static_cast<FootClass*>(techno);
		if (!foot->Owner
			|| foot->Owner != HouseClass::CurrentPlayer
			|| !foot->IsAlive
			|| !foot->IsOnMap
			|| foot->InLimbo) {
			return nullptr;
		}

		return foot;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return nullptr;
	}
}

CellStruct TryGetFootCell(FootClass* foot) {
	__try {
		return foot ? foot->GetMapCoords() : CellStruct::Empty;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return CellStruct::Empty;
	}
}

bool TryIsActiveCrateCell(const CellStruct& cell) {
	__try {
		CellClass* mapCell = MapClass::Instance.TryGetCellAt(cell);
		return mapCell && mapCell->OverlayTypeIndex != -1;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

bool TryGetCrateAtIndex(int index, CrateTarget* outCrate) {
	__try {
		MapClass& map = MapClass::Instance;
		const int timeLeft = map.Crates[index].CrateTimer.TimeLeft;
		if (timeLeft <= 0) {
			return false;
		}

		const CellStruct cell = map.Crates[index].Location;
		CellClass* mapCell = map.TryGetCellAt(cell);
		if (!mapCell || mapCell->OverlayTypeIndex == -1) {
			return false;
		}

		outCrate->Cell = cell;
		outCrate->TimeLeft = timeLeft;
		return true;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

bool TryCellExists(const CellStruct& cell) {
	__try {
		return MapClass::Instance.TryGetCellAt(cell) != nullptr;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

bool TryIssueMove(FootClass* foot, const CellStruct& cell) {
	int houseIndex = -1;
	if (!TryGetCurrentHouseIndex(&houseIndex) || !foot) {
		return false;
	}

	__try {
		EventClass event(
			houseIndex,
			TargetClass(foot),
			Mission::Move,
			TargetClass(cell),
			TargetClass(cell),
			TargetClass());
		EventClass::OutList.Add(event);

		return true;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

bool IsCustomToolbarWindowAtPoint(HWND hwnd) {
	while (hwnd) {
		char className[64] = {};
		GetClassNameA(hwnd, className, sizeof(className));
		if (strcmp(className, "RA2CustomToolbar") == 0) {
			return true;
		}
		hwnd = GetParent(hwnd);
	}

	return false;
}

bool TryScreenCursorToCell(CellStruct* outCell) {
	POINT cursor = {};
	if (!GetCursorPos(&cursor)) {
		return false;
	}

	if (IsCustomToolbarWindowAtPoint(WindowFromPoint(cursor))) {
		return false;
	}

	HWND gameHwnd = GetMainWindowForProcessId(GetCurrentProcessId());
	if (!gameHwnd) {
		return false;
	}

	POINT client = cursor;
	RECT rect = {};
	if (!ScreenToClient(gameHwnd, &client) || !GetClientRect(gameHwnd, &rect)) {
		return false;
	}

	if (client.x < rect.left || client.x >= rect.right || client.y < rect.top || client.y >= rect.bottom) {
		return false;
	}

	__try {
		Point2D point { client.x, client.y };
		const CoordStruct coord = TacticalClass::Instance->ClientToCoords(point);
		*outCell = CellClass::Coord2Cell(coord);
		return MapClass::Instance.TryGetCellAt(*outCell) != nullptr;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

bool TrySetGameWaypointMode(bool enabled) {
	__try {
		MapClass::Instance.SetWaypointMode(enabled ? 1 : 0, enabled);
		Utils::LogFormat("CrateAssist game waypoint mode: %d", enabled ? 1 : 0);
		return true;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		Utils::Log("CrateAssist game waypoint mode failed.");
		return false;
	}
}

bool AppendRouteCell(std::vector<CellStruct>* route, const CellStruct& cell) {
	if (!route || !TryCellExists(cell)) {
		return false;
	}

	if (!route->empty() && CellDistance2(route->back(), cell) < 2) {
		return false;
	}

	route->push_back(cell);
	return true;
}

bool TryImportPlanningRoute() {
	auto house = HouseClass::CurrentPlayer;
	if (!house) {
		return false;
	}

	std::vector<CellStruct> bestRoute;
	for (int pathIndex = 0; pathIndex < 12; ++pathIndex) {
		WaypointPathClass* path = house->PlanningPaths[pathIndex];
		if (!path || path->Waypoints.Count <= 0 || !path->Waypoints.Items) {
			continue;
		}

		std::vector<CellStruct> route;
		for (int waypointIndex = 0; waypointIndex < path->Waypoints.Count; ++waypointIndex) {
			AppendRouteCell(&route, path->Waypoints.Items[waypointIndex].Coords);
		}

		if (route.size() > bestRoute.size()) {
			bestRoute.swap(route);
		}
	}

	if (bestRoute.size() < 2) {
		return false;
	}

	g_routeCrate.Route.swap(bestRoute);
	Utils::LogFormat("CrateAssist imported planning route: points=%d",
		static_cast<int>(g_routeCrate.Route.size()));
	return true;
}

std::vector<FootClass*> GetSelectedFootUnits() {
	std::vector<FootClass*> units;
	const int count = TryGetSelectedCount();

	for (int i = 0; i < count; ++i) {
		FootClass* foot = TryAsFoot(TryGetSelectedTechno(i));
		if (!foot) {
			continue;
		}

		bool exists = false;
		for (auto item : units) {
			if (item == foot) {
				exists = true;
				break;
			}
		}

		if (!exists) {
			units.push_back(foot);
		}
	}

	return units;
}

std::vector<FootClass*> FilterLiveUnits(const std::vector<FootClass*>& units) {
	std::vector<FootClass*> live;
	for (auto unit : units) {
		if (TryAsFoot(unit)) {
			live.push_back(unit);
		}
	}
	return live;
}

std::vector<CrateTarget> GetActiveCrates() {
	std::vector<CrateTarget> crates;
	for (int i = 0; i < CRATE_MAX_COUNT; ++i) {
		CrateTarget crate = {};
		if (TryGetCrateAtIndex(i, &crate)) {
			crates.push_back(crate);
		}
	}
	return crates;
}

CellStruct GetGroupCenterCell(const std::vector<FootClass*>& units) {
	if (units.empty()) {
		return CellStruct::Empty;
	}

	int sumX = 0;
	int sumY = 0;
	int count = 0;

	for (auto unit : units) {
		const CellStruct cell = TryGetFootCell(unit);
		if (cell == CellStruct::Empty) {
			continue;
		}

		sumX += cell.X;
		sumY += cell.Y;
		++count;
	}

	if (count <= 0) {
		return CellStruct::Empty;
	}

	return CellStruct {
		static_cast<short>(sumX / count),
		static_cast<short>(sumY / count)
	};
}

bool FindNearestCrate(const std::vector<FootClass*>& units, CrateTarget* outCrate) {
	const auto crates = GetActiveCrates();
	if (crates.empty() || units.empty()) {
		return false;
	}

	const CellStruct center = GetGroupCenterCell(units);
	bool found = false;
	int bestScore = 0x7FFFFFFF;
	CrateTarget best = {};

	for (const auto& crate : crates) {
		const int score = CellDistance2(center, crate.Cell);
		if (!found || score < bestScore) {
			found = true;
			bestScore = score;
			best = crate;
		}
	}

	if (!found) {
		return false;
	}

	*outCrate = best;
	return true;
}

std::vector<CellStruct> BuildApproachCells(const CellStruct& crateCell, size_t neededCount) {
	std::vector<CellStruct> cells;
	int radius = 1;

	while (cells.size() < neededCount && radius <= 6) {
		for (int dy = -radius; dy <= radius; ++dy) {
			for (int dx = -radius; dx <= radius; ++dx) {
				if (dx == 0 && dy == 0) {
					continue;
				}
				if (AbsInt(dx) != radius && AbsInt(dy) != radius) {
					continue;
				}

				CellStruct cell {
					static_cast<short>(crateCell.X + dx),
					static_cast<short>(crateCell.Y + dy)
				};

				if (!TryCellExists(cell)) {
					continue;
				}

				cells.push_back(cell);
				if (cells.size() >= neededCount) {
					return cells;
				}
			}
		}

		++radius;
	}

	return cells;
}

std::vector<CellStruct> AssignApproachAndMove(std::vector<FootClass*>& sortedUnits, const CellStruct& crateCell) {
	std::vector<CellStruct> assignedCells;

	std::sort(sortedUnits.begin(), sortedUnits.end(), [crateCell](FootClass* a, FootClass* b) {
		return CellDistance2(TryGetFootCell(a), crateCell) < CellDistance2(TryGetFootCell(b), crateCell);
	});

	auto approachCells = BuildApproachCells(crateCell, sortedUnits.size());
	if (approachCells.empty()) {
		for (auto unit : sortedUnits) {
			TryIssueMove(unit, crateCell);
			assignedCells.push_back(crateCell);
		}
		return assignedCells;
	}

	std::vector<bool> used(approachCells.size(), false);
	for (auto unit : sortedUnits) {
		const CellStruct current = TryGetFootCell(unit);
		size_t bestIndex = 0;
		bool found = false;
		int bestScore = 0x7FFFFFFF;

		for (size_t i = 0; i < approachCells.size(); ++i) {
			if (used[i]) {
				continue;
			}

			const int score = CellDistance2(current, approachCells[i]);
			if (!found || score < bestScore) {
				found = true;
				bestScore = score;
				bestIndex = i;
			}
		}

		if (found) {
			used[bestIndex] = true;
			TryIssueMove(unit, approachCells[bestIndex]);
			assignedCells.push_back(approachCells[bestIndex]);
		} else {
			TryIssueMove(unit, crateCell);
			assignedCells.push_back(crateCell);
		}
	}

	return assignedCells;
}

bool StartPickupForUnits(std::vector<FootClass*> units, const CellStruct& crateCell) {
	units = FilterLiveUnits(units);
	if (units.empty()) {
		PrintGameMessage(L"请先选中单位");
		Utils::Log("CrateAssist pickup failed: no selected units.");
		return false;
	}

	if (!TryIsActiveCrateCell(crateCell)) {
		PrintGameMessage(L"未找到可拾取的箱子");
		Utils::Log("CrateAssist pickup failed: no active crate.");
		return false;
	}

	if (units.size() <= 1) {
		TryIssueMove(units[0], crateCell);
	} else {
		auto approachCells = AssignApproachAndMove(units, crateCell);
		const DWORD now = GetTickCount();
		g_pendingPickup.Active = true;
		g_pendingPickup.Charging = false;
		g_pendingPickup.Units = units;
		g_pendingPickup.ApproachCells = approachCells;
		g_pendingPickup.CrateCell = crateCell;
		const int configuredDelay = Config::getCrateApproachDelay();
		const int chargeDelay = std::min(std::max(configuredDelay, 1200), 6000);
		g_pendingPickup.DueTick = now + static_cast<DWORD>(chargeDelay);
		g_pendingPickup.LastChargeTick = 0;
		g_pendingPickup.ChargeUntilTick = 0;
	}

	Utils::LogFormat(
		"CrateAssist pickup: units=%d crate=(%d,%d)",
		static_cast<int>(units.size()),
		crateCell.X,
		crateCell.Y);
	return true;
}

double PointSegmentDistance2(const CellStruct& point, const CellStruct& a, const CellStruct& b) {
	const double px = point.X;
	const double py = point.Y;
	const double ax = a.X;
	const double ay = a.Y;
	const double bx = b.X;
	const double by = b.Y;
	const double vx = bx - ax;
	const double vy = by - ay;
	const double wx = px - ax;
	const double wy = py - ay;
	const double length2 = vx * vx + vy * vy;

	if (length2 <= 0.0001) {
		const double dx = px - ax;
		const double dy = py - ay;
		return dx * dx + dy * dy;
	}

	double t = (wx * vx + wy * vy) / length2;
	if (t < 0.0) {
		t = 0.0;
	} else if (t > 1.0) {
		t = 1.0;
	}

	const double cx = ax + t * vx;
	const double cy = ay + t * vy;
	const double dx = px - cx;
	const double dy = py - cy;
	return dx * dx + dy * dy;
}

bool IsCrateNearRoute(const CellStruct& crateCell) {
	const int width = Config::getCrateRouteWidth();
	const double threshold = static_cast<double>(width * width);

	if (g_routeCrate.Route.empty()) {
		return false;
	}

	if (g_routeCrate.Route.size() == 1) {
		return CellDistance2(crateCell, g_routeCrate.Route[0]) <= width * width;
	}

	for (size_t i = 1; i < g_routeCrate.Route.size(); ++i) {
		if (PointSegmentDistance2(crateCell, g_routeCrate.Route[i - 1], g_routeCrate.Route[i]) <= threshold) {
			return true;
		}
	}

	return false;
}

bool FindNearestRouteCrate(const std::vector<FootClass*>& units, CrateTarget* outCrate) {
	const auto crates = GetActiveCrates();
	if (crates.empty() || units.empty()) {
		return false;
	}

	const CellStruct center = GetGroupCenterCell(units);
	bool found = false;
	int bestScore = 0x7FFFFFFF;
	CrateTarget best = {};

	for (const auto& crate : crates) {
		if (!IsCrateNearRoute(crate.Cell)) {
			continue;
		}

		const int score = CellDistance2(center, crate.Cell);
		if (!found || score < bestScore) {
			found = true;
			bestScore = score;
			best = crate;
		}
	}

	if (!found) {
		return false;
	}

	*outCrate = best;
	return true;
}

bool IsUnitNearCell(FootClass* unit, const CellStruct& cell, int tolerance) {
	const CellStruct current = TryGetFootCell(unit);
	return AbsInt(static_cast<int>(current.X) - static_cast<int>(cell.X)) <= tolerance
		&& AbsInt(static_cast<int>(current.Y) - static_cast<int>(cell.Y)) <= tolerance;
}

bool IsPickupApproachReady(const std::vector<FootClass*>& units, const std::vector<CellStruct>& approachCells, const CellStruct& crateCell) {
	if (units.empty()) {
		return false;
	}

	for (size_t i = 0; i < units.size(); ++i) {
		FootClass* unit = units[i];
		const CellStruct approach = i < approachCells.size() ? approachCells[i] : crateCell;
		if (IsUnitNearCell(unit, approach, 1) || IsUnitNearCell(unit, crateCell, 1)) {
			continue;
		}

		return false;
	}

	return true;
}

void IssueCrateCharge(const std::vector<FootClass*>& units, const CellStruct& crateCell) {
	auto sortedUnits = FilterLiveUnits(units);
	std::sort(sortedUnits.begin(), sortedUnits.end(), [crateCell](FootClass* a, FootClass* b) {
		return CellDistance2(TryGetFootCell(a), crateCell) < CellDistance2(TryGetFootCell(b), crateCell);
	});

	for (auto unit : sortedUnits) {
		TryIssueMove(unit, crateCell);
	}
}

void BeginCrateCharge(DWORD now) {
	g_pendingPickup.Charging = true;
	g_pendingPickup.LastChargeTick = 0;
	g_pendingPickup.ChargeUntilTick = now + 7000;
	Utils::LogFormat(
		"CrateAssist pickup begin charge: units=%d crate=(%d,%d)",
		static_cast<int>(g_pendingPickup.Units.size()),
		g_pendingPickup.CrateCell.X,
		g_pendingPickup.CrateCell.Y);
}

void TickPendingPickup() {
	if (!g_pendingPickup.Active) {
		return;
	}

	const DWORD now = GetTickCount();
	g_pendingPickup.Units = FilterLiveUnits(g_pendingPickup.Units);
	if (g_pendingPickup.Units.empty() || !TryIsActiveCrateCell(g_pendingPickup.CrateCell)) {
		g_pendingPickup.Active = false;
		return;
	}

	if (!g_pendingPickup.Charging) {
		if (!IsPickupApproachReady(g_pendingPickup.Units, g_pendingPickup.ApproachCells, g_pendingPickup.CrateCell)
			&& now < g_pendingPickup.DueTick) {
			return;
		}

		BeginCrateCharge(now);
	}

	if (now > g_pendingPickup.ChargeUntilTick) {
		g_pendingPickup.Active = false;
		return;
	}

	if (g_pendingPickup.LastChargeTick == 0 || now - g_pendingPickup.LastChargeTick >= 220) {
		g_pendingPickup.LastChargeTick = now;
		IssueCrateCharge(g_pendingPickup.Units, g_pendingPickup.CrateCell);
	}
}

void SampleRoutePoint() {
	const DWORD now = GetTickCount();
	if (now - g_routeCrate.LastSampleTick < static_cast<DWORD>(Config::getCrateRouteSampleInterval())) {
		return;
	}

	if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0) {
		return;
	}

	CellStruct cell = CellStruct::Empty;
	if (!TryScreenCursorToCell(&cell)) {
		return;
	}

	g_routeCrate.LastSampleTick = now;
	if (!g_routeCrate.Route.empty() && CellDistance2(g_routeCrate.Route.back(), cell) < 2) {
		return;
	}

	if (AppendRouteCell(&g_routeCrate.Route, cell)) {
		Utils::LogFormat("CrateAssist route sample: (%d,%d)", cell.X, cell.Y);
	}
}

void StartRouteCapture() {
	g_routeCrate.Capturing = true;
	g_routeCrate.Running = false;
	g_routeCrate.Route.clear();
	g_routeCrate.Units.clear();
	g_routeCrate.NextRouteIndex = 0;
	g_routeCrate.LastSampleTick = 0;
	g_routeCrate.LastRouteMoveTick = 0;
	g_routeCrate.LastCrateScanTick = 0;
	TrySetGameWaypointMode(true);
	PrintGameMessage(L"开始圈定跑图路线");
	Utils::Log("CrateAssist route capture started.");
}

void StartRouteRun() {
	g_routeCrate.Capturing = false;
	TryImportPlanningRoute();
	TrySetGameWaypointMode(false);

	if (g_routeCrate.Route.size() < 2) {
		g_routeCrate.Running = false;
		PrintGameMessage(L"跑图路线点不足");
		Utils::Log("CrateAssist route run failed: not enough route points.");
		return;
	}

	g_routeCrate.Units = FilterLiveUnits(GetSelectedFootUnits());
	if (g_routeCrate.Units.empty()) {
		g_routeCrate.Running = false;
		PrintGameMessage(L"请先选中单位");
		Utils::Log("CrateAssist route run failed: no selected units.");
		return;
	}

	g_routeCrate.Running = true;
	g_routeCrate.NextRouteIndex = 0;
	g_routeCrate.LastRouteMoveTick = 0;
	g_routeCrate.LastCrateScanTick = 0;
	PrintGameMessage(L"开始跑图捡箱");
	Utils::LogFormat(
		"CrateAssist route run started: units=%d route=%d",
		static_cast<int>(g_routeCrate.Units.size()),
		static_cast<int>(g_routeCrate.Route.size()));
}

void StopRouteRun() {
	g_routeCrate.Capturing = false;
	g_routeCrate.Running = false;
	g_routeCrate.Units.clear();
	g_pendingPickup.Active = false;
	TrySetGameWaypointMode(false);
	PrintGameMessage(L"已停止跑图捡箱");
	Utils::Log("CrateAssist route run stopped.");
}

void TickRouteRun() {
	if (!g_routeCrate.Running) {
		return;
	}

	g_routeCrate.Units = FilterLiveUnits(g_routeCrate.Units);
	if (g_routeCrate.Units.empty()) {
		StopRouteRun();
		return;
	}

	if (g_pendingPickup.Active) {
		return;
	}

	const DWORD now = GetTickCount();
	if (now - g_routeCrate.LastCrateScanTick >= 500) {
		g_routeCrate.LastCrateScanTick = now;

		CrateTarget crate = {};
		if (FindNearestRouteCrate(g_routeCrate.Units, &crate)) {
			StartPickupForUnits(g_routeCrate.Units, crate.Cell);
			return;
		}
	}

	if (g_routeCrate.Route.empty()) {
		return;
	}

	if (g_routeCrate.LastRouteMoveTick != 0
		&& now - g_routeCrate.LastRouteMoveTick < static_cast<DWORD>(Config::getCrateRouteMoveInterval())) {
		return;
	}

	g_routeCrate.LastRouteMoveTick = now;
	const CellStruct dest = g_routeCrate.Route[g_routeCrate.NextRouteIndex % g_routeCrate.Route.size()];
	for (auto unit : g_routeCrate.Units) {
		TryIssueMove(unit, dest);
	}

	g_routeCrate.NextRouteIndex = (g_routeCrate.NextRouteIndex + 1) % g_routeCrate.Route.size();
}

enum class FormationMode {
	Square,
	Vertical,
	Horizontal,
};

int CeilDiv(int value, int divisor) {
	return divisor > 0 ? (value + divisor - 1) / divisor : value;
}

int SquareColumns(int count) {
	int columns = 1;
	while (columns * columns < count) {
		++columns;
	}
	return columns;
}

bool HasCell(const std::vector<CellStruct>& cells, const CellStruct& cell) {
	for (const auto& item : cells) {
		if (SameCell(item, cell)) {
			return true;
		}
	}
	return false;
}

CellStruct FindFormationCell(const CellStruct& preferred, const CellStruct& center, const std::vector<CellStruct>& used) {
	if (TryCellExists(preferred) && !HasCell(used, preferred)) {
		return preferred;
	}

	for (int radius = 1; radius <= 5; ++radius) {
		for (int dy = -radius; dy <= radius; ++dy) {
			for (int dx = -radius; dx <= radius; ++dx) {
				if (AbsInt(dx) != radius && AbsInt(dy) != radius) {
					continue;
				}

				CellStruct cell {
					static_cast<short>(preferred.X + dx),
					static_cast<short>(preferred.Y + dy)
				};

				if (TryCellExists(cell) && !HasCell(used, cell)) {
					return cell;
				}
			}
		}
	}

	if (TryCellExists(center) && !HasCell(used, center)) {
		return center;
	}

	return preferred;
}

std::vector<CellStruct> BuildFormationTargets(int count, FormationMode mode, const CellStruct& center) {
	std::vector<CellStruct> targets;
	if (count <= 0) {
		return targets;
	}

	int columns = 1;
	int rows = 1;
	if (mode == FormationMode::Square) {
		columns = SquareColumns(count);
		rows = CeilDiv(count, columns);
	} else if (mode == FormationMode::Vertical) {
		rows = std::min(10, count);
		columns = CeilDiv(count, rows);
	} else {
		columns = std::min(10, count);
		rows = CeilDiv(count, columns);
	}

	const int startX = -(columns / 2);
	const int startY = -(rows / 2);

	for (int i = 0; i < count; ++i) {
		int col = 0;
		int row = 0;
		if (mode == FormationMode::Vertical) {
			col = i / rows;
			row = i % rows;
		} else {
			col = i % columns;
			row = i / columns;
		}

		CellStruct preferred {
			static_cast<short>(center.X + startX + col),
			static_cast<short>(center.Y + startY + row)
		};
		targets.push_back(FindFormationCell(preferred, center, targets));
	}

	return targets;
}

void ApplySelectedFormation(FormationMode mode, const char* logName, const wchar_t* message) {
	auto units = FilterLiveUnits(GetSelectedFootUnits());
	if (units.empty()) {
		PrintGameMessage(L"请先选中单位");
		Utils::LogFormat("Formation failed: %s no selected units.", logName ? logName : "");
		return;
	}

	const CellStruct center = GetGroupCenterCell(units);
	auto targets = BuildFormationTargets(static_cast<int>(units.size()), mode, center);
	std::vector<bool> used(targets.size(), false);

	for (auto unit : units) {
		const CellStruct current = TryGetFootCell(unit);
		size_t bestIndex = 0;
		bool found = false;
		int bestScore = 0x7FFFFFFF;

		for (size_t i = 0; i < targets.size(); ++i) {
			if (used[i]) {
				continue;
			}

			const int score = CellDistance2(current, targets[i]);
			if (!found || score < bestScore) {
				found = true;
				bestScore = score;
				bestIndex = i;
			}
		}

		if (found) {
			used[bestIndex] = true;
			TryIssueMove(unit, targets[bestIndex]);
		}
	}

	PrintGameMessage(message);
	Utils::LogFormat("Formation applied: %s units=%d", logName ? logName : "", static_cast<int>(units.size()));
}

}

void StartOptimalCratePickup() {
	auto units = GetSelectedFootUnits();
	if (units.empty()) {
		PrintGameMessage(L"请先选中单位");
		Utils::Log("CrateAssist pickup failed: no selected units.");
		return;
	}

	CrateTarget crate = {};
	if (!FindNearestCrate(units, &crate)) {
		PrintGameMessage(L"未找到可拾取的箱子");
		Utils::Log("CrateAssist pickup failed: no nearest crate.");
		return;
	}

	StartPickupForUnits(units, crate.Cell);
}

void ToggleRouteCratePickup() {
	if (g_routeCrate.Running) {
		StopRouteRun();
		return;
	}

	if (g_routeCrate.Capturing) {
		StartRouteRun();
		return;
	}

	StartRouteCapture();
}

void TickCrateAssist() {
	if (g_routeCrate.Capturing) {
		SampleRoutePoint();
	}

	TickPendingPickup();
	TickRouteRun();
}

bool IsRouteCrateCaptureActive() {
	return g_routeCrate.Capturing;
}

bool IsRouteCrateRunActive() {
	return g_routeCrate.Running;
}

void ApplySelectedFormationSquare() {
	ApplySelectedFormation(FormationMode::Square, "square", L"阵型口");
}

void ApplySelectedFormationVertical() {
	ApplySelectedFormation(FormationMode::Vertical, "vertical", L"阵型1");
}

void ApplySelectedFormationHorizontal() {
	ApplySelectedFormation(FormationMode::Horizontal, "horizontal", L"阵型一");
}
