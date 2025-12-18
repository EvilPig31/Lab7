#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include "npc.h"
#include "visitor.h"
#include "observer.h"

class GameEngine {
private:
    static constexpr double MAP_MIN_X = 0.0;
    static constexpr double MAP_MAX_X = 100.0;
    static constexpr double MAP_MIN_Y = 0.0;
    static constexpr double MAP_MAX_Y = 100.0;
    static constexpr int GAME_DURATION = 30;
    static constexpr int NPC_COUNT = 50;
    static constexpr int DISPLAY_INTERVAL = 1;
    
    std::vector<std::shared_ptr<NPC>> npcs;
    BattleQueue battleQueue;
    BattleLogger battleLogger;
    
    std::thread movementThread;
    std::thread battleThread;
    std::thread displayThread;
    
    std::atomic<bool> gameRunning;
    std::atomic<int> elapsedTime;
    
    mutable std::mutex coutMutex;
    
public:
    GameEngine();
    ~GameEngine();
    
    void initializeGame();
    void run();
    void stop();
    
private:
    void movementWorker();
    void battleWorker();
    void displayWorker();
    void processBattle(const BattleTask& task);
    void printMap() const;
    void printSurvivors() const;
    void createRandomNPCs();
    template<typename T>
    void safePrint(const T& message) const;
};

#endif