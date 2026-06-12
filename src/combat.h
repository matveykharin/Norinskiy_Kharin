#ifndef EVIL_NERDS_COMBAT_H_
#define EVIL_NERDS_COMBAT_H_

#include <map>
#include <string>

#include "types.h"

// ---------------------------------------------------------------------------
// Combat — пошаговая боевая система
//
// Порядок хода:
//   1. Игрок выбирает действие (атака / эффект / комбо / предмет)
//   2. Если враг жив — враг атакует
//   3. Проверка конца боя
// ---------------------------------------------------------------------------

// Запускает полный бой до победы или поражения.
// Возвращает результат боя.
CombatResult RunCombat(
    Player& player,
    EnemyData enemy,
    const std::map<std::string, Notebook>& notebooks);

// ---------------------------------------------------------------------------
// Отдельные шаги боя — используются внутри RunCombat,
// вынесены в публичный интерфейс для тестирования
// ---------------------------------------------------------------------------

// Обычная атака тетрадью — урон на основе суммы base_damage тетрадей
void PlayerAttack(CombatState& state,
                  const std::map<std::string, Notebook>& notebooks);

// Применяет один из трёх эффектов
void PlayerUseEffect(CombatState& state, EffectType effect,
                     const std::map<std::string, Notebook>& notebooks);

// Комбо-удар: тратит 1 страницу из каждой тетради (снижает base_damage),
// наносит суммарный урон всех тетрадей с множителем
void PlayerUseCombo(CombatState& state,
                    std::map<std::string, Notebook>& notebooks);

// Использует предмет из инвентаря во время боя
void PlayerUseItemInCombat(CombatState& state, const std::string& item_id);

// Ход врага
void EnemyTurn(CombatState& state);

// Проверяет не закончился ли бой, обновляет state.result
void CheckBattleEnd(CombatState& state);

// ---------------------------------------------------------------------------
// Вспомогательные функции расчёта
// ---------------------------------------------------------------------------

// Рассчитывает итоговый урон с учётом точности и уклонения.
// Возвращает 0 если атака промахнулась.
int CalcDamage(int base_damage, int accuracy, int dodge);

// Проверяет попадание по шансу (0–100)
bool RollHit(int accuracy, int dodge);

#endif  // EVIL_NERDS_COMBAT_H_
