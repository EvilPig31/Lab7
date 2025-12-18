#include "../include/game_engine.h"
#include "../include/npc_factory.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <algorithm>
#include <sstream>

GameEngine::GameEngine() 
    : gameRunning(false), elapsedTime(0) {
    
    battleLogger.attach(new ConsoleLogger());
    battleLogger.attach(new FileLogger("game_log.txt"));
}

GameEngine::~GameEngine() {
    stop();
}
template<typename T>
void GameEngine::safePrint(const T& message) const {
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << message;
}

void GameEngine::initializeGame() {
    safePrint("Initializing game with " + std::to_string(NPC_COUNT) + " NPCs...\n");
    
    createRandomNPCs();
    
    safePrint("Game initialized. Starting threads...\n");
}

void GameEngine::createRandomNPCs() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> typeDist(0, 2);
    std::uniform_real_distribution<> posDist(MAP_MIN_X + 1, MAP_MAX_X - 1);
    
    for (int i = 0; i < NPC_COUNT; i++) {
        int type = typeDist(gen);
        double x = posDist(gen);
        double y = posDist(gen);
        
        std::shared_ptr<NPC> npc;
        switch (type) {
            case 0:
                npc = std::make_shared<Squirrel>("Squirrel_" + std::to_string(i), x, y);
                break;
            case 1:
                npc = std::make_shared<Werewolf>("Werewolf_" + std::to_string(i), x, y);
                break;
            case 2:
                npc = std::make_shared<Druid>("Druid_" + std::to_string(i), x, y);
                break;
        }
        
        npcs.push_back(npc);
    }
}

void GameEngine::run() {
    gameRunning = true;
    elapsedTime = 0;
    
    movementThread = std::thread(&GameEngine::movementWorker, this);
    battleThread = std::thread(&GameEngine::battleWorker, this);
    displayThread = std::thread(&GameEngine::displayWorker, this);
    
    std::this_thread::sleep_for(std::chrono::seconds(GAME_DURATION));
    
    stop();
    
    if (movementThread.joinable()) movementThread.join();
    if (battleThread.joinable()) battleThread.join();
    if (displayThread.joinable()) displayThread.join();
    
    printSurvivors();
}

void GameEngine::stop() {
    gameRunning = false;
    battleQueue.stop();
}

void GameEngine::movementWorker() {
    std::random_device rd;
    std::mt19937 g(rd());
    
    while (gameRunning) {
        std::vector<size_t> indices(npcs.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), g);
        
        for (size_t idx : indices) {
            auto& npc = npcs[idx];
            if (!npc->isAlive()) continue;
            
            npc->move(MAP_MIN_X, MAP_MAX_X, MAP_MIN_Y, MAP_MAX_Y);
            
            DetectionVisitor detector(npcs, battleQueue, npc);
            detector.detectBattles();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void GameEngine::battleWorker() {
    while (gameRunning || !battleQueue.isEmpty()) {
        BattleTask task;
        if (battleQueue.tryGetTask(task)) {
            processBattle(task);
        }
    }
    
    safePrint("Battle thread stopped.\n");
}

void GameEngine::processBattle(const BattleTask& task) {
    auto attacker = task.attacker;
    auto defender = task.defender;
    
    if (!attacker->isAlive() || !defender->isAlive()) {
        return;
    }
    
    double distance = attacker->calculateDistance(defender.get());
    if (distance > attacker->getAttackDistance()) {
        return;
    }
    
    if (!attacker->canAttack(defender.get())) {
        return;
    }
    
    if (attacker->tryAttack(defender.get())) {
        defender->setAlive(false);
        
        std::stringstream ss;
        ss << attacker->getName() << " (" << attacker->getType() 
           << ") killed " << defender->getName() << " (" << defender->getType() << ")\n";
        safePrint(ss.str());
        
        battleLogger.logBattleEvent(ss.str());
    }
}

void GameEngine::displayWorker() {
    while (gameRunning && elapsedTime < GAME_DURATION) {
        printMap();
        
        std::this_thread::sleep_for(std::chrono::seconds(DISPLAY_INTERVAL));
        elapsedTime++;
    }
    
    safePrint("Display thread stopped.\n");
}

void GameEngine::printMap() const {
    const int MAP_WIDTH = 50;
    const int MAP_HEIGHT = 20;
    
    char map[MAP_HEIGHT][MAP_WIDTH];
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            map[y][x] = '.';
        }
    }
    
    for (const auto& npc : npcs) {
        if (npc->isAlive()) {
            double npcX = npc->getX();
            double npcY = npc->getY();
            
            int mapX = static_cast<int>((npcX - MAP_MIN_X) / (MAP_MAX_X - MAP_MIN_X) * (MAP_WIDTH - 1));
            int mapY = static_cast<int>((npcY - MAP_MIN_Y) / (MAP_MAX_Y - MAP_MIN_Y) * (MAP_HEIGHT - 1));
            
            if (mapX >= 0 && mapX < MAP_WIDTH && mapY >= 0 && mapY < MAP_HEIGHT) {
                map[mapY][mapX] = npc->getMapSymbol();
            }
        }
    }
    
    std::stringstream ss;
    ss << "\n=== Time: " << elapsedTime << "s ===\n";
    ss << std::string(MAP_WIDTH + 2, '-') << "\n";
    for (int y = 0; y < MAP_HEIGHT; y++) {
        ss << '|';
        for (int x = 0; x < MAP_WIDTH; x++) {
            ss << map[y][x];
        }
        ss << "|\n";
    }
    ss << std::string(MAP_WIDTH + 2, '-') << "\n";
    
    ss << "Legend: S=Squirrel, W=Werewolf, D=Druid\n";
    
    int aliveCount = 0;
    int squirrels = 0, werewolves = 0, druids = 0;
    
    for (const auto& npc : npcs) {
        if (npc->isAlive()) {
            aliveCount++;
            if (npc->getType() == "Squirrel") squirrels++;
            else if (npc->getType() == "Werewolf") werewolves++;
            else if (npc->getType() == "Druid") druids++;
        }
    }
    
    ss << "Alive: " << aliveCount 
       << " (S:" << squirrels 
       << " W:" << werewolves 
       << " D:" << druids << ")\n";
    
    ss << "Battle queue: " << battleQueue.size() << " tasks\n";
    
    safePrint(ss.str());
}

void GameEngine::printSurvivors() const {
    std::stringstream ss;
    ss << "\n=== GAME OVER ===\n";
    ss << "Total time: " << elapsedTime << " seconds\n";
    
    std::vector<std::shared_ptr<NPC>> survivors;
    int totalSquirrels = 0, totalWerewolves = 0, totalDruids = 0;
    
    for (const auto& npc : npcs) {
        if (npc->isAlive()) {
            survivors.push_back(npc);
            if (npc->getType() == "Squirrel") totalSquirrels++;
            else if (npc->getType() == "Werewolf") totalWerewolves++;
            else if (npc->getType() == "Druid") totalDruids++;
        }
    }
    
    ss << "\n=== SURVIVORS ===\n";
    ss << "Total survivors: " << survivors.size() << "\n";
    ss << "Squirrels: " << totalSquirrels << "\n";
    ss << "Werewolves: " << totalWerewolves << "\n";
    ss << "Druids: " << totalDruids << "\n";
    
    if (!survivors.empty()) {
        ss << "\nSurvivor list:\n";
        ss << std::left << std::setw(20) << "Name" 
           << std::setw(15) << "Type" 
           << std::setw(10) << "X" 
           << std::setw(10) << "Y" << "\n";
        ss << std::string(55, '-') << "\n";
        
        for (const auto& npc : survivors) {
            ss << std::left << std::setw(20) << npc->getName()
               << std::setw(15) << npc->getType()
               << std::setw(10) << std::fixed << std::setprecision(1) << npc->getX()
               << std::setw(10) << npc->getY() << "\n";
        }
    }
    
    safePrint(ss.str());
}