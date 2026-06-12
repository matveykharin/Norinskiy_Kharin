#include "renderer.h"

#include <iostream>
#include <string>

#include "input.h"
#include "player.h"
#include "world.h"

// ---------------------------------------------------------------------------
// Вспомогательные константы оформления
// ---------------------------------------------------------------------------

static const std::string kSeparator  = "----------------------------------------";
static const std::string kSeparatorB = "========================================";

// ---------------------------------------------------------------------------
// Вспомогательные функции
// ---------------------------------------------------------------------------

static std::string HpBar(int hp, int max_hp) {
  const int kBarWidth = 20;
  int filled = (max_hp > 0) ? (hp * kBarWidth / max_hp) : 0;
  std::string bar = "[";
  for (int i = 0; i < kBarWidth; ++i) {
    bar += (i < filled) ? "#" : ".";
  }
  bar += "] " + std::to_string(hp) + "/" + std::to_string(max_hp);
  return bar;
}

static std::string ObjectTypeToString(ObjectType type) {
  switch (type) {
    case ObjectType::kEnemy: return "враг";
    case ObjectType::kTrap:  return "ловушка";
    case ObjectType::kChest: return "сундук";
    case ObjectType::kNpc:   return "NPC";
  }
  return "";
}

// ---------------------------------------------------------------------------
// Вывод локации
// ---------------------------------------------------------------------------

void RenderLocation(const Location& loc) {
  std::cout << "\n" << kSeparatorB << "\n";
  std::cout << loc.name << "\n";
  std::cout << kSeparatorB << "\n";
  std::cout << loc.description << "\n";
  std::cout << kSeparator << "\n";

  // Выходы
  std::vector<Direction> dirs = GetAvailableDirections(loc);
  if (!dirs.empty()) {
    std::cout << "Выходы: ";
    bool first = true;
    for (const auto& dir : dirs) {
      if (!first) std::cout << ", ";
      std::cout << DirectionToString(dir);
      first = false;
    }
    std::cout << "\n";
  } else {
    std::cout << "Выходов нет.\n";
  }

  // Объекты в локации
  if (!loc.objects.empty()) {
    std::cout << kSeparator << "\n";
    std::cout << "Здесь есть:\n";
    for (const auto& obj : loc.objects) {
      std::cout << "  [" << ObjectTypeToString(obj.type) << "] ";
      switch (obj.type) {
        case ObjectType::kEnemy:
          if (obj.enemy) std::cout << obj.enemy->name;
          break;
        case ObjectType::kTrap:
          if (obj.trap) std::cout << obj.trap->name;
          break;
        case ObjectType::kChest:
          if (obj.chest) {
            std::cout << "Сундук";
            if (obj.chest->is_opened) std::cout << " (открыт)";
          }
          break;
        case ObjectType::kNpc:
          if (obj.npc) std::cout << obj.npc->name;
          break;
      }
      std::cout << "\n";
    }
  }

  std::cout << kSeparator << "\n";
  std::cout << "Команды: север/юг/восток/запад, осмотреться, инвентарь\n";
}

void RenderLocationFull(const Location& loc) {
  std::cout << "\n" << kSeparatorB << "\n";
  std::cout << loc.name << "\n";
  std::cout << kSeparatorB << "\n";
  std::cout << loc.full_description << "\n";
  std::cout << kSeparator << "\n";

  std::vector<Direction> dirs = GetAvailableDirections(loc);
  if (!dirs.empty()) {
    std::cout << "Выходы: ";
    bool first = true;
    for (const auto& dir : dirs) {
      if (!first) std::cout << ", ";
      std::cout << DirectionToString(dir);
      first = false;
    }
    std::cout << "\n";
  }
}

// ---------------------------------------------------------------------------
// Инвентарь
// ---------------------------------------------------------------------------

void RenderInventory(const Player& player,
                     const std::map<std::string, Notebook>& notebooks) {
  std::cout << "\n" << kSeparatorB << "\n";
  std::cout << "ИНВЕНТАРЬ\n";
  std::cout << kSeparatorB << "\n";

  // Тетради
  std::cout << "Тетради (" << player.notebook_ids.size() << "/7):\n";
  if (player.notebook_ids.empty()) {
    std::cout << "  (нет тетрадей)\n";
  } else {
    for (const auto& nb_id : player.notebook_ids) {
      auto it = notebooks.find(nb_id);
      if (it != notebooks.end()) {
        std::cout << "  - " << it->second.name
                  << " [урон: " << it->second.base_damage << "]\n";
        std::cout << "    Комбо: " << it->second.combo_effect_description << "\n";
      }
    }
  }

  std::cout << kSeparator << "\n";

  // Предметы
  std::cout << "Предметы:\n";
  if (player.inventory.empty()) {
    std::cout << "  (инвентарь пуст)\n";
  } else {
    int idx = 1;
    for (const auto& item : player.inventory) {
      std::cout << "  " << idx++ << ". " << item.name
                << " — " << item.description << "\n";
    }
  }

  std::cout << kSeparator << "\n";
  std::cout << "HP: " << HpBar(player.stats.hp, player.stats.max_hp) << "\n";
  std::cout << kSeparatorB << "\n";
}

// ---------------------------------------------------------------------------
// Бой
// ---------------------------------------------------------------------------

void RenderCombat(const CombatState& state) {
  std::cout << "\n" << kSeparatorB << "\n";
  std::cout << "  БОЙ\n";
  std::cout << kSeparatorB << "\n";

  std::cout << "Рэн      HP: "
            << HpBar(state.player.stats.hp, state.player.stats.max_hp) << "\n";
  std::cout << state.enemy.name << " HP: "
            << HpBar(state.enemy.stats.hp, state.enemy.stats.max_hp) << "\n";

  if (state.enemy_stunned) {
    std::cout << "[" << state.enemy.name << " оглушён!]\n";
  }

  std::cout << kSeparator << "\n";
  std::cout << "Действия:\n";
  std::cout << "  [атака]   — удар тетрадью\n";
  std::cout << "  [эффект]  — использовать эффект (доп.урон / дебафф / стан)\n";

  if (CanUseCombo(state.player)) {
    std::cout << "  [комбо]   — комбо-удар всеми тетрадями\n";
  }

  if (!state.player.inventory.empty()) {
    std::cout << "  [инвентарь] — использовать предмет\n";
  }

  std::cout << kSeparator << "\n";
}

// ---------------------------------------------------------------------------
// Диалог
// ---------------------------------------------------------------------------

void RenderDialogue(const std::string& npc_name,
                    const std::vector<std::string>& lines) {
  std::cout << "\n" << kSeparatorB << "\n";
  std::cout << npc_name << ":\n";
  std::cout << kSeparator << "\n";
  for (const auto& line : lines) {
    std::cout << "  \"" << line << "\"\n";
  }
  std::cout << kSeparatorB << "\n";
  std::cout << "[Нажмите Enter чтобы продолжить]\n";
  std::cin.ignore();
}

// ---------------------------------------------------------------------------
// Ловушка
// ---------------------------------------------------------------------------

void RenderTrap(const TrapData& trap) {
  std::cout << "\n" << kSeparatorB << "\n";
  std::cout << "ЛОВУШКА: " << trap.name << "\n";
  std::cout << kSeparatorB << "\n";
  std::cout << trap.description << "\n";
  std::cout << kSeparator << "\n";
  std::cout << "Выберите действие:\n";

  int idx = 1;
  for (const auto& opt : trap.options) {
    std::cout << "  " << idx++ << ". " << opt.description << "\n";
  }
  std::cout << kSeparator << "\n";
}

// ---------------------------------------------------------------------------
// Системные сообщения
// ---------------------------------------------------------------------------

void RenderMessage(const std::string& msg) {
  std::cout << msg << "\n";
}

void RenderVictory(const Player& player) {
  std::cout << "\n" << kSeparatorB << "\n";
  std::cout << kSeparatorB << "\n";
  std::cout << "  ПОБЕДА!\n";
  std::cout << kSeparatorB << "\n";
  std::cout << "  " << player.name << " собрал все 7 тетрадей!\n";
  std::cout << "  Теперь ему точно покорится выпускной экзамен.\n";
  std::cout << kSeparatorB << "\n";
  std::cout << kSeparatorB << "\n";
}

void RenderGameOver(const Player& player) {
  std::cout << "\n" << kSeparatorB << "\n";
  std::cout << "  GAME OVER\n";
  std::cout << kSeparatorB << "\n";
  std::cout << "  " << player.name << " потерял сознание в коридоре.\n";
  std::cout << "  Тетради собраны: " << player.notebook_ids.size() << "/7\n";
  std::cout << kSeparatorB << "\n";
}

void RenderIntro() {
  std::cout << kSeparatorB << "\n";
  std::cout << kSeparatorB << "\n";
  std::cout << "         ЗЛЫЕ УМНИКИ\n";
  std::cout << kSeparatorB << "\n";
  std::cout << "  Ты — Рэн, школьник выпускного класса.\n";
  std::cout << "  Впереди экзамены, а ты не знаешь ничего.\n";
  std::cout << "  Но под столом в столовой ты нашёл пророчество:\n";
  std::cout << "  семь Злых Умников владеют тетрадями знаний.\n";
  std::cout << "  Найди их. Победи. Забери тетради.\n";
  std::cout << kSeparatorB << "\n";
  std::cout << kSeparatorB << "\n";
  std::cout << "[Нажмите Enter чтобы начать]\n";
  std::cin.ignore();
}

// ---------------------------------------------------------------------------
// Получение ввода от игрока во время боя
// ---------------------------------------------------------------------------

Command GetCombatCommand(const Player& player) {
  (void)player;
  std::cout << "> ";
  std::string line = ReadLine();
  return ParseCommand(line);
}

EffectType GetEffectChoice() {
  std::cout << "\n  1. Дополнительный урон\n";
  std::cout << "  2. Дебафф (снизить атаку врага)\n";
  std::cout << "  3. Стан (враг пропускает ход)\n";

  while (true) {
    std::cout << "Выбери эффект (1-3): ";
    std::string line = ReadLine();
    auto effect = ParseEffect(line);
    if (effect.has_value()) return effect.value();
    std::cout << "Неверный ввод. Введи 1, 2 или 3.\n";
  }
}

std::string GetItemChoice(const Player& player) {
  if (player.inventory.empty()) {
    std::cout << "Инвентарь пуст.\n";
    return "";
  }

  int idx = 1;
  for (const auto& item : player.inventory) {
    std::cout << "  " << idx++ << ". " << item.name << "\n";
  }
  std::cout << "  0. Отмена\n";

  while (true) {
    std::cout << "Выбери предмет: ";
    std::string line = ReadLine();
    auto num = ParseInt(line);

    if (!num.has_value()) {
      std::cout << "Введи номер предмета.\n";
      continue;
    }

    int choice = num.value();
    if (choice == 0) return "";

    if (choice >= 1 && choice <= static_cast<int>(player.inventory.size())) {
      return player.inventory[choice - 1].id;
    }

    std::cout << "Неверный номер. Попробуй ещё раз.\n";
  }
}
