#include "combat.h"

#include <cstdlib>
#include <iostream>
#include <string>

#include "player.h"
#include "renderer.h"

// ---------------------------------------------------------------------------
// Константы баланса
// ---------------------------------------------------------------------------

static constexpr int kComboMultiplierPercent = 150;  // комбо = 150% урона
static constexpr int kDebuffAmount           = 3;    // дебафф снижает атаку на 3
static constexpr int kStunTurns              = 1;    // оглушение на 1 ход
static constexpr int kExtraDamageBonus       = 5;    // доп. урон от эффекта

// ---------------------------------------------------------------------------
// Вспомогательные функции
// ---------------------------------------------------------------------------

bool RollHit(int accuracy, int dodge) {
  int chance = accuracy - dodge;
  if (chance < 5)  chance = 5;   // минимальный шанс попасть — 5%
  if (chance > 95) chance = 95;  // максимальный шанс попасть — 95%
  return (std::rand() % 100) < chance;
}

int CalcDamage(int base_damage, int accuracy, int dodge) {
  if (!RollHit(accuracy, dodge)) return 0;
  return base_damage;
}

// ---------------------------------------------------------------------------
// Действия игрока
// ---------------------------------------------------------------------------

void PlayerAttack(CombatState& state,
                  const std::map<std::string, Notebook>& notebooks) {
  int damage = GetTotalNotebookDamage(state.player, notebooks);
  if (damage == 0) damage = state.player.stats.attack;  // fallback без тетрадей

  int dealt = CalcDamage(damage,
                         state.player.stats.accuracy,
                         state.enemy.stats.dodge);
  if (dealt > 0) {
    state.enemy.stats.hp -= dealt;
    if (state.enemy.stats.hp < 0) state.enemy.stats.hp = 0;
    RenderMessage("Рэн атакует тетрадью! Урон: " + std::to_string(dealt));
  } else {
    RenderMessage("Рэн промахнулся!");
  }
}

void PlayerUseEffect(CombatState& state, EffectType effect,
                     const std::map<std::string, Notebook>& notebooks) {
  switch (effect) {
    case EffectType::kExtraDamage: {
      int base   = GetTotalNotebookDamage(state.player, notebooks);
      int damage = base + kExtraDamageBonus;
      int dealt  = CalcDamage(damage,
                              state.player.stats.accuracy,
                              state.enemy.stats.dodge);
      if (dealt > 0) {
        state.enemy.stats.hp -= dealt;
        if (state.enemy.stats.hp < 0) state.enemy.stats.hp = 0;
        RenderMessage("Усиленный удар! Урон: " + std::to_string(dealt));
      } else {
        RenderMessage("Рэн промахнулся с усиленным ударом!");
      }
      break;
    }
    case EffectType::kDebuff: {
      state.enemy.stats.attack -= kDebuffAmount;
      if (state.enemy.stats.attack < 0) state.enemy.stats.attack = 0;
      RenderMessage("Атака врага снижена на " +
                    std::to_string(kDebuffAmount) + "!");
      break;
    }
    case EffectType::kStun: {
      state.enemy_stunned = true;
      RenderMessage("Враг оглушён и пропустит следующий ход!");
      break;
    }
  }
}

void PlayerUseCombo(CombatState& state,
                    std::map<std::string, Notebook>& notebooks) {
  if (!CanUseCombo(state.player)) {
    RenderMessage("Недостаточно тетрадей для комбо!");
    return;
  }

  // Считаем суммарный урон и снижаем base_damage каждой тетради на 1
  int total_damage = 0;
  for (const auto& nb_id : state.player.notebook_ids) {
    auto it = notebooks.find(nb_id);
    if (it != notebooks.end()) {
      total_damage += it->second.base_damage;
      it->second.base_damage -= 1;
      if (it->second.base_damage < 1) it->second.base_damage = 1;
    }
  }

  int combo_damage = (total_damage * kComboMultiplierPercent) / 100;
  int dealt = CalcDamage(combo_damage,
                         state.player.stats.accuracy,
                         state.enemy.stats.dodge);
  if (dealt > 0) {
    state.enemy.stats.hp -= dealt;
    if (state.enemy.stats.hp < 0) state.enemy.stats.hp = 0;
    RenderMessage("КОМБО-УДАР! Урон: " + std::to_string(dealt));
  } else {
    RenderMessage("Комбо-удар не попал!");
  }
}

void PlayerUseItemInCombat(CombatState& state, const std::string& item_id) {
  if (!HasItem(state.player, item_id)) {
    RenderMessage("Такого предмета нет в инвентаре!");
    return;
  }

  for (const auto& item : state.player.inventory) {
    if (item.id == item_id) {
      switch (item.type) {
        case ItemType::kHealthPotion:
          HealPlayer(state.player, item.value);
          RenderMessage("Использовано: " + item.name +
                        ". Восстановлено HP: " + std::to_string(item.value));
          break;
        case ItemType::kConsumable:
          // Шпаргалка: +15 к точности на этот бой
          // Кроссовки: +10 к уклонению на этот бой
          if (item.id == "consumable_focus") {
            state.player.stats.accuracy += item.value;
            RenderMessage("Использована шпаргалка! Точность +" +
                          std::to_string(item.value));
          } else if (item.id == "consumable_dodge") {
            state.player.stats.dodge += item.value;
            RenderMessage("Надеты кроссовки! Уклонение +" +
                          std::to_string(item.value));
          }
          break;
      }
      break;
    }
  }
  RemoveItem(state.player, item_id);
}

// ---------------------------------------------------------------------------
// Ход врага
// ---------------------------------------------------------------------------

void EnemyTurn(CombatState& state) {
  if (state.enemy_stunned) {
    RenderMessage(state.enemy.name + " оглушён и пропускает ход.");
    state.enemy_stunned = false;
    return;
  }

  int dealt = CalcDamage(state.enemy.stats.attack,
                         state.enemy.stats.accuracy,
                         state.player.stats.dodge);
  if (dealt > 0) {
    DamagePlayer(state.player, dealt);
    RenderMessage(state.enemy.name + " атакует! Урон: " +
                  std::to_string(dealt));
  } else {
    RenderMessage(state.enemy.name + " промахнулся!");
  }
}

// ---------------------------------------------------------------------------
// Проверка конца боя
// ---------------------------------------------------------------------------

void CheckBattleEnd(CombatState& state) {
  if (state.enemy.stats.hp <= 0) {
    state.result = CombatResult::kPlayerWon;
  } else if (!IsPlayerAlive(state.player)) {
    state.result = CombatResult::kPlayerLost;
  }
}

// ---------------------------------------------------------------------------
// Главный цикл боя
// ---------------------------------------------------------------------------

CombatResult RunCombat(Player& player,
                       EnemyData enemy,
                       const std::map<std::string, Notebook>& notebooks) {
  // notebooks копируем — комбо меняет base_damage только в рамках боя
  std::map<std::string, Notebook> combat_notebooks = notebooks;
  CombatState state(player, std::move(enemy));

  RenderMessage("\n=== БОЙ НАЧАЛСЯ ===");
  RenderMessage(state.enemy.meet_dialogue);

  while (state.result == CombatResult::kInProgress) {
    RenderCombat(state);

    // Ход игрока
    Command cmd = GetCombatCommand(state.player);

    switch (cmd) {
      case Command::kAttack:
        PlayerAttack(state, combat_notebooks);
        break;
      case Command::kUseEffect: {
        EffectType effect = GetEffectChoice();
        PlayerUseEffect(state, effect, combat_notebooks);
        break;
      }
      case Command::kUseCombo:
        PlayerUseCombo(state, combat_notebooks);
        break;
      case Command::kOpenInventory: {
        std::string item_id = GetItemChoice(state.player);
        if (!item_id.empty()) {
          PlayerUseItemInCombat(state, item_id);
        }
        break;
      }
      default:
        RenderMessage("Неверная команда. Попробуй ещё раз.");
        continue;  // не тратим ход врага
    }

    CheckBattleEnd(state);
    if (state.result != CombatResult::kInProgress) break;

    // Ход врага
    EnemyTurn(state);
    CheckBattleEnd(state);
  }

  if (state.result == CombatResult::kPlayerWon) {
    RenderMessage("\n" + state.enemy.name + " повержен!");
  } else {
    RenderMessage("\nРэн потерял сознание...");
  }

  return state.result;
}
