#ifndef EVIL_NERDS_RENDERER_H_
#define EVIL_NERDS_RENDERER_H_

#include <map>
#include <string>
#include <vector>

#include "types.h"

// ---------------------------------------------------------------------------
// Renderer — весь вывод в консоль и получение ввода от игрока.
//
// Никакой игровой логики здесь нет — только отображение состояния
// и чтение выбора пользователя.
// ---------------------------------------------------------------------------

// --- Вывод состояния мира ---

// Короткое описание локации + список выходов + объекты
void RenderLocation(const Location& loc);

// Полное описание локации (команда "осмотреться")
void RenderLocationFull(const Location& loc);

// --- Вывод инвентаря ---

void RenderInventory(
    const Player& player,
    const std::map<std::string, Notebook>& notebooks);

// --- Вывод боя ---

void RenderCombat(const CombatState& state);

// --- Вывод диалога ---

void RenderDialogue(const std::string& npc_name,
                    const std::vector<std::string>& lines);

// --- Вывод ловушки ---

void RenderTrap(const TrapData& trap);

// --- Системные сообщения ---

void RenderMessage(const std::string& msg);
void RenderVictory(const Player& player);
void RenderGameOver(const Player& player);
void RenderIntro();

// --- Получение ввода от игрока во время боя ---

// Показывает меню боя и читает команду
Command GetCombatCommand(const Player& player);

// Показывает меню эффектов и читает выбор
EffectType GetEffectChoice();

// Показывает инвентарь и читает выбор предмета.
// Возвращает id предмета или "" если игрок отменил выбор.
std::string GetItemChoice(const Player& player);

#endif  // EVIL_NERDS_RENDERER_H_
