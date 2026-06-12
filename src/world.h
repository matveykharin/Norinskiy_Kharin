#ifndef EVIL_NERDS_WORLD_H_
#define EVIL_NERDS_WORLD_H_

#include <string>
#include <vector>

#include "types.h"

// ---------------------------------------------------------------------------
// World — навигация по миру и работа с объектами локаций
// ---------------------------------------------------------------------------

// Возвращает ссылку на текущую локацию
Location& GetCurrentLocation(World& world);

// Перемещает игрока в указанном направлении.
// Возвращает true если переход успешен, false если выхода нет.
bool MovePlayer(World& world, Direction dir);

// Ищет первый объект заданного типа в локации.
// Возвращает указатель на объект или nullptr если не найден.
GameObject* FindObject(Location& loc, ObjectType type);

// Удаляет объект из локации по id (после победы над врагом, открытия сундука).
void RemoveObject(Location& loc, const std::string& object_id);

// Возвращает список направлений, в которых есть выходы из локации.
std::vector<Direction> GetAvailableDirections(const Location& loc);

// Возвращает строковое название направления (для вывода игроку).
std::string DirectionToString(Direction dir);

#endif  // EVIL_NERDS_WORLD_H_
