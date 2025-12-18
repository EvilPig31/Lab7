#ifndef NPC_FACTORY_H
#define NPC_FACTORY_H

#include <memory>
#include <string>
#include <vector>
#include "npc.h"

class NPCFactory{
public:
    enum class NPCType{
        SQUIRREL,
        WEREWOLF,
        DRUID
    };
    static std::shared_ptr<NPC> createNPC(NPCType type, const std::string& name, double x, double y);
    static bool saveToFile(const std::vector<std::shared_ptr<NPC>>& npcs, const std::string& filename);
    static std::vector<std::shared_ptr<NPC>> loadFromFile(const std::string& filename);
    static NPCType stringToType(const std::string& typeStr);
    static std::string typeToString(NPCType type);
};

#endif