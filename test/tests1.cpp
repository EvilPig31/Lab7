#include <gtest/gtest.h>
#include "../include/npc.h"
#include "../include/npc_factory.h"
#include "../include/visitor.h"
#include "../include/observer.h"
#include "../include/game_engine.h"
#include <fstream>
#include <memory>
#include <thread>
#include <chrono>
#include <cstdio>

using namespace std;

TEST(NPCTest, NPCConstructor) {
    Squirrel squirrel("TestSquirrel", 100.0, 200.0);
    
    EXPECT_EQ(squirrel.getName(), "TestSquirrel");
    EXPECT_DOUBLE_EQ(squirrel.getX(), 100.0);
    EXPECT_DOUBLE_EQ(squirrel.getY(), 200.0);
    EXPECT_TRUE(squirrel.isAlive());
    EXPECT_EQ(squirrel.getType(), "Squirrel");
}

TEST(NPCTest, CoordinateValidation) {
    EXPECT_TRUE(NPC::isValidCoordinates(1.0, 1.0));
    EXPECT_TRUE(NPC::isValidCoordinates(500.0, 500.0));
    
    EXPECT_FALSE(NPC::isValidCoordinates(0.0, 100.0));
    EXPECT_FALSE(NPC::isValidCoordinates(100.0, 0.0));
    EXPECT_FALSE(NPC::isValidCoordinates(501.0, 100.0));
}

TEST(NPCTest, DistanceCalculation) {
    Squirrel s1("S1", 0.0, 0.0);
    Squirrel s2("S2", 3.0, 4.0);
    
    EXPECT_DOUBLE_EQ(s1.calculateDistance(&s2), 5.0);
}

TEST(NPCTest, SquirrelAttackRules) {
    Squirrel squirrel("Sq", 100, 100);
    Werewolf wolf("Wolf", 101, 101);
    Druid druid("Dru", 102, 102);
    
    EXPECT_TRUE(squirrel.canAttack(&wolf));
    EXPECT_TRUE(squirrel.canAttack(&druid));
    EXPECT_FALSE(squirrel.canAttack(&squirrel));
}

TEST(NPCTest, WerewolfAttackRules) {
    Werewolf wolf("Wolf", 100, 100);
    Druid druid("Dru", 101, 101);
    Squirrel squirrel("Sq", 102, 102);
    
    EXPECT_TRUE(wolf.canAttack(&druid));
    EXPECT_FALSE(wolf.canAttack(&squirrel));
}

TEST(NPCTest, DruidAttackRules) {
    Druid druid("Dru", 100, 100);
    Squirrel squirrel("Sq", 101, 101);
    Werewolf wolf("Wolf", 102, 102);
    
    EXPECT_FALSE(druid.canAttack(&squirrel));
    EXPECT_FALSE(druid.canAttack(&wolf));
}

TEST(NPCTest, MovementDistances) {
    Squirrel squirrel("Sq", 100, 100);
    Werewolf wolf("Wolf", 100, 100);
    Druid druid("Dru", 100, 100);
    
    EXPECT_DOUBLE_EQ(squirrel.getMoveDistance(), 5.0);
    EXPECT_DOUBLE_EQ(wolf.getMoveDistance(), 40.0);
    EXPECT_DOUBLE_EQ(druid.getMoveDistance(), 10.0);
}

TEST(NPCTest, AttackDistances) {
    Squirrel squirrel("Sq", 100, 100);
    Werewolf wolf("Wolf", 100, 100);
    Druid druid("Dru", 100, 100);
    
    EXPECT_DOUBLE_EQ(squirrel.getAttackDistance(), 5.0);
    EXPECT_DOUBLE_EQ(wolf.getAttackDistance(), 5.0);
    EXPECT_DOUBLE_EQ(druid.getAttackDistance(), 10.0);
}

TEST(NPCTest, MapSymbols) {
    Squirrel squirrel("Sq", 100, 100);
    Werewolf wolf("Wolf", 100, 100);
    Druid druid("Dru", 100, 100);
    
    EXPECT_EQ(squirrel.getMapSymbol(), 'S');
    EXPECT_EQ(wolf.getMapSymbol(), 'W');
    EXPECT_EQ(druid.getMapSymbol(), 'D');
}

TEST(NPCTest, DiceRoll) {
    for (int i = 0; i < 100; i++) {
        int roll = NPC::rollDice();
        EXPECT_GE(roll, 1);
        EXPECT_LE(roll, 6);
    }
}

TEST(NPCTest, MovementWithinBounds) {
    Squirrel squirrel("Sq", 50.0, 50.0);
    for (int i = 0; i < 100; i++) {
        squirrel.move(0.0, 100.0, 0.0, 100.0);
        EXPECT_GE(squirrel.getX(), 0.0);
        EXPECT_LE(squirrel.getX(), 100.0);
        EXPECT_GE(squirrel.getY(), 0.0);
        EXPECT_LE(squirrel.getY(), 100.0);
    }
}

TEST(FactoryTest, CreateNPC) {
    auto squirrel = NPCFactory::createNPC(NPCFactory::NPCType::SQUIRREL, 
                                         "TestSquirrel", 100, 200);
    EXPECT_NE(squirrel, nullptr);
    EXPECT_EQ(squirrel->getType(), "Squirrel");
    
    auto werewolf = NPCFactory::createNPC(NPCFactory::NPCType::WEREWOLF, 
                                         "TestWolf", 150, 250);
    EXPECT_NE(werewolf, nullptr);
    EXPECT_EQ(werewolf->getType(), "Werewolf");
    
    auto druid = NPCFactory::createNPC(NPCFactory::NPCType::DRUID, 
                                      "TestDruid", 200, 300);
    EXPECT_NE(druid, nullptr);
    EXPECT_EQ(druid->getType(), "Druid");
}

TEST(FactoryTest, InvalidCoordinates) {
    auto npc = NPCFactory::createNPC(NPCFactory::NPCType::SQUIRREL, 
                                    "BadNPC", 0, 0);
    EXPECT_EQ(npc, nullptr);
}

TEST(BattleQueueTest, BasicOperations) {
    BattleQueue queue;
    
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_EQ(queue.size(), 0);
    
    auto npc1 = make_shared<Squirrel>("Sq1", 100, 100);
    auto npc2 = make_shared<Werewolf>("Wolf1", 101, 101);
    
    BattleTask task(npc1, npc2);
    queue.addTask(task);
    
    EXPECT_FALSE(queue.isEmpty());
    EXPECT_EQ(queue.size(), 1);
    
    BattleTask retrieved;
    EXPECT_TRUE(queue.tryGetTask(retrieved));
    EXPECT_EQ(retrieved.attacker, npc1);
    EXPECT_EQ(retrieved.defender, npc2);
    
    EXPECT_TRUE(queue.isEmpty());
}

TEST(BattleQueueTest, MultipleTasks) {
    BattleQueue queue;
    
    auto npc1 = make_shared<Squirrel>("Sq1", 100, 100);
    auto npc2 = make_shared<Werewolf>("Wolf1", 101, 101);
    auto npc3 = make_shared<Druid>("Dru1", 102, 102);
    
    queue.addTask(BattleTask(npc1, npc2));
    queue.addTask(BattleTask(npc1, npc3));
    
    EXPECT_EQ(queue.size(), 2);
    
    BattleTask task1, task2;
    EXPECT_TRUE(queue.tryGetTask(task1));
    EXPECT_TRUE(queue.tryGetTask(task2));
    
    EXPECT_EQ(task1.attacker, npc1);
    EXPECT_EQ(task1.defender, npc2);
    EXPECT_EQ(task2.defender, npc3);
}

TEST(BattleQueueTest, StopSignal) {
    BattleQueue queue;
    queue.stop();
    BattleTask task;
    EXPECT_FALSE(queue.tryGetTask(task));
    EXPECT_TRUE(queue.shouldStop());
}
TEST(DetectionVisitorTest, DetectBattlesWithinRange) {
    vector<shared_ptr<NPC>> npcs;
    BattleQueue queue;
    
    auto squirrel = make_shared<Squirrel>("Sq", 100, 100);
    auto wolf = make_shared<Werewolf>("Wolf", 101, 101);
    
    npcs.push_back(squirrel);
    npcs.push_back(wolf);
    
    DetectionVisitor detector(npcs, queue, squirrel);
    detector.detectBattles();
    EXPECT_FALSE(queue.isEmpty());
}

TEST(DetectionVisitorTest, NoBattleOutOfRange) {
    vector<shared_ptr<NPC>> npcs;
    BattleQueue queue;
    
    auto squirrel = make_shared<Squirrel>("Sq", 100, 100);
    auto wolf = make_shared<Werewolf>("Wolf", 200, 200);
    
    npcs.push_back(squirrel);
    npcs.push_back(wolf);
    
    DetectionVisitor detector(npcs, queue, squirrel);
    detector.detectBattles();
    EXPECT_TRUE(queue.isEmpty());
}

TEST(DetectionVisitorTest, DeadNPCNoDetection) {
    vector<shared_ptr<NPC>> npcs;
    BattleQueue queue;
    
    auto squirrel = make_shared<Squirrel>("Sq", 100, 100);
    auto wolf = make_shared<Werewolf>("Wolf", 101, 101);
    wolf->setAlive(false);
    
    npcs.push_back(squirrel);
    npcs.push_back(wolf);
    
    DetectionVisitor detector(npcs, queue, squirrel);
    detector.detectBattles();
    EXPECT_TRUE(queue.isEmpty());
}

TEST(ObserverTest, ConsoleLogger) {
    ConsoleLogger logger;
    logger.update("Test event");
    EXPECT_TRUE(true);
}

TEST(ObserverTest, FileLoggerCreatesFile) {
    string filename = "test_log.txt";
    FileLogger logger(filename);
    
    logger.update("Test event");
    
    ifstream file(filename);
    EXPECT_TRUE(file.is_open());
    file.close();
    
    remove(filename.c_str());
}

TEST(ObserverTest, BattleLoggerNotifiesMultiple) {
    BattleLogger logger;
    
    class MockObserver : public BattleObserver {
    public:
        int count = 0;
        string lastMsg;
        
        void update(const string& event) override {
            count++;
            lastMsg = event;
        }
    };
    
    MockObserver obs1, obs2;
    
    logger.attach(&obs1);
    logger.attach(&obs2);
    
    logger.logBattleEvent("Test");
    
    EXPECT_EQ(obs1.count, 1);
    EXPECT_EQ(obs2.count, 1);
    EXPECT_EQ(obs1.lastMsg, "Test");
}

TEST(GameEngineTest, Initialization) {
    GameEngine engine;
    EXPECT_TRUE(true);
}

TEST(IntegrationTest, CompleteBattleScenario) {
    vector<shared_ptr<NPC>> npcs;
    BattleQueue queue;
    auto squirrel = make_shared<Squirrel>("Sq", 100, 100);
    auto wolf = make_shared<Werewolf>("Wolf", 101, 101);
    auto druid = make_shared<Druid>("Dru", 102, 102);
    
    npcs.push_back(squirrel);
    npcs.push_back(wolf);
    npcs.push_back(druid);
    DetectionVisitor detector1(npcs, queue, squirrel);
    detector1.detectBattles();
    
    DetectionVisitor detector2(npcs, queue, wolf);
    detector2.detectBattles();
    EXPECT_GE(queue.size(), 1);
    while (!queue.isEmpty()) {
        BattleTask task;
        if (queue.tryGetTask(task)) {
            if (task.attacker && task.defender && 
                task.attacker->isAlive() && task.defender->isAlive()) {
                
                double dist = task.attacker->calculateDistance(task.defender.get());
                if (dist <= task.attacker->getAttackDistance() && 
                    task.attacker->canAttack(task.defender.get())) {
                    task.attacker->tryAttack(task.defender.get());
                }
            }
        }
    }
    
    EXPECT_TRUE(true);
}

TEST(ThreadSafetyTest, ConcurrentAccessToNPC) {
    auto squirrel = make_shared<Squirrel>("Sq", 50, 50);
    vector<thread> threads;
    atomic<int> readCount{0};
    atomic<int> writeCount{0};
    
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&squirrel, &readCount]() {
            double x = squirrel->getX();
            double y = squirrel->getY();
            readCount++;
        });
    }
    
    for (int i = 0; i < 5; i++) {
        threads.emplace_back([&squirrel, &writeCount]() {
            // Двигаем NPC
            squirrel->move(0, 100, 0, 100);
            writeCount++;
        });
    }
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    EXPECT_EQ(readCount, 10);
    EXPECT_EQ(writeCount, 5);
}

TEST(ThreadSafetyTest, BattleQueueConcurrentAccess) {
    BattleQueue queue;
    atomic<int> added{0};
    vector<thread> addThreads;
    for (int i = 0; i < 5; i++) {
        addThreads.emplace_back([&queue, &added, i]() {
            auto npc1 = make_shared<Squirrel>("Sq" + to_string(i), 100, 100);
            auto npc2 = make_shared<Werewolf>("Wolf" + to_string(i), 101, 101);
            
            for (int j = 0; j < 10; j++) {
                queue.addTask(BattleTask(npc1, npc2));
                added++;
                this_thread::sleep_for(chrono::milliseconds(1));
            }
        });
    }
    
    vector<thread> removeThreads;
    for (int i = 0; i < 3; i++) {
        removeThreads.emplace_back([&queue]() {
            for (int j = 0; j < 20; j++) {
                BattleTask task;
                queue.tryGetTask(task);
                this_thread::sleep_for(chrono::milliseconds(2));
            }
        });
    }
    for (auto& t : addThreads) {
        if (t.joinable()) t.join();
    }
    this_thread::sleep_for(chrono::milliseconds(100));
    queue.stop();
    
    for (auto& t : removeThreads) {
        if (t.joinable()) t.join();
    }
    
    EXPECT_TRUE(added > 0);
}

TEST(EdgeCasesTest, EmptyBattleQueue) {
    BattleQueue queue;
    
    EXPECT_TRUE(queue.isEmpty());
    
    BattleTask task;
    EXPECT_FALSE(queue.tryGetTask(task));
    
    queue.stop();
    EXPECT_TRUE(queue.shouldStop());
}

TEST(EdgeCasesTest, NPCSelfDistance) {
    Squirrel squirrel("Sq", 100, 100);
    EXPECT_DOUBLE_EQ(squirrel.calculateDistance(&squirrel), 0.0);
}

TEST(EdgeCasesTest, NullNPCDistance) {
    Squirrel squirrel("Sq", 100, 100);
    EXPECT_DOUBLE_EQ(squirrel.calculateDistance(nullptr), 999999.0);
}

TEST(EdgeCasesTest, ZeroMovement) {
    Squirrel squirrel("Sq", 50, 50);
    EXPECT_TRUE(squirrel.isAlive());
    squirrel.move(0, 100, 0, 100);
    EXPECT_GE(squirrel.getX(), 0.0);
    EXPECT_LE(squirrel.getX(), 100.0);
}

TEST(PerformanceTest, BattleQueueThroughput) {
    BattleQueue queue;
    
    auto start = chrono::high_resolution_clock::now();
    const int TASK_COUNT = 1000;
    for (int i = 0; i < TASK_COUNT; i++) {
        auto npc1 = make_shared<Squirrel>("Sq" + to_string(i), 100, 100);
        auto npc2 = make_shared<Werewolf>("Wolf" + to_string(i), 101, 101);
        queue.addTask(BattleTask(npc1, npc2));
    }
    
    auto mid = chrono::high_resolution_clock::now();
    int retrieved = 0;
    while (retrieved < TASK_COUNT) {
        BattleTask task;
        if (queue.tryGetTask(task)) {
            retrieved++;
        }
    }
    
    auto end = chrono::high_resolution_clock::now();
    auto addTime = chrono::duration_cast<chrono::microseconds>(mid - start);
    auto removeTime = chrono::duration_cast<chrono::microseconds>(end - mid);
    EXPECT_LT(addTime.count(), 100000);
    EXPECT_LT(removeTime.count(), 100000);
    
    cout << "\nPerformance: Add " << TASK_COUNT << " tasks: " 
         << addTime.count() << "µs, Remove: " << removeTime.count() << "µs" << endl;
}

TEST(RandomnessTest, DiceDistribution) {
    const int ROLLS = 10000;
    int counts[7] = {0};
    
    for (int i = 0; i < ROLLS; i++) {
        int roll = NPC::rollDice();
        EXPECT_GE(roll, 1);
        EXPECT_LE(roll, 6);
        counts[roll]++;
    }
    
    for (int i = 1; i <= 6; i++) {
        EXPECT_GT(counts[i], 0) << "Value " << i << " never rolled";
    }
    double expected = ROLLS / 6.0;
    for (int i = 1; i <= 6; i++) {
        double ratio = counts[i] / expected;
        EXPECT_GT(ratio, 0.5) << "Value " << i << " appears too rarely";
        EXPECT_LT(ratio, 1.5) << "Value " << i << " appears too often";
    }
}

TEST(RandomnessTest, MovementRandomness) {
    Squirrel squirrel("Sq", 50, 50);
    double prevX = squirrel.getX();
    double prevY = squirrel.getY();
    bool moved = false;
    
    for (int i = 0; i < 10; i++) {
        squirrel.move(0, 100, 0, 100);
        double newX = squirrel.getX();
        double newY = squirrel.getY();
        
        if (newX != prevX || newY != prevY) {
            moved = true;
        }
        
        prevX = newX;
        prevY = newY;
    }
    
    EXPECT_TRUE(moved) << "NPC didn't move after 10 attempts";
}

TEST(GameLogicTest, SquirrelCanKillWerewolf) {
    Squirrel squirrel("Sq", 100, 100);
    Werewolf wolf("Wolf", 101, 101);
    int kills = 0;
    const int ATTEMPTS = 1000;
    
    for (int i = 0; i < ATTEMPTS; i++) {
        if (squirrel.tryAttack(&wolf)) {
            kills++;
        }
    }
    EXPECT_GT(kills, 0) << "Squirrel never killed werewolf in " << ATTEMPTS << " attempts";
    EXPECT_LT(kills, ATTEMPTS) << "Squirrel always killed werewolf (should be random)";
}

TEST(GameLogicTest, WerewolfCannotKillSquirrel) {
    Werewolf wolf("Wolf", 100, 100);
    Squirrel squirrel("Sq", 101, 101);
    EXPECT_FALSE(wolf.tryAttack(&squirrel));
}

TEST(GameLogicTest, DruidNeverAttacks) {
    Druid druid("Dru", 100, 100);
    Squirrel squirrel("Sq", 101, 101);
    Werewolf wolf("Wolf", 102, 102);
    
    EXPECT_FALSE(druid.tryAttack(&squirrel));
    EXPECT_FALSE(druid.tryAttack(&wolf));
    
    for (int i = 0; i < 100; i++) {
        EXPECT_FALSE(druid.tryAttack(&squirrel));
    }
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    
    cout << "Running Multi-threaded Dungeon Game tests..." << endl;
    cout << "============================================" << endl;
    
    int result = RUN_ALL_TESTS();
    
    const char* testFiles[] = {
        "test_log.txt",
        "game_log.txt",
        "test_save.txt",
        nullptr
    };
    
    for (int i = 0; testFiles[i] != nullptr; i++) {
        ifstream file(testFiles[i]);
        if (file.good()) {
            file.close();
            remove(testFiles[i]);
        }
    }
    return result;
}