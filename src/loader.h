#ifndef EVIL_NERDS_LOADER_H_
#define EVIL_NERDS_LOADER_H_

#include <map>
#include <string>
#include <vector>

#include "types.h"

// ---------------------------------------------------------------------------
// Loader — загрузка всех игровых данных из JSON-файлов в assets/
//
// Все функции принимают путь к папке assets и возвращают данные.
// Строки сравниваются только здесь при разборе type/direction из JSON.
// ---------------------------------------------------------------------------

// Загружает стартовые данные игрока
Player LoadPlayer(const std::string& assets_dir);

// Загружает все локации мира
World LoadWorld(const std::string& assets_dir);

// Загружает все тетради по id
std::map<std::string, Notebook> LoadNotebooks(const std::string& assets_dir);

// Загружает все предметы по id
std::map<std::string, Item> LoadItems(const std::string& assets_dir);

// Загружает все диалоги по id
std::map<std::string, std::vector<std::string>> LoadDialogues(
    const std::string& assets_dir);

#endif  // EVIL_NERDS_LOADER_H_
