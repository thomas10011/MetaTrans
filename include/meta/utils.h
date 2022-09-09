#pragma once
#include "yaml-cpp/yaml.h"
#include "meta/MetaTrans.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include <iostream>

using namespace llvm;

namespace MetaTrans {

    class YamlUtil {
        protected:

            static std::unordered_map<std::string, InstType> str_inst_type_map;

        public:
            static int test();
            
            // parse config file.
            // return a map between ASM / IR to TIR.
            template<class T>
            static std::unordered_map<T, std::vector<InstType>>* parseMapConfig(
                std::string filePath,
                std::unordered_map<std::string, T> keyMap
            ) {
                auto        map   = new std::unordered_map<T, std::vector<InstType>>(); 
                YAML::Node  config = YAML::LoadFile(filePath);
                for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
                    T                       key = keyMap[it->first.as<std::string>()];
                    std::vector<InstType>   value;
                    for (auto tmp : it->second) {
                        value.push_back(str_inst_type_map[tmp.as<std::string>()]);
                    }
                    (*map)[key] = value; 
                }
                if (DebugFlag) {
                    std::cout << "loading config file: " << filePath        << "\n";
                    std::cout << "size of configs is: " << config  .size() << "\n";
                    std::cout << "size of key map: "   << keyMap .size() << "\n";
                }
                return map;
            }

    };

    class MetaUtil {
        
        public:
                
            static void printValueType(Value* value);

            static void printInstDependencyGraph(MetaBB* bb);

            static void printInstOperand(Instruction* inst);

            template<typename T>
            static std::string typeVecToString(std::vector<T> type_vector) {
                static_assert(
                    std::is_same<T, InstType>::value ||
                    std::is_same<T, DataType>::value ,
                    "Type of template not support!"
                );
                std::string s = "{ " + MetaUtil::toString(type_vector[0]);
                for (unsigned i = 1u; i < type_vector.size(); i++) { s += ", " + MetaUtil::toString(type_vector[i]); }
                s += " }";
                return s;
            }

            static std::string toString(DataType type);

            static std::string toString(InstType type);

            static std::vector<InstType> getInstType(Instruction& inst);

            /// if a value not exist in the map, create it by default constructor.
            template<typename K, typename V>
            static V* createValue(K* key, std::unordered_map<K*, V*>& map) {
                auto pair = map.find(key);
                if (pair != map.end()) return nullptr;
                return map[key] = new V();
            }
        
    };
}

