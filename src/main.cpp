#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <string>

#include "combat.h"
#include "input.h"
#include "loader.h"
#include "player.h"
#include "renderer.h"
#include "types.h"
#include "world.h"

// ---------------------------------------------------------------------------
// Обработка события в локации после перемещения
// ---------------------------------------------------------------------------

static void HandleLocationEvent(
    World& world,
    Player& player,
    const std::map<std::string, Notebook>& notebooks,
    const std::map<std::string, Item>& all_items) {

  Location& loc = GetCurrentLocation(world);

  // --- Враг ---
  GameObject* enemy_obj = FindObject(loc, ObjectType::kEnemy);
  if (enemy_obj != nullptr && enemy_obj->enemy.has_value()) {
    EnemyData enemy = enemy_obj->enemy.value();
    RenderMessage("\n" + enemy.name + " преграждает путь!");

    CombatResult result = RunCombat(player, std::move(enemy), notebooks);

    if (result == CombatResult::kPlayerWon) {
      // Забираем тетрадь если есть
      const std::string& nb_id = enemy_obj->enemy->notebook_id;
      if (!nb_id.empty()) {
        auto nb_it = notebooks.find(nb_id);
        if (nb_it != notebooks.end()) {
          AddNotebook(player, nb_id);
          RenderMessage("Ты забрал: " + nb_it->second.name);
          RenderMessage("Тетрадей собрано: " +
                        std::to_string(player.notebook_ids.size()) + "/7");
        }
      }
      RemoveObject(loc, enemy_obj->id);
    } else {
      RenderGameOver(player);
    }
    return;
  }

  // --- Ловушка ---
  GameObject* trap_obj = FindObject(loc, ObjectType::kTrap);
  if (trap_obj != nullptr && trap_obj->trap.has_value() &&
      !trap_obj->trap->is_triggered) {

    // Загружаем полные данные ловушки из traps.json по id
    // (в locations.json хранятся только базовые поля без options)
    // Здесь используем данные прямо из объекта — options уже загружены
    TrapData& trap = trap_obj->trap.value();
    RenderTrap(trap);

    if (trap.options.empty()) {
      trap.is_triggered = true;
      return;
    }

    int choice = 0;
    while (choice < 1 || choice > static_cast<int>(trap.options.size())) {
      std::cout << "Твой выбор: ";
      std::string line = ReadLine();
      auto num = ParseInt(line);
      if (num.has_value()) choice = num.value();
      if (choice < 1 || choice > static_cast<int>(trap.options.size())) {
        RenderMessage("Введи число от 1 до " +
                      std::to_string(trap.options.size()));
      }
    }

    const TrapOption& opt = trap.options[choice - 1];
    bool success = (std::rand() % 100) < opt.success_chance;

    if (success) {
      RenderMessage("Успех! Ты прошёл испытание без потерь.");
      if (opt.success_damage > 0) {
        DamagePlayer(player, opt.success_damage);
        RenderMessage("Но всё же получил урон: " +
                      std::to_string(opt.success_damage));
      }
    } else {
      DamagePlayer(player, opt.fail_damage);
      RenderMessage("Неудача! Ты получил урон: " +
                    std::to_string(opt.fail_damage));
    }

    RenderMessage("HP: " + std::to_string(player.stats.hp) + "/" +
                  std::to_string(player.stats.max_hp));
    trap.is_triggered = true;
    return;
  }

  // --- Сундук ---
  GameObject* chest_obj = FindObject(loc, ObjectType::kChest);
  if (chest_obj != nullptr && chest_obj->chest.has_value() &&
      !chest_obj->chest->is_opened) {

    ChestData& chest = chest_obj->chest.value();
    RenderMessage("\nТы нашёл сундук!");

    for (const auto& item_id : chest.item_ids) {
      auto it = all_items.find(item_id);
      if (it != all_items.end()) {
        bool added = AddItem(player, it->second);
        if (added) {
          RenderMessage("  Получен предмет: " + it->second.name);
        }
      }
    }
    chest.is_opened = true;
    return;
  }
}

// ---------------------------------------------------------------------------
// Обработка команды осмотреться — показывает NPC и сундуки
// ---------------------------------------------------------------------------

static void HandleLookAround(
    World& world,
    Player& player,
    const std::map<std::string, Item>& all_items,
    const std::map<std::string, std::vector<std::string>>& dialogues) {

  Location& loc = GetCurrentLocation(world);
  RenderLocationFull(loc);

  // NPC
  GameObject* npc_obj = FindObject(loc, ObjectType::kNpc);
  if (npc_obj != nullptr && npc_obj->npc.has_value()) {
    const NpcData& npc = npc_obj->npc.value();
    auto it = dialogues.find(npc.dialogue_id);
    if (it != dialogues.end()) {
      RenderDialogue(npc.name, it->second);
    }
    return;
  }

  // Открытый сундук — повторный осмотр
  GameObject* chest_obj = FindObject(loc, ObjectType::kChest);
  if (chest_obj != nullptr && chest_obj->chest.has_value() &&
      chest_obj->chest->is_opened) {
    RenderMessage("Сундук уже открыт и пуст.");
    return;
  }

  (void)player;
  (void)all_items;
}

// ---------------------------------------------------------------------------
// Главный игровой цикл
// ---------------------------------------------------------------------------

int main() {
  std::srand(static_cast<unsigned>(std::time(nullptr)));

  // Загрузка всех данных
  const std::string kAssetsDir = "assets";
  Player player       = LoadPlayer(kAssetsDir);
  World world         = LoadWorld(kAssetsDir);
  auto notebooks      = LoadNotebooks(kAssetsDir);
  auto items          = LoadItems(kAssetsDir);
  auto dialogues      = LoadDialogues(kAssetsDir);

  RenderIntro();
  RenderLocation(GetCurrentLocation(world));

  // Проверяем события в стартовой локации
  HandleLocationEvent(world, player, notebooks, items);

  bool running = true;
  while (running && IsPlayerAlive(player)) {

    // Проверка победы
    if (AllNotebooksCollected(player)) {
      RenderVictory(player);
      break;
    }

    std::cout << "\n> ";
    std::string input = ReadLine();
    Command cmd = ParseCommand(input);

    switch (cmd) {
      case Command::kMoveNorth:
      case Command::kMoveSouth:
      case Command::kMoveEast:
      case Command::kMoveWest: {
        Direction dir;
        if      (cmd == Command::kMoveNorth) dir = Direction::kNorth;
        else if (cmd == Command::kMoveSouth) dir = Direction::kSouth;
        else if (cmd == Command::kMoveEast)  dir = Direction::kEast;
        else                                  dir = Direction::kWest;

        bool moved = MovePlayer(world, dir);
        if (moved) {
          RenderLocation(GetCurrentLocation(world));
          HandleLocationEvent(world, player, notebooks, items);
        } else {
          RenderMessage("В этом направлении нет выхода.");
        }
        break;
      }

      case Command::kLookAround:
        HandleLookAround(world, player, items, dialogues);
        break;

      case Command::kOpenInventory:
        RenderInventory(player, notebooks);
        break;

      // Боевые команды вне боя — подсказка
      case Command::kAttack:
      case Command::kUseEffect:
      case Command::kUseCombo:
        RenderMessage("Здесь не с кем сражаться.");
        break;

      case Command::kUnknown:
      default:
        RenderMessage("Неизвестная команда. Попробуй: север, юг, восток, запад, осмотреться, инвентарь.");
        break;
    }

    // Проверка смерти после ловушки
    if (!IsPlayerAlive(player)) {
      RenderGameOver(player);
      running = false;
    }
  }

  return 0;
}
