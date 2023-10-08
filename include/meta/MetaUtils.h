//===-- MetaUtils.h -----------------*- C++ -----------------------------*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of utility functions. These functions are
// mainly general functions, such as string compare and process, parse yaml files,
// convert int to string and string to int, etc. These functions can be widly 
// used in preprocessing, learning, and translating.
//
//===----------------------------------------------------------------------===//

#pragma once
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "yaml-cpp/yaml.h"
#include "MetaTrans.h"
#include <queue>
#include <iostream>
#include <unordered_map>

using namespace llvm;

namespace MetaTrans {
    static std::vector<std::string> InstTypeName = {"LOAD", "STORE", "COMPARE", "CALL", "BRANCH", "JUMP", "PHI", "ADD", "SUB", "MUL", "DIV", "REMAINDER", "AND", "OR", "XOR", "SHIFT", "NEG", "RET", "ALLOCATION", "ADDRESSING", "EXCEPTION", "SWAP", "MIN", "MAX", "SQRT", "FENCE", "CONVERT", "HINT", "MOV", "CSR", "SIGN", "COMPLEX"};

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
                    if(str_inst_type_map.find(tmp.as<std::string>()) != str_inst_type_map.end()) {
                        value.push_back(std::make_pair(str_inst_type_map[tmp.as<std::string>()], info));
                    }
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
                    
                    if(str_inst_type_map.find(opcode) != str_inst_type_map.end()) {
                        value.push_back(std::make_pair(str_inst_type_map[opcode], info));
                    }
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

    static void setDataRoot(MetaFunction* MF);

    static std::string findDataRoot(MetaInst* inst);

    static std::string findDataRootRecursive(MetaOperand* inst, std::unordered_set<MetaOperand*> set);


    template<typename T>
    static std::string typeVecToString(std::vector<T> type_vector) {
        static_assert(
            std::is_same<T, InstType>::value ||
            std::is_same<T, DataType>::value ,
            "Type of template not support!"
        );
        if (type_vector.size() == 0) return "{ null }";
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

    static bool isFuncPtrDeclaration(std::string& str);

    // text中是否包含s
    static bool contain(std::string s, std::string& text);

    // test 中 s 出现的所有位置
    static std::vector<int> find(std::string s, std::string& text);

    // s是否为空串或者由空字符组成
    static bool isEmpty(std::string s);

    // 去除text两端的s
    static void strip(std::string s, std::string& text);

    // 根据delimiter切分text
    static std::vector<std::string> split(std::string delimiter, const std::string& text);

    // 根据delimiter连接字符串
    static std::string join(std::string delimiter, const std::vector<std::string>& list);

    // 去除text两端的空字符，返回新的字符串，不改变str
    static std::string trim(const std::string& str);

    // text是否以s开头
    static bool startwith(std::string s, const std::string& text);

    static bool isNumber(std::string s);
    
    // 比对两个字符串，忽略大小写
    static bool same(std::string x, std::string y);

    static std::string int2hex(int64_t num);

    static int64_t hex2int(std::string str);

    static std::string toString(DataType type);

    static DataType stringToDataType(std::string str);

    static void writeMapping(std::string, std::string file);

    static std::string toString(InstType type);

    static std::string upper(std::string s);

    static std::string lower(std::string& str); 
    
    static InstType stringToInstType(std::string str);

    static std::string toString(std::vector<InstType> type);

    static DataType extractDataType(Type& dataType);

    static int extractDataWidth(Type& dataType);

    static DataType extractDataType(std::string& src);

    static int extractDataWidth(std::string& src);

    static int extractPtrLevel(std::string& src);

    static void writeToFile(std::string, std::string file);

    static std::string readFromFile(std::string file);

    /// if a value not exist in the map, create it by default constructor.
    template<typename K, typename V>
    static V* createValue(K* key, std::unordered_map<K*, V*>& map) {
        auto pair = map.find(key);
        if (pair != map.end()) return nullptr;
        return map[key] = new V();
    }
    
    static int paintInsColorRecursive(MetaInst* inst, int color, int type, int depth, Path* p, std::unordered_set<MetaInst*>& visited);

    static int paintColor(MetaFunction* mF, int startColor);

    static int paintColorCheck(MetaFunction* mF);

    static unsigned long hashCode(std::vector<MetaInst*> instList);
    
    static unsigned long long getAvailableMemory();

    static int count_args(const char* fmt);

    static bool test();

};

template<class X, class Y>
class BidirectionMap {
private:
    std::unordered_map<X, Y> forward;
    std::unordered_map<Y, X> backward;

public:

    BidirectionMap& put(X key, Y value) {
        forward[key] = value;
        backward[value] = key;
        return *this;
    }

    BidirectionMap& put(Y key, X value) {
        backward[key] = value;
        forward[value] = key;
        return *this;
    }

    Y get(X key) { return forward[key]; }
    X get(Y key) { return backward[key]; }
    
    using BiMapForwardIterator = typename std::unordered_map<X, Y>::iterator;
    using BiMapBackwardIterator = typename std::unordered_map<X, Y>::iterator;

    BiMapForwardIterator begin() {
        return forward.begin();
    }
    BiMapForwardIterator end() {
        return forward.end();
    }
    BiMapBackwardIterator rbegin() {
        return backward.begin();
    }
    BiMapBackwardIterator rend() {
        return backward.end();
    }

    void clear() {
        forward.clear();
        backward.clear();
    }
};


template <class T>
class Graph {

private:
T root;

std::unordered_map<T, std::vector<T>*> succTable;
std::unordered_map<T, std::vector<T>*> predTable;

int _size;

public:

Graph() : root(NULL) {

}

~Graph() {
    for (auto it = succTable.begin(); it != succTable.end(); ++it) delete it->second;
}


Graph& setRoot(T r) { root = r; return *this; }
T getRoot() { return root; }

// add an edge from x to y.
// x --> y
Graph& add(T x, T y) {
    std::vector<T>* succs = getSuccPtr(x);
    std::vector<T>* preds = getPredPtr(y);
    succs->push_back(y);
    preds->push_back(x);
    return *this;
}

Graph& add(T x, const std::vector<T>& Y) {
    std::vector<T>* succs = getSuccPtr(x);
    for (T y : Y) {
    std::vector<T>* preds = getPredPtr(y);
        succs->push_back(y);
        preds->push_back(x);
    }
    return *this;
}

std::vector<T> getSuccessors(T x) {
    return *(getSuccPtr(x));
}

std::vector<T> getPredecessors(T x) {
    return *(getPredPtr(x));
}

T getSuccessor(T x, int idx) {
    return succTable[x]->at(idx);
}

T getPredecessor(T x, int idx) {
    return predTable[x]->at(idx);
}

std::vector<T>* getSuccPtr(T x) {
    if (succTable.find(x) == succTable.end()) {
        ++ _size;
        return succTable[x] = new std::vector<T>();
    }

    else 
        return succTable[x];
}


std::vector<T>* getPredPtr(T x) {
    if (predTable.find(x) == predTable.end()) {
        ++ _size;
        return predTable[x] = new std::vector<T>();
    }
    else 
        return predTable[x];
}

std::vector<T> getDFSseq() {
    std::vector<T> discovered, seq;
    std::unordered_set<T> visited;
    discovered.push_back(root);

    assert(root);
    while (!discovered.empty()) {
        T cur = discovered.back(); discovered.pop_back();

        if (visited.count(cur)) continue;

        visited.insert(cur);
        seq.push_back(cur);

        std::vector<T> succ = getSuccessors(cur);
        for (auto it = succ.rbegin(); it != succ.rend(); ++it) {
            if (!visited.count(*it)) {
                discovered.push_back(*it);
            }
        }
    }
    return seq;
}

std::vector<T> getBFSseq() {
    std::queue<T> que;
    std::vector<T> seq;
    std::unordered_set<T> visited;
    que.push(root);
    while (!que.empty()) {
        T cur = que.front(); que.pop();

        if (visited.count(cur)) continue;

        visited.insert(cur);
        seq.push_back(cur);

        for (auto t : getSuccessors(cur)) 
            if (!visited.count(t))
                que.push(t);
    }
    return seq;
}


int size() { return _size; }


};


template<class T>
class DominateTree {
    
private:

Graph<T> tree;

void dfs (
    const Graph<T>& g,
    T& cur,
    std::unordered_map<T, int>& dfn,
    std::unordered_map<T, T>& parent,
    std::vector<T>& seq
) {
    dfn[cur] = seq.size();
    seq.push_back(cur);
    for (T suc : g.getSuccessors(cur)) {
        if (dfn.find(suc) != dfn.end()) {
            dfs(suc);
            parent[suc] = cur;
        }
    }
}


std::unordered_map<T, T> getSdom(
    const Graph<T>& g,
    std::unordered_map<T, int>& dfn,
    std::unordered_map<T, T>& parent,
    std::vector<T>& seq
) {
    std::unordered_map<T, T> sdom, father;

    for (T t : seq) {
        father[t] = sdom[t] = t;
    }

    auto find = [&] (T x) -> T {
        if (father[x] == x) return x;
        // 路径压缩的过程中顺便更新最小的sdom
        if (dfn[sdom[father[x]]] < dfn[sdom[x]]) sdom[x] = father[x];
        return father[x] = find(father[x]);
    };

    for (auto it = seq.rbegin(); it != seq.rend(); ++it) {
        T curr = *it, semi = NULL;
        for (T pred : g.getPredecessors(curr)) {
            find(pred);
            if (dfn[pred] < dfn[curr]) {
                if (dfn.find(semi) != dfn.end() && dfn[semi] < dfn[pred])
                    semi = pred;
            }
            else {
                if (dfn[sdom[pred]] < dfn[sdom[curr]])
                    semi = sdom[pred];
            }
        }
        sdom[curr] = semi;
        father[curr] = parent[curr];
    }
    return sdom;
}

public:

DominateTree(const Graph<T>& g) {
    std::unordered_map<T, int> dfn;
    std::unordered_map<T, T> parent;
    std::vector<T> seq;
    dfs(g, g.getRoot(), dfn, parent, seq);

}

std::vector<T> getDominateFrontier() {

}

T getImmDomNode(T x) {
    return tree.getPredecessor(0);
}

std::vector<T> getDomNodes(T x) {

}

};

} // namespace MetaTrans

