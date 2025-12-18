#ifndef VISITOR_H
#define VISITOR_H

#include <vector>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

class NPC;
class Squirrel;
class Werewolf;
class Druid;

class NPCVisitor {
public:
    virtual ~NPCVisitor() = default;
    virtual void visit(Squirrel* squirrel) = 0;
    virtual void visit(Werewolf* werewolf) = 0;
    virtual void visit(Druid* druid) = 0;
};

struct BattleTask {
    std::shared_ptr<NPC> attacker;
    std::shared_ptr<NPC> defender;
    
    BattleTask() : attacker(nullptr), defender(nullptr) {}
    
    BattleTask(std::shared_ptr<NPC> a, std::shared_ptr<NPC> d)
        : attacker(a), defender(d) {}
};

class BattleQueue {
private:
    std::queue<BattleTask> tasks;
    mutable std::mutex mtx;
    std::condition_variable cv;
    bool stopFlag = false;
    
public:
    void addTask(const BattleTask& task);
    bool tryGetTask(BattleTask& task);
    void stop();
    bool isEmpty() const;
    bool shouldStop() const;
    size_t size() const;
};

class DetectionVisitor : public NPCVisitor {
private:
    std::vector<std::shared_ptr<NPC>>& npcs;
    BattleQueue& battleQueue;
    std::shared_ptr<NPC> currentNPC;
    void detectForNPC(NPC* npc);
public:
    DetectionVisitor(std::vector<std::shared_ptr<NPC>>& npcs, 
                     BattleQueue& queue, 
                     std::shared_ptr<NPC> npc);
    
    void visit(Squirrel* squirrel) override;
    void visit(Werewolf* werewolf) override;
    void visit(Druid* druid) override;
    
    void detectBattles();
};

#endif