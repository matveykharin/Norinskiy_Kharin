#ifndef EVIL_NERDS_TYPES_H_
#define EVIL_NERDS_TYPES_H_

#include <string>
#include <vector>
#include <map>
#include <optional>

// ---------------------------------------------------------------------------
// Enums
// ---------------------------------------------------------------------------

enum class Direction {
  kNorth,
  kSouth,
  kEast,
  kWest
};

enum class Command {
  kMoveNorth,
  kMoveSouth,
  kMoveEast,
  kMoveWest,
  kLookAround,
  kOpenInventory,
  kAttack,
  kUseEffect,
  kUseCombo,
  kUnknown
};

enum class ObjectType {
  kEnemy,
  kTrap,
  kChest,
  kNpc
};

enum class EffectType {
  kExtraDamage,   // Дополнительный урон
  kDebuff,        // Снижение атаки врага
  kStun           // Враг пропускает ход
};

enum class ItemType {
  kHealthPotion,
  kConsumable
};

// ---------------------------------------------------------------------------
// Stats — общие характеристики юнита
// ---------------------------------------------------------------------------

struct Stats {
  int hp;
  int max_hp;
  int attack;
  int accuracy;   // шанс попасть (0–100)
  int dodge;      // шанс уклониться (0–100)
};

// ---------------------------------------------------------------------------
// Item — предмет в инвентаре
// ---------------------------------------------------------------------------

struct Item {
  std::string id;
  std::string name;
  std::string description;
  ItemType type;
  int value;      // сколько HP восстанавливает (для зелья) или бонус урона
};

// ---------------------------------------------------------------------------
// Notebook — тетрадь одного из Умников
// ---------------------------------------------------------------------------

struct Notebook {
  std::string id;
  std::string name;       // название предмета ("Тетрадь по математике")
  std::string subject;    // школьный предмет
  int base_damage;        // урон обычной атакой этой тетрадью
  std::string combo_effect_description;  // описание комбо-эффекта
};

// ---------------------------------------------------------------------------
// Специфичные данные объектов локации
// ---------------------------------------------------------------------------

struct EnemyData {
  std::string id;
  std::string name;
  Stats stats;
  std::string notebook_id;   // тетрадь, которую охраняет
  std::string meet_dialogue; // фраза при встрече
  bool is_boss;
};

// Один вариант выбора в ловушке
struct TrapOption {
  std::string description;   // текст варианта
  int success_damage;        // урон при успехе (0 = нет урона)
  int fail_damage;           // урон при провале
  int success_chance;        // шанс успеха (0–100)
};

struct TrapData {
  std::string id;
  std::string name;
  std::string description;
  std::vector<TrapOption> options;  // 3 варианта выбора
  bool is_triggered;
};

struct ChestData {
  std::string id;
  std::vector<std::string> item_ids;
  bool is_opened;
};

struct NpcData {
  std::string id;
  std::string name;
  std::string dialogue_id;   // ключ к диалогу в dialogues.json
};

// ---------------------------------------------------------------------------
// GameObject — объект в локации
// ---------------------------------------------------------------------------

struct GameObject {
  std::string id;
  ObjectType type;

  // Только одно из полей используется в зависимости от type
  std::optional<EnemyData> enemy;
  std::optional<TrapData>  trap;
  std::optional<ChestData> chest;
  std::optional<NpcData>   npc;
};

// ---------------------------------------------------------------------------
// Location — одна комната/коридор
// ---------------------------------------------------------------------------

struct Location {
  std::string id;
  std::string name;
  std::string description;          // короткое описание
  std::string full_description;     // при команде "осмотреться"
  std::map<Direction, std::string> exits;  // Direction → id соседней локации
  std::vector<GameObject> objects;
};

// ---------------------------------------------------------------------------
// Player
// ---------------------------------------------------------------------------

struct Player {
  std::string name;
  Stats stats;
  std::vector<std::string> notebook_ids;  // собранные тетради (по id)
  std::vector<Item> inventory;
  bool stun_next_turn;                    // эффект оглушения
};

// ---------------------------------------------------------------------------
// World
// ---------------------------------------------------------------------------

struct World {
  std::map<std::string, Location> locations;  // id → Location
  std::string current_location_id;
};

// ---------------------------------------------------------------------------
// Combat
// ---------------------------------------------------------------------------

enum class CombatResult {
  kPlayerWon,
  kPlayerLost,
  kInProgress
};

struct CombatState {
  Player& player;
  EnemyData enemy;
  bool enemy_stunned;
  CombatResult result;

  CombatState(Player& p, EnemyData e)
      : player(p),
        enemy(std::move(e)),
        enemy_stunned(false),
        result(CombatResult::kInProgress) {}
};

#endif  // EVIL_NERDS_TYPES_H_
