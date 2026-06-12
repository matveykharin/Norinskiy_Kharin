#include "world.h"

#include <algorithm>
#include <stdexcept>

// ---------------------------------------------------------------------------
// Навигация
// ---------------------------------------------------------------------------

Location& GetCurrentLocation(World& world) {
  auto it = world.locations.find(world.current_location_id);
  if (it == world.locations.end()) {
    throw std::runtime_error("Current location not found: " +
                             world.current_location_id);
  }
  return it->second;
}

bool MovePlayer(World& world, Direction dir) {
  Location& current = GetCurrentLocation(world);

  auto it = current.exits.find(dir);
  if (it == current.exits.end()) {
    return false;  // выхода в этом направлении нет
  }

  const std::string& next_id = it->second;
  if (world.locations.find(next_id) == world.locations.end()) {
    return false;  // локация существует в exits, но не загружена в мир
  }

  world.current_location_id = next_id;
  return true;
}

// ---------------------------------------------------------------------------
// Работа с объектами локации
// ---------------------------------------------------------------------------

GameObject* FindObject(Location& loc, ObjectType type) {
  for (auto& obj : loc.objects) {
    if (obj.type == type) {
      return &obj;
    }
  }
  return nullptr;
}

void RemoveObject(Location& loc, const std::string& object_id) {
  auto it = std::remove_if(loc.objects.begin(), loc.objects.end(),
                           [&object_id](const GameObject& obj) {
                             return obj.id == object_id;
                           });
  loc.objects.erase(it, loc.objects.end());
}

// ---------------------------------------------------------------------------
// Вспомогательные функции
// ---------------------------------------------------------------------------

std::vector<Direction> GetAvailableDirections(const Location& loc) {
  std::vector<Direction> dirs;
  for (const auto& [dir, _] : loc.exits) {
    dirs.push_back(dir);
  }
  return dirs;
}

std::string DirectionToString(Direction dir) {
  switch (dir) {
    case Direction::kNorth: return "север";
    case Direction::kSouth: return "юг";
    case Direction::kEast:  return "восток";
    case Direction::kWest:  return "запад";
  }
  return "неизвестно";
}
