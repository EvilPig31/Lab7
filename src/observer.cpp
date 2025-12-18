#include "../include/observer.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>

void BattleSubject::attach(BattleObserver* observer){
    observers.push_back(observer);
}
void BattleSubject::detach(BattleObserver* observer){
    auto it = std::find(observers.begin(), observers.end(), observer);
    if (it != observers.end()) {
        observers.erase(it);
    }
}
void BattleSubject::notify(const std::string& event){
    for (auto observer : observers) {
        observer->update(event);
    }
}
void ConsoleLogger::update(const std::string& event){
    std::time_t now = std::time(nullptr);
    std::tm* timeinfo = std::localtime(&now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S]", timeinfo);
    
    std::cout << buffer << " " << event << std::endl;
}
FileLogger::FileLogger(const std::string& filename) : filename(filename){}
void FileLogger::update(const std::string& event){
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        std::time_t now = std::time(nullptr);
        std::tm* timeinfo = std::localtime(&now);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S]", timeinfo);
        
        file << buffer << " " << event << std::endl;
        file.close();
    }
}
void BattleLogger::logBattleEvent(const std::string& event){
    notify(event);
}