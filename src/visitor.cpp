#include "../include/visitor.h"
#include "../include/npc.h"
#include "../include/observer.h"
#include <iostream>
#include <algorithm>
#include <chrono>

void BattleQueue::addTask(const BattleTask& task) {
    std::lock_guard<std::mutex> lock(mtx);
    tasks.push(task);
    cv.notify_one();
}

bool BattleQueue::tryGetTask(BattleTask& task) {
    std::unique_lock<std::mutex> lock(mtx);
    
    cv.wait_for(lock, std::chrono::milliseconds(100), 
                [this]() { return !tasks.empty() || stopFlag; });
    
    if (stopFlag && tasks.empty()) {
        return false;
    }
    
    if (!tasks.empty()) {
        task = tasks.front();
        tasks.pop();
        return true;
    }
    
    return false;
}

void BattleQueue::stop() {
    std::lock_guard<std::mutex> lock(mtx);
    stopFlag = true;
    cv.notify_all();
}

bool BattleQueue::isEmpty() const {
    std::lock_guard<std::mutex> lock(mtx);
    return tasks.empty();
}

bool BattleQueue::shouldStop() const {
    std::lock_guard<std::mutex> lock(mtx);
    return stopFlag && tasks.empty();
}

size_t BattleQueue::size() const {
    std::lock_guard<std::mutex> lock(mtx);
    return tasks.size();
}
void DetectionVisitor::detectForNPC(NPC* npc) {
    if (!npc->isAlive()) return;
    std::vector<std::shared_ptr<NPC>> aliveTargets;
    {
        for (auto& target : npcs) {
            if (target && target != currentNPC && target->isAlive()) {
                aliveTargets.push_back(target);
            }
        }
    }
    
    for (auto& target : aliveTargets) {
        double distance = npc->calculateDistance(target.get());
        if (distance <= npc->getAttackDistance()) {
            if (npc->canAttack(target.get())) {
                battleQueue.addTask(BattleTask(currentNPC, target));
            }
        }
    }
}

DetectionVisitor::DetectionVisitor(std::vector<std::shared_ptr<NPC>>& npcs, BattleQueue& queue, std::shared_ptr<NPC> npc)
    : npcs(npcs), battleQueue(queue), currentNPC(npc) {}

void DetectionVisitor::visit(Squirrel* squirrel) {
    detectForNPC(squirrel);
}

void DetectionVisitor::visit(Werewolf* werewolf) {
    detectForNPC(werewolf);
}

void DetectionVisitor::visit(Druid* druid) {
}

void DetectionVisitor::detectBattles() {
    currentNPC->accept(*this);
}