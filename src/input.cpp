#include "input.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>

// ---------------------------------------------------------------------------
// Вспомогательная функция — нормализация строки
// ---------------------------------------------------------------------------

std::string Normalize(const std::string& input) {
  std::string result = input;

  // Обрезаем пробелы слева
  auto left = std::find_if(result.begin(), result.end(),
                           [](unsigned char c) { return !std::isspace(c); });
  result.erase(result.begin(), left);

  // Обрезаем пробелы справа
  auto right = std::find_if(result.rbegin(), result.rend(),
                            [](unsigned char c) { return !std::isspace(c); });
  result.erase(right.base(), result.end());

  // Приводим к нижнему регистру
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  return result;
}

// ---------------------------------------------------------------------------
// Чтение ввода
// ---------------------------------------------------------------------------

std::string ReadLine() {
  std::string line;
  std::getline(std::cin, line);
  return line;
}

// ---------------------------------------------------------------------------
// Парсинг команд главного игрового цикла
// ---------------------------------------------------------------------------

Command ParseCommand(const std::string& input) {
  const std::string s = Normalize(input);

  // Движение — полные слова и сокращения
  if (s == "север"   || s == "с"  || s == "north" || s == "n") return Command::kMoveNorth;
  if (s == "юг"      || s == "ю"  || s == "south" || s == "s") return Command::kMoveSouth;
  if (s == "восток"  || s == "в"  || s == "east"  || s == "e") return Command::kMoveEast;
  if (s == "запад"   || s == "з"  || s == "west"  || s == "w") return Command::kMoveWest;

  // Осмотреться
  if (s == "осмотреться" || s == "осмотрись" || s == "look" || s == "l")
    return Command::kLookAround;

  // Инвентарь
  if (s == "инвентарь" || s == "и" || s == "inventory" || s == "i")
    return Command::kOpenInventory;

  // Боевые команды
  if (s == "атака"  || s == "атаковать" || s == "attack" || s == "a")
    return Command::kAttack;
  if (s == "эффект" || s == "effect"    || s == "eff"    || s == "э")
    return Command::kUseEffect;
  if (s == "комбо"  || s == "combo"     || s == "c"      || s == "к")
    return Command::kUseCombo;

  return Command::kUnknown;
}

// ---------------------------------------------------------------------------
// Парсинг направления
// ---------------------------------------------------------------------------

std::optional<Direction> ParseDirection(const std::string& input) {
  const std::string s = Normalize(input);

  if (s == "север"  || s == "с" || s == "north" || s == "n") return Direction::kNorth;
  if (s == "юг"     || s == "ю" || s == "south" || s == "s") return Direction::kSouth;
  if (s == "восток" || s == "в" || s == "east"  || s == "e") return Direction::kEast;
  if (s == "запад"  || s == "з" || s == "west"  || s == "w") return Direction::kWest;

  return std::nullopt;
}

// ---------------------------------------------------------------------------
// Парсинг выбора эффекта
// ---------------------------------------------------------------------------

std::optional<EffectType> ParseEffect(const std::string& input) {
  const std::string s = Normalize(input);

  if (s == "1" || s == "урон"    || s == "доп урон") return EffectType::kExtraDamage;
  if (s == "2" || s == "дебафф"  || s == "debuff")   return EffectType::kDebuff;
  if (s == "3" || s == "оглушить"|| s == "стан")      return EffectType::kStun;

  return std::nullopt;
}

// ---------------------------------------------------------------------------
// Парсинг числа
// ---------------------------------------------------------------------------

std::optional<int> ParseInt(const std::string& input) {
  const std::string s = Normalize(input);
  if (s.empty()) return std::nullopt;

  try {
    size_t pos = 0;
    int value = std::stoi(s, &pos);
    if (pos != s.size()) return std::nullopt;  // не всё число (например "1abc")
    return value;
  } catch (const std::exception&) {
    return std::nullopt;
  }
}
