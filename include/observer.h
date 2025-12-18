#ifndef OBSERVER_H
#define OBSERVER_H

#include <string>
#include <vector>
#include <memory>

class BattleSubject{
private:
    std::vector<class BattleObserver*> observers;
public:
    void attach(BattleObserver * observer);
    void detach(BattleObserver * observer);
    void notify(const std::string &event);
};

class BattleObserver {
public:
    virtual ~BattleObserver() = default;
    virtual void update(const std::string &event) = 0;
};

class ConsoleLogger : public BattleObserver {
public:
    void update(const std::string &event) override;
};

class FileLogger : public BattleObserver {
private:
    std::string filename;
    
public:
    FileLogger(const std::string &filename = "log.txt");
    void update(const std::string &event) override;
};

class BattleLogger : public BattleSubject {
public:
    void logBattleEvent(const std::string &event);
};

#endif