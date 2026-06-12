#include "loader.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "../include/nlohmann/json.hpp"

using json = nlohmann::json;

// ---------------------------------------------------------------------------
// Вспомогательные функции парсинга — единственное место в проекте,
// где строки сравниваются для определения типов
// ---------------------------------------------------------------------------

static Direction ParseDirection(const std::string& s) {
  if (s == "north") return Direction::kNorth;
  if (s == "south") return Direction::kSouth;
  if (s == "east")  return Direction::kEast;
  if (s == "west")  return Direction::kWest;
  if (s == "up")    return Direction::kNorth;  // вверх = север для лестниц
  if (s == "down")  return Direction::kSouth;  // вниз  = юг  для лестниц
  throw std::runtime_error("Unknown direction: " + s);
}

static ObjectType ParseObjectType(const std::string& s) {
  if (s == "enemy") return ObjectType::kEnemy;
  if (s == "trap")  return ObjectType::kTrap;
  if (s == "chest") return ObjectType::kChest;
  if (s == "npc")   return ObjectType::kNpc;
  throw std::runtime_error("Unknown object type: " + s);
}

static ItemType ParseItemType(const std::string& s) {
  if (s == "health_potion") return ItemType::kHealthPotion;
  if (s == "consumable")    return ItemType::kConsumable;
  throw std::runtime_error("Unknown item type: " + s);
}

// ---------------------------------------------------------------------------
// Открыть JSON-файл и вернуть распарсенный объект
// ---------------------------------------------------------------------------

static json OpenJson(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file: " + path);
  }
  return json::parse(file);
}

// ---------------------------------------------------------------------------
// Парсинг отдельных структур из JSON-узлов
// ---------------------------------------------------------------------------

static Stats ParseStats(const json& j) {
  return Stats{
    .hp       = j.at("hp").get<int>(),
    .max_hp   = j.at("max_hp").get<int>(),
    .attack   = j.at("attack").get<int>(),
    .accuracy = j.at("accuracy").get<int>(),
    .dodge    = j.at("dodge").get<int>()
  };
}

static EnemyData ParseEnemy(const json& j) {
  EnemyData e;
  e.id            = j.at("id").get<std::string>();
  e.name          = j.at("name").get<std::string>();
  e.stats         = ParseStats(j.at("stats"));
  e.notebook_id   = j.at("notebook_id").get<std::string>();
  e.meet_dialogue = j.at("meet_dialogue").get<std::string>();
  e.is_boss       = j.at("is_boss").get<bool>();
  return e;
}

static TrapData ParseTrap(const json& j) {
  TrapData t;
  t.id           = j.at("id").get<std::string>();
  t.name         = j.at("name").get<std::string>();
  t.description  = j.at("description").get<std::string>();
  t.is_triggered = j.at("is_triggered").get<bool>();

  if (j.contains("options")) {
    for (const auto& opt : j.at("options")) {
      TrapOption o;
      o.description    = opt.at("description").get<std::string>();
      o.success_chance = opt.at("success_chance").get<int>();
      o.success_damage = opt.at("success_damage").get<int>();
      o.fail_damage    = opt.at("fail_damage").get<int>();
      t.options.push_back(std::move(o));
    }
  }
  return t;
}

static ChestData ParseChest(const json& j) {
  ChestData c;
  c.id        = j.at("id").get<std::string>();
  c.is_opened = j.at("is_opened").get<bool>();
  for (const auto& item_id : j.at("item_ids")) {
    c.item_ids.push_back(item_id.get<std::string>());
  }
  return c;
}

static NpcData ParseNpc(const json& j) {
  NpcData n;
  n.id          = j.at("id").get<std::string>();
  n.name        = j.at("name").get<std::string>();
  n.dialogue_id = j.at("dialogue_id").get<std::string>();
  return n;
}

static GameObject ParseGameObject(const json& j) {
  GameObject obj;
  obj.id   = j.at("id").get<std::string>();
  obj.type = ParseObjectType(j.at("type").get<std::string>());

  switch (obj.type) {
    case ObjectType::kEnemy:
      obj.enemy = ParseEnemy(j.at("enemy"));
      break;
    case ObjectType::kTrap:
      obj.trap = ParseTrap(j.at("trap"));
      break;
    case ObjectType::kChest:
      obj.chest = ParseChest(j.at("chest"));
      break;
    case ObjectType::kNpc:
      obj.npc = ParseNpc(j.at("npc"));
      break;
  }
  return obj;
}

static Location ParseLocation(const json& j) {
  Location loc;
  loc.id               = j.at("id").get<std::string>();
  loc.name             = j.at("name").get<std::string>();
  loc.description      = j.at("description").get<std::string>();
  loc.full_description = j.at("full_description").get<std::string>();

  for (const auto& [dir_str, target_id] : j.at("exits").items()) {
    loc.exits[ParseDirection(dir_str)] = target_id.get<std::string>();
  }

  for (const auto& obj_json : j.at("objects")) {
    loc.objects.push_back(ParseGameObject(obj_json));
  }
  return loc;
}

// ---------------------------------------------------------------------------
// Публичные функции загрузки
// ---------------------------------------------------------------------------

Player LoadPlayer(const std::string& assets_dir) {
  json j = OpenJson(assets_dir + "/player.json");

  Player p;
  p.name           = j.at("name").get<std::string>();
  p.stats          = ParseStats(j.at("stats"));
  p.stun_next_turn = false;

  for (const auto& nb_id : j.at("notebook_ids")) {
    p.notebook_ids.push_back(nb_id.get<std::string>());
  }
  return p;
}

World LoadWorld(const std::string& assets_dir) {
  json j = OpenJson(assets_dir + "/locations.json");

  World world;
  for (const auto& loc_json : j.at("locations")) {
    Location loc = ParseLocation(loc_json);
    std::string id = loc.id;
    world.locations[id] = std::move(loc);
  }
  world.current_location_id = "canteen";  // стартовая локация
  return world;
}

std::map<std::string, Notebook> LoadNotebooks(const std::string& assets_dir) {
  json j = OpenJson(assets_dir + "/notebooks.json");

  std::map<std::string, Notebook> notebooks;
  for (const auto& nb_json : j.at("notebooks")) {
    Notebook nb;
    nb.id                       = nb_json.at("id").get<std::string>();
    nb.name                     = nb_json.at("name").get<std::string>();
    nb.subject                  = nb_json.at("subject").get<std::string>();
    nb.base_damage              = nb_json.at("base_damage").get<int>();
    nb.combo_effect_description = nb_json.at("combo_effect_description").get<std::string>();
    notebooks[nb.id]            = std::move(nb);
  }
  return notebooks;
}

std::map<std::string, Item> LoadItems(const std::string& assets_dir) {
  json j = OpenJson(assets_dir + "/items.json");

  std::map<std::string, Item> items;
  for (const auto& item_json : j.at("items")) {
    Item item;
    item.id          = item_json.at("id").get<std::string>();
    item.name        = item_json.at("name").get<std::string>();
    item.description = item_json.at("description").get<std::string>();
    item.type        = ParseItemType(item_json.at("type").get<std::string>());
    item.value       = item_json.at("value").get<int>();
    items[item.id]   = std::move(item);
  }
  return items;
}

std::map<std::string, std::vector<std::string>> LoadDialogues(
    const std::string& assets_dir) {
  json j = OpenJson(assets_dir + "/dialogues.json");

  std::map<std::string, std::vector<std::string>> dialogues;
  for (const auto& dlg_json : j.at("dialogues")) {
    std::string id = dlg_json.at("id").get<std::string>();
    std::vector<std::string> lines;
    for (const auto& line : dlg_json.at("lines")) {
      lines.push_back(line.get<std::string>());
    }
    dialogues[id] = std::move(lines);
  }
  return dialogues;
}
