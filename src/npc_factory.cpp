#include "../include/npc_factory.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::shared_ptr<NPC> NPCFactory::createNPC(NPCType type, const std::string& name, double x, double y){
    if (!NPC::isValidCoordinates(x, y)) {
        std::cerr << "Error: Coordinates must be in range (0 < x <= 500, 0 < y <= 500)" << std::endl;
        return nullptr;
    }
    switch (type){
        case NPCType::SQUIRREL:
            return std::make_shared<Squirrel>(name, x, y);
        case NPCType::WEREWOLF:
            return std::make_shared<Werewolf>(name, x, y);
        case NPCType::DRUID:
            return std::make_shared<Druid>(name, x, y);
        default:
            return nullptr;
    }
}
bool NPCFactory::saveToFile(const std::vector<std::shared_ptr<NPC>>& npcs, const std::string& filename){
    std::ofstream file(filename);
    if (!file.is_open()){
        std::cerr << "Error: Cannot open file " << filename << " for writing" << std::endl;
        return false;
    }
    for (const auto& npc : npcs){
        if (npc->isAlive()) {
            file << typeToString(stringToType(npc->getType())) << ","
                 << npc->getName() << ","
                 << npc->getX() << ","
                 << npc->getY() << "\n";
        }
    }
    file.close();
    std::cout << "Saved " << npcs.size() << " NPCs to " << filename << std::endl;
    return true;
}
std::vector<std::shared_ptr<NPC>> NPCFactory::loadFromFile(const std::string& filename){
    std::vector<std::shared_ptr<NPC>> loadedNPCs;
    std::ifstream file(filename);
    if (!file.is_open()){
        std::cerr << "Error: Cannot open file " << filename << " for reading" << std::endl;
        return loadedNPCs;
    }
    std::string line;
    while (std::getline(file, line)){
        std::stringstream ss(line);
        std::string typeStr, name;
        double x, y;
        if (std::getline(ss, typeStr, ',') && std::getline(ss, name, ',') && (ss >> x) && ss.ignore() && (ss >> y)) {
            NPCType type = stringToType(typeStr);
            auto npc = createNPC(type, name, x, y);
            if (npc){
                loadedNPCs.push_back(npc);
            }
        }
    }
    file.close();
    std::cout << "Loaded " << loadedNPCs.size() << " NPCs from " << filename << std::endl;
    return loadedNPCs;
}
NPCFactory::NPCType NPCFactory::stringToType(const std::string& typeStr){
    if (typeStr == "SQUIRREL") return NPCType::SQUIRREL;
    if (typeStr == "WEREWOLF") return NPCType::WEREWOLF;
    if (typeStr == "DRUID") return NPCType::DRUID;
    return NPCType::SQUIRREL;
}
std::string NPCFactory::typeToString(NPCType type){
    switch (type){
        case NPCType::SQUIRREL: return "SQUIRREL";
        case NPCType::WEREWOLF: return "WEREWOLF";
        case NPCType::DRUID: return "DRUID";
        default: return "UNKNOWN";
    }
}