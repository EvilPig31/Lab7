#ifndef NPC_H
#define NPC_H

#include <string>
#include <memory>
#include <random>
#include <mutex>

class NPCVisitor;

class NPC {
protected:
    std::string name;
    double x;
    double y;
    bool alive;
    mutable std::mutex mtx;
    static std::mt19937 rng;
    static std::uniform_int_distribution<int> dice;
    
public:
    NPC(const std::string& name, double x, double y);
    virtual ~NPC() = default;
    std::string getName() const;
    std::string getType() const;
    double getX() const;
    double getY() const;
    bool isAlive() const;
    void setPosition(double newX, double newY);
    void setAlive(bool status);
    
    virtual void accept(NPCVisitor& visitor) = 0;
    virtual bool canAttack(const NPC* other) const = 0;
    virtual double getMoveDistance() const = 0;
    virtual double getAttackDistance() const = 0;
    
    void move(double minX, double maxX, double minY, double maxY);
    
    double calculateDistance(const NPC* other) const;
    
    static bool isValidCoordinates(double x, double y);
    
    static int rollDice();
    
    virtual bool tryAttack(NPC* other) = 0;
    
    virtual char getMapSymbol() const = 0;
    
    std::unique_lock<std::mutex> getLock() const;
};

class Squirrel : public NPC {
public:
    Squirrel(const std::string& name, double x, double y);
    void accept(NPCVisitor& visitor) override;
    bool canAttack(const NPC* other) const override;
    double getMoveDistance() const override;
    double getAttackDistance() const override;
    bool tryAttack(NPC* other) override;
    char getMapSymbol() const override;
};

class Werewolf : public NPC {
public:
    Werewolf(const std::string& name, double x, double y);
    void accept(NPCVisitor& visitor) override;
    bool canAttack(const NPC* other) const override;
    double getMoveDistance() const override;
    double getAttackDistance() const override;
    bool tryAttack(NPC* other) override;
    char getMapSymbol() const override;
};

class Druid : public NPC {
public:
    Druid(const std::string& name, double x, double y);
    void accept(NPCVisitor& visitor) override;
    bool canAttack(const NPC* other) const override;
    double getMoveDistance() const override;
    double getAttackDistance() const override;
    bool tryAttack(NPC* other) override;
    char getMapSymbol() const override;
};

#endif