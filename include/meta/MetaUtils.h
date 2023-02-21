#pragma once
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "yaml-cpp/yaml.h"
#include "MetaTrans.h"

#include <iostream>
#include <unordered_map>

using namespace llvm;

namespace MetaTrans {
    static std::vector<std::string> InstTypeName = {"LOAD", "STORE", "COMPARE", "CALL", "BRANCH", "JUMP", "PHI", "ADD", "SUB", "MUL", "DIV", "REMAINDER", "AND", "OR", "XOR", "SHIFT", "NEG", "RET", "ALLOCATION", "ADDRESSING", "EXCEPTION", "SWAP", "MIN", "MAX", "SQRT", "FENCE", "CONVERT", "HINT", "MOV"};

    class YamlUtil {

        public:

            static std::unordered_map<std::string, InstType> str_inst_type_map;

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
            
            // parse config file.
            // return a map between ASM / IR to TIR.
            static std::unordered_map<std::string, std::vector<std::pair<InstType,std::vector<int>>>>* parseAsmMapConfig(std::string filePath) {
                auto        map   = new std::unordered_map<std::string, std::vector<std::pair<InstType,std::vector<int>>>>(); 
                YAML::Node  config = YAML::LoadFile(filePath);
                for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
                    std::string                  key = it->first.as<std::string>();
                    //std::cout   << "DEBUG:: Found " << key <<std::endl;

                    std::vector<std::pair<InstType, std::vector<int>>>   value;
                    std::string opcode;
                    for (auto tmp : it->second) {
                        //value.push_back(str_inst_type_map[tmp.as<std::string>()]);
                     
                        // if(tmp.IsSequence())
                        //      std::cout << "String opcode is IsSequence = " << std::endl;

                        // if(tmp.Type()==YAML::NodeType::Null)
                        //     continue;
                        std::vector<int> info;
                        if(tmp.IsScalar()){
                            std::cout << "String opcode IsScalar =  " << tmp.as<std::string>() << std::endl;
                            opcode = tmp.as<std::string>();
                            value.push_back(std::make_pair(str_inst_type_map[tmp.as<std::string>()], info));
                        }
                        else if(tmp.IsMap()){
                                std::cout << "tmp is IsMap !" << std::endl;
                            // if(tmp.first.Type()==YAML::NodeType::Null)
                            //     std::cout << "tmp.first is NULL !" << std::endl;
                            //  if(tmp.first.IsScalar())
                            //     std::cout << "tmp.first is Scalar !" << std::endl;
                            //  if(tmp.second.IsSequence())
                            //     std::cout << "tmp.second is IsMap !" << std::endl;
                            //  if(tmp.second.IsMap())
                            //     std::cout << "tmp.second is IsMap !" << std::endl;
                             for (auto i = tmp.begin(); i!= tmp.end(); i++) {
                                //std::cout << "Enter Loop: auto infoNode : tmp.second" << std::endl;
                                if(i->first.IsScalar()){
                                  std::cout << "Operation is Scalar !" << std::endl;
                                  opcode = i->first.as<std::string>();
                                  std::cout << "Operation = " << opcode <<std::endl;
                                }
                                if(i->first.IsMap())
                                  std::cout << "i->first is Map !" << std::endl;
                                if(i->second.IsMap())
                                  std::cout << "TypeSrc is Map !" << std::endl;
                                if(i->second.IsSequence()){
                                  std::cout << "TypeSrc is Sequence !" << std::endl;
                                  YAML::Node list = i->second;
                                   for ( auto list= i->second.begin();  list!= i->second.end(); list++) {
                                        //std::cout << "Enter Loop: auto infoNode : tmp.second = " << list->as<int>()<< std::endl;
                                        // if (infoNode.Type() == YAML::NodeType::Scalar) 
                                        info.push_back(list->as<int>());
                                
                                    }
                                }
                                if(i->second.IsScalar()){
                                //   std::cout << "i->second is scalar !" << std::endl;
                                //   std::cout << "i->second = " << i->second.as<int>()<<std::endl;
                                  info.push_back(i->second.as<int>());
                                }
                        //if(tmp.IsSequence() && tmp.second.Type() != YAML::NodeType::Null){
                            std::cout << "Entering else tmp is not scalar! " << std::endl;
                            // YAML::Node src = tmp.second.as<YAML::Node>();
                        // if(tmp.IsSequence() && tmp.second.Type() != YAML::NodeType::Null){
                           
                            value.push_back(std::make_pair(str_inst_type_map[opcode], info));

                            std::cout << "Make pair completes! " << std::endl;

                            }   

                        }
                        // if(it->second.IsSequence())
                        //     value.push_back(std::make_pair(str_inst_type_map[tmp.first.as<std::string>()], info));
                        // if(it->second.IsScalar())
                            

                        // if( info.size() != 0 ) 
                        //     std::cout   << "DEBUG::parseAsmMapConfig:: Operand "
                        //             << opcode << ", Of Inst  " <<  key 
                        //             << ", has the type source " << info[0] << std::endl;
                    }
                     (*map)[key] = value; 
                }
                
                if (DebugFlag) {
                    std::cout << "loading config file: " << filePath        << "\n";
                    std::cout << "size of configs is: " << config  .size() << "\n";
                }
                return map;
            }

    };

    class MetaUtil {
        
        public:

            static void compareCFG(MetaBB* x, MetaBB* y);
                
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

            template<typename T>
            static std::string vectorToJsonString(std::vector<T*> vec) {
                if (vec.empty()) return "[]";
                std::string res = "[";
                for (T* e : vec) res += e->toString() + ",";
                res[res.length() - 1] = ']';
                return res;
            }

            template<typename T>
            static std::string vectorToJsonString(std::vector<T> vec) {
                if (vec.empty()) return "[]";
                std::string res = "[";
                for (T& e : vec) res += e.toString() + ",";
                res[res.length() - 1] = ']';
                return res;
            }

            template<typename T>
            static void printVector(std::vector<T> vector, std::string msg) {
                std::cout << msg << std::endl;
                std::cout << "!! vector size: " << vector.size() << " !!" << std::endl;
                std::cout << "[ ";
                for (auto v : vector) 
                    std::cout << v << ", ";
                std::cout << " ]" << std::endl;
            }

            template<typename K, typename V>
            static void printMap(std::unordered_map<K, V> map, std::string msg) {
                std::cout << msg << std::endl;
                std::cout << "!! map size: " << map.size() << " !!" << std::endl;
                for (auto pair = map.begin(); pair != map.end(); ++pair)
                    std::cout << "key: "<< pair->first << "  -->>  " << "value: " << pair->second << std::endl;
            }
            

            static std::string toString(DataType type);

            static DataType stringToDataType(std::string str);

            static std::string toString(InstType type);

            static std::string upper(std::string s);

            static InstType stringToInstType(std::string str);

            static std::string toString(std::vector<InstType> type);

            static DataType extractDataType(Type& dataType);

            static int extractDataWidth(Type& dataType);

            static void writeToFile(std::string, std::string file);

            static std::string readFromFile(std::string file);

            /// if a value not exist in the map, create it by default constructor.
            template<typename K, typename V>
            static V* createValue(K* key, std::unordered_map<K*, V*>& map) {
                auto pair = map.find(key);
                if (pair != map.end()) return nullptr;
                return map[key] = new V();
            }
            
            static void paintInsColorRecursive(MetaInst* inst, int color, int type, int depth, Path* p);

            static void paintColor(MetaFunction* mF, int startColor);
        
            static unsigned long hashCode(std::vector<MetaInst*> instList);

            static unsigned long long getAvailableMemory();

    };



}

