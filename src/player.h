#ifndef EVIL_NERDS_PLAYER_H_
#define EVIL_NERDS_PLAYER_H_

#include <map>
#include <string>

#include "types.h"

// ---------------------------------------------------------------------------
// Player — управление состоянием игрока
// ---------------------------------------------------------------------------

// --- Тетради ---

// Добавляет тетрадь в коллекцию игрока
void AddNotebook(Player& player, const std::string& notebook_id);

// Проверяет, есть ли у игрока тетрадь с данным id
bool HasNotebook(const Player& player, const std::string& notebook_id);

// Возвращает true если игрок собрал все 7 тетрадей — условие победы
bool AllNotebooksCollected(const Player& player);

// Возвращает суммарный урон всех собранных тетрадей
int GetTotalNotebookDamage(
    const Player& player,
    const std::map<std::string, Notebook>& notebooks);

// --- Инвентарь ---

// Добавляет предмет в инвентарь. Возвращает false если предмет уже есть.
bool AddItem(Player& player, const Item& item);

// Удаляет предмет из инвентаря по id
void RemoveItem(Player& player, const std::string& item_id);

// Использует предмет: применяет эффект и удаляет из инвентаря.
// Возвращает false если предмета нет в инвентаре.
bool UseItem(Player& player, const std::string& item_id);

// Проверяет, есть ли предмет в инвентаре
bool HasItem(const Player& player, const std::string& item_id);

// --- Здоровье ---

// Восстанавливает HP игроку (не выше max_hp)
void HealPlayer(Player& player, int amount);

// Наносит урон игроку
void DamagePlayer(Player& player, int amount);

// Возвращает true если игрок жив
bool IsPlayerAlive(const Player& player);

// --- Комбо ---

// Возвращает true если у игрока есть 2 или больше тетрадей для комбо
bool CanUseCombo(const Player& player);

#endif  // EVIL_NERDS_PLAYER_H_
