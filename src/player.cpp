#include "player.h"

#include <algorithm>

// Количество тетрадей для победы
static constexpr int kTotalNotebooks = 7;

// ---------------------------------------------------------------------------
// Тетради
// ---------------------------------------------------------------------------

void AddNotebook(Player& player, const std::string& notebook_id) {
  if (!HasNotebook(player, notebook_id)) {
    player.notebook_ids.push_back(notebook_id);
  }
}

bool HasNotebook(const Player& player, const std::string& notebook_id) {
  for (const auto& id : player.notebook_ids) {
    if (id == notebook_id) return true;
  }
  return false;
}

bool AllNotebooksCollected(const Player& player) {
  return static_cast<int>(player.notebook_ids.size()) >= kTotalNotebooks;
}

int GetTotalNotebookDamage(
    const Player& player,
    const std::map<std::string, Notebook>& notebooks) {
  int total = 0;
  for (const auto& id : player.notebook_ids) {
    auto it = notebooks.find(id);
    if (it != notebooks.end()) {
      total += it->second.base_damage;
    }
  }
  return total;
}

// ---------------------------------------------------------------------------
// Инвентарь
// ---------------------------------------------------------------------------

bool AddItem(Player& player, const Item& item) {
  if (HasItem(player, item.id)) return false;
  player.inventory.push_back(item);
  return true;
}

void RemoveItem(Player& player, const std::string& item_id) {
  auto it = std::remove_if(player.inventory.begin(), player.inventory.end(),
                           [&item_id](const Item& item) {
                             return item.id == item_id;
                           });
  player.inventory.erase(it, player.inventory.end());
}

bool UseItem(Player& player, const std::string& item_id) {
  for (const auto& item : player.inventory) {
    if (item.id == item_id) {
      switch (item.type) {
        case ItemType::kHealthPotion:
          HealPlayer(player, item.value);
          break;
        case ItemType::kConsumable:
          // Бонус точности/уклонения применяется в combat.cpp
          // здесь только удаляем предмет
          break;
      }
      RemoveItem(player, item_id);
      return true;
    }
  }
  return false;
}

bool HasItem(const Player& player, const std::string& item_id) {
  for (const auto& item : player.inventory) {
    if (item.id == item_id) return true;
  }
  return false;
}

// ---------------------------------------------------------------------------
// Здоровье
// ---------------------------------------------------------------------------

void HealPlayer(Player& player, int amount) {
  player.stats.hp += amount;
  if (player.stats.hp > player.stats.max_hp) {
    player.stats.hp = player.stats.max_hp;
  }
}

void DamagePlayer(Player& player, int amount) {
  player.stats.hp -= amount;
  if (player.stats.hp < 0) {
    player.stats.hp = 0;
  }
}

bool IsPlayerAlive(const Player& player) {
  return player.stats.hp > 0;
}

// ---------------------------------------------------------------------------
// Комбо
// ---------------------------------------------------------------------------

bool CanUseCombo(const Player& player) {
  return player.notebook_ids.size() >= 2;
}
