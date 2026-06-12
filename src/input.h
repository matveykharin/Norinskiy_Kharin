#ifndef EVIL_NERDS_INPUT_H_
#define EVIL_NERDS_INPUT_H_

#include <optional>
#include <string>

#include "types.h"

// ---------------------------------------------------------------------------
// Input — единственное место в проекте где читается ввод пользователя
// и строки преобразуются в типы игры.
//
// Все функции сначала приводят строку к нижнему регистру и обрезают пробелы,
// затем сравнивают — поэтому "North", "NORTH", "north" работают одинаково.
// ---------------------------------------------------------------------------

// Читает строку из stdin и возвращает её
std::string ReadLine();

// Преобразует строку в команду главного цикла
Command ParseCommand(const std::string& input);

// Преобразует строку в направление движения.
// Возвращает nullopt если строка не распознана.
std::optional<Direction> ParseDirection(const std::string& input);

// Преобразует строку "1"/"2"/"3" в EffectType.
// Возвращает nullopt если строка не распознана.
std::optional<EffectType> ParseEffect(const std::string& input);

// Преобразует строку в число.
// Возвращает nullopt если строка не является числом.
std::optional<int> ParseInt(const std::string& input);

// Приводит строку к нижнему регистру и обрезает пробелы по краям
std::string Normalize(const std::string& input);

#endif  // EVIL_NERDS_INPUT_H_
