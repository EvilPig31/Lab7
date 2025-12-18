#include "../include/npc.h"
#include "../include/visitor.h"
#include <cmath>
#include <iostream>
#include <random>
#include <chrono>

std::mt19937 NPC::rng(std::chrono::steady_clock::now().time_since_epoch().count());
std::uniform_int_distribution<int> NPC::dice(1, 6);

NPC::NPC(const std::string& name, double x, double y) 
    : name(name), x(x), y(y), alive(true) {}

std::string NPC::getName() const {
    std::lock_guard<std::mutex> lock(mtx);
    return name;
}

std::string NPC::getType() const {
    if (dynamic_cast<const Squirrel*>(this)) return "Squirrel";
    if (dynamic_cast<const Werewolf*>(this)) return "Werewolf";
    if (dynamic_cast<const Druid*>(this)) return "Druid";
    return "Unknown";
}

bool NPC::isValidCoordinates(double x, double y) {
    return (x > 0 && x <= 500 && y > 0 && y <= 500);
}

double NPC::getX() const {
    std::lock_guard<std::mutex> lock(mtx);
    return x;
}

double NPC::getY() const {
    std::lock_guard<std::mutex> lock(mtx);
    return y;
}

bool NPC::isAlive() const {
    std::lock_guard<std::mutex> lock(mtx);
    return alive;
}

void NPC::setPosition(double newX, double newY) {
    std::lock_guard<std::mutex> lock(mtx);
    x = newX;
    y = newY;
}

void NPC::setAlive(bool status) {
    std::lock_guard<std::mutex> lock(mtx);
    alive = status;
}

std::unique_lock<std::mutex> NPC::getLock() const {
    return std::unique_lock<std::mutex>(const_cast<std::mutex&>(mtx));
}

double NPC::calculateDistance(const NPC* other) const {
    if (!other || !other->isAlive()) return 999999.0;
    if (other == this) return 0.0;
    
    std::unique_lock<std::mutex> lock1(mtx, std::defer_lock);
    std::unique_lock<std::mutex> lock2(other->mtx, std::defer_lock);
    
    std::lock(lock1, lock2);
    
    double dx = x - other->x;
    double dy = y - other->y;
    return std::sqrt(dx * dx + dy * dy);
}

int NPC::rollDice() {
    return dice(rng);
}

void NPC::move(double minX, double maxX, double minY, double maxY) {
    if (!isAlive()) return;
    
    std::lock_guard<std::mutex> lock(mtx);
    
    std::uniform_real_distribution<double> dirDist(-1.0, 1.0);
    double dirX = dirDist(rng);
    double dirY = dirDist(rng);
    
    double length = std::sqrt(dirX * dirX + dirY * dirY);
    if (length > 0) {
        dirX /= length;
        dirY /= length;
    }
    
    double moveDist = getMoveDistance();
    double newX = x + dirX * moveDist;
    double newY = y + dirY * moveDist;
    
    if (newX < minX) newX = minX;
    if (newX > maxX) newX = maxX;
    if (newY < minY) newY = minY;
    if (newY > maxY) newY = maxY;
    
    x = newX;
    y = newY;
}

Squirrel::Squirrel(const std::string& name, double x, double y) 
    : NPC(name, x, y) {}

void Squirrel::accept(NPCVisitor& visitor) {
    visitor.visit(this);
}

bool Squirrel::canAttack(const NPC* other) const {
    if (!other || !other->isAlive()) return false;
    std::string type = other->getType();
    return (type == "Werewolf" || type == "Druid");
}

double Squirrel::getMoveDistance() const {
    return 5.0;
}

double Squirrel::getAttackDistance() const {
    return 5.0;
}

bool Squirrel::tryAttack(NPC* other) {
    if (!canAttack(other)) return false;
    
    int attackRoll = rollDice();
    int defenseRoll = other->rollDice();
    
    return attackRoll > defenseRoll;
}

char Squirrel::getMapSymbol() const {
    return 'S';
}

Werewolf::Werewolf(const std::string& name, double x, double y) 
    : NPC(name, x, y) {}

void Werewolf::accept(NPCVisitor& visitor) {
    visitor.visit(this);
}

bool Werewolf::canAttack(const NPC* other) const {
    if (!other || !other->isAlive()) return false;
    return other->getType() == "Druid";
}

double Werewolf::getMoveDistance() const {
    return 40.0;
}

double Werewolf::getAttackDistance() const {
    return 5.0;
}

bool Werewolf::tryAttack(NPC* other) {
    if (!canAttack(other)) return false;
    
    int attackRoll = rollDice();
    int defenseRoll = other->rollDice();
    
    return attackRoll > defenseRoll;
}

char Werewolf::getMapSymbol() const {
    return 'W';
}

Druid::Druid(const std::string& name, double x, double y) 
    : NPC(name, x, y) {}

void Druid::accept(NPCVisitor& visitor) {
    visitor.visit(this);
}

bool Druid::canAttack(const NPC* other) const {
    return false;
}

double Druid::getMoveDistance() const {
    return 10.0;
}

double Druid::getAttackDistance() const {
    return 10.0;
}

bool Druid::tryAttack(NPC* other) {
    return false;
}

char Druid::getMapSymbol() const {
    return 'D';
}