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
#include <functional>

using namespace llvm;

namespace MetaTrans {
static std::vector<std::string> InstTypeName = {"LOAD", "STORE", "COMPARE", "CALL", "BRANCH", "JUMP", "PHI", "ADD", "SUB", "MUL", "DIV", "REMAINDER", "AND", "OR", "XOR", "SHIFT", "NEG", "RET", "ALLOCATION", "ADDRESSING", "EXCEPTION", "SWAP", "MIN", "MAX", "SQRT", "FENCE", "CONVERT", "HINT", "MOV", "CSR", "SIGN", "COMPLEX"};

class LibFunctionTable {

private:

std::string TableName;
std::map<std::string, std::pair<std::string, std::string>> LibFunctionMap;

public:

LibFunctionTable(std::string path);
~LibFunctionTable();
void setName(std::string path);
void loadLibFunctionTable();
std::string getReturnType(std::string funcName);
std::vector<std::string> getArgumentTypesList(std::string funcName);
bool getIsVarArgs(std::string funcName);
void dump(std::string funcName);
bool contain(const std::string& funcName);

};

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

    static std::string lower(std::string str); 
    
    static InstType stringToInstType(std::string str);

    static std::string toString(std::vector<InstType> type);

    static DataType extractDataType(Type& dataType);

    static int extractDataWidth(Type& dataType);

    static DataType extractDataType(std::string& src);

    static int extractDataWidth(std::string& src);

    static int extractPtrLevel(std::string& src);

    static void writeToFile(std::string, std::string file);

    static std::string readFromFile(std::string file);

    // union
    template<typename T>
    static std::unordered_set<T> u(const std::unordered_set<T>& x, const std::unordered_set<T>& y) {
        std::unordered_set<T> result = x;
        result.insert(y.begin(), y.end());
        return result;
    }

    // intersection
    template<typename T>
    static std::unordered_set<T> i(const std::unordered_set<T>& x, const std::unordered_set<T>& y) {
        std::unordered_set<T> result;
        for (auto e : x) if (y.count(e)) result.insert(e);
        return result;
    }

    // exclusive, x - y
    template<typename T>
    static std::unordered_set<T> e(const std::unordered_set<T>& x, const std::unordered_set<T>& y)  {
        std::unordered_set<T> result;
        for (auto e : x) if (!y.count(e)) result.insert(e);
        return result;
    }

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

    template<typename T>
    static int64_t hash(std::unordered_set<T> set) {
        size_t result = 0;
        std::hash<T> hash;
        for (auto e : set) {
            result += hash(e);
        }
        return result;
    }

    template<typename T>
    static int64_t hash(std::vector<T> vec) {
        size_t result = 0;
        std::hash<T> hash;
        for (auto e : vec) {
            result += hash(e);
        }
        return result;
    }

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
class GraphNode {
private:
T data;
std::vector<GraphNode<T>*> succVec;
std::vector<GraphNode<T>*> prevVec;

public:
GraphNode(T _data) : data(_data) { }
GraphNode() : data(NULL) { }
~GraphNode() {
    succVec.clear();
    prevVec.clear();
}

T getData() { return data; }
GraphNode& setData(T _data) { data = _data; return *this;}
GraphNode& addPred(GraphNode<T>* pred) { prevVec.push_back(pred); return*this; }
GraphNode& addSucc(GraphNode<T>* succ) { succVec.push_back(succ); return*this; }
std::vector<GraphNode<T>*> getPred() { return prevVec; }
std::vector<GraphNode<T>*> getSucc() { return succVec; }
GraphNode<T>* getPred(int idx) { return prevVec.at(idx); }
GraphNode<T>* getSucc(int idx) { return succVec.at(idx); }
};

template <class T>
class Graph {
protected:
GraphNode<T>* root;
// 存点
std::vector<GraphNode<T>*> nodeVec;
std::unordered_map<T, int> idxMap;

std::vector<GraphNode<T>*> cachedPostOrder;

public:

Graph() : root(NULL) {

}

Graph(const Graph<T>& g) {
    printf("INFO: Coping graph.\n");
    this->idxMap = g.idxMap;
    // 拷贝节点
    for (int i = 0; i < g.nodeVec.size(); ++i) {
        GraphNode<T>* node = g.nodeVec.at(i);
        GraphNode<T>* newNode = new GraphNode<T>(node->getData());
        this->nodeVec.push_back(newNode);
    }
    // 拷贝边
    for (int i = 0; i < g.nodeVec.size(); ++i) {
        GraphNode<T>* node = g.nodeVec[i];
        GraphNode<T>* newNode = this->nodeVec[i];
        for (auto next : node->getSucc()) {
            newNode->addSucc(this->nodeVec[this->idxMap[next->getData()]]);
        }
        for (auto prev : node->getPred()) {
            newNode->addPred(this->nodeVec[this->idxMap[prev->getData()]]);
        }
    }

    this->root = getNode(g.root->getData());
}

~Graph() {
    for (auto node : nodeVec) delete node;
}


Graph& setRoot(T r) { root = getNode(r); return *this; }
T getRoot() { return root->getData(); }
GraphNode<T>* getRootNode() { return root; }

// add an edge from x to y.
// x --> y
Graph& add(T x, T y) {
    GraphNode<T>* m = addNode(x);
    GraphNode<T>* n = addNode(y);
    m->addSucc(n);
    n->addPred(m);
    return *this;
}

Graph& add(T x, const std::vector<T>& Y) {
    for (T y : Y) add(x, y);
    return *this;
}

std::vector<T> getSuccessors(T x) {
    std::vector<T> result;
    for (GraphNode<T>* node : getNode(x)->getSucc()) {
        result.push_back(node->getData());
    }
    return result;
}

// 添加一个节点，不会重复添加
GraphNode<T>* addNode(T n) {
    if (idxMap.find(n) == idxMap.end()) {
        GraphNode<T>* node = new GraphNode<T>(n);
        nodeVec.push_back(node);
        idxMap[n] = nodeVec.size() - 1;
    }
    return nodeVec.at(idxMap[n]);;
}

GraphNode<T>* getNode(T n) {
    if (idxMap.count(n)) return nodeVec.at(idxMap[n]);
    printf("WARN: didn't find node!\n");
    return nullptr;
}

int getIdx(T n) {
    if (idxMap.count(n)) return idxMap[n];
    return -1;
}

std::vector<T> getPredecessors(T x) {
    std::vector<T> result;
    for (GraphNode<T>* node : getNode(x)->getPred()) {
        result.push_back(node->getData());
    }
    return result;
}

std::vector<GraphNode<T>*> getPreOrderDFSNodeSeq() {
    using NodePtr = GraphNode<T>*;
    std::vector<NodePtr> stack = { root }, seq = { root };
    std::unordered_set<NodePtr> visited, discovered = { root };
    stack.push_back(root);
    
    assert(root);
    while (!stack.empty()) {
        NodePtr top = stack.back();

        bool flag = false;
        for (NodePtr suc : top->getSucc()) {
            if (discovered.count(suc))  { /* backward */ }
            else if (visited.count(suc))  { /* forward */ }
            else {
                flag = true;
                stack.push_back(suc);
                discovered.insert(suc);
                seq.push_back(suc);
            }
        }
        if (!flag) {
            stack.pop_back();
            visited.insert(top);
        }
    }
    return seq;
}

std::vector<GraphNode<T>*> getPostOrderDFSNodeSeq() {
    if (!cachedPostOrder.empty()) return cachedPostOrder;
    using NodePtr = GraphNode<T>*;
    std::vector<NodePtr> stack = { root }, cachedPostOrder;
    std::unordered_set<NodePtr> visited, discovered = { root };
    assert(root);
    while (!stack.empty()) {
        NodePtr top = stack.back();

        bool flag = false;
        for (NodePtr suc : top->getSucc()) {
            if (discovered.count(suc))  { /* backward */ }
            else if (visited.count(suc))  { /* forward */ }
            else {
                flag = true;
                stack.push_back(suc);
                discovered.insert(suc);
            }
        }
        if (!flag) {
            stack.pop_back();
            visited.insert(top);
            cachedPostOrder.push_back(top);
        }
    }
    return cachedPostOrder;
}

std::vector<GraphNode<T>*> getReversePostOrderDFSNodeSeq() {
    using NodePtr = GraphNode<T>*;
    std::vector<NodePtr> seq = getPostOrderDFSNodeSeq();
    std::reverse(seq.begin(), seq.end());
    return seq;
}

std::vector<T> getPostOrderDFSSeq() {
    return (unpack(getPostOrderDFSNodeSeq()));
}

std::vector<T> getPreOrderDFSSeq() {
    return (unpack(getPreOrderDFSNodeSeq()));
}

std::vector<T> getReversePostOrderDFSSeq() {
    return (unpack(getReversePostOrderDFSNodeSeq()));
}

std::vector<int> getPostOrderDFSIdxSeq() {
    return (unpack2idx(getPostOrderDFSNodeSeq()));
}

std::vector<GraphNode<T>*> getBFSNodeSeq() {
    using NodePtr = GraphNode<T>*;
    std::queue<NodePtr> que;
    std::vector<NodePtr> seq;
    std::unordered_set<NodePtr> visited;
    que.push(root);
    while (!que.empty()) {
        NodePtr cur = que.front(); que.pop();

        if (visited.count(cur)) continue;

        visited.insert(cur);
        seq.push_back(cur);

        for (auto t : cur->getSucc()) if (!visited.count(t)) que.push(t);
    }

    return seq;
}

std::vector<T> getBFSSeq() {
    return unpack(getBFSNodeSeq());
}

std::vector<int> getBFSIdxSeq() {
    return unpack2idx(getBFSNodeSeq());
}

std::vector<T> unpack(std::vector<GraphNode<T>*> nodes) {
    std::vector<T> result;
    for (GraphNode<T>* node : nodes) {
        result.push_back(node->getData());
    }
    return result;
} 

std::vector<int> unpack2idx(std::vector<GraphNode<T>*> nodes) {
    std::vector<int> result;
    for (GraphNode<T>* node : nodes) {
        result.push_back(idxMap[node->getData()]);
    }
    return result;
} 

int size() { return nodeVec.size(); }

Graph<T>& clear() {
    idxMap.clear();
    for (auto node : nodeVec) delete node;
    nodeVec.clear();
    cachedPostOrder.clear();
    root = nullptr;
    return *this;
}


Graph<T>& dump() {
    for (int i = 0; i < nodeVec.size(); ++i) {
        auto node = nodeVec[i];
        printf("node %d[%lx] --> [", i, node);
        for (auto suc : node->getSucc()) {
            printf("%d[%lx], ", idxMap[suc->getData()], suc);
        }
        printf("]\n");
    }
}

};


template<class T>
class DominateTree : public Graph<T> {
    
protected:

std::unordered_map<GraphNode<T>*, std::unordered_set<GraphNode<T>*>> domMap;
std::unordered_map<GraphNode<T>*, GraphNode<T>*> idomMap; // key' dominator is value.
std::unordered_map<GraphNode<T>*, std::unordered_set<GraphNode<T>*>> domFrtMap; // record dominate frontier

std::unordered_map<GraphNode<T>*, std::vector<GraphNode<T>*>> tree;

void naivedom() {
    using NodePtr = GraphNode<T>*;
    assert(this->root);
    const int rootIdx = this->idxMap[this->root->getData()];
    std::vector<NodePtr> seq = this->getReversePostOrderDFSNodeSeq(); // 最好是图的逆后序

    domMap[this->root] = { this->root };
    bool flag = true;
    while (flag) {
        flag = false;
        // 对所有节点遍历
        for (NodePtr u : seq) {
            std::vector<NodePtr> preds = u->getPred();
            std::unordered_set<NodePtr> tmp;
            int idx = 0;
            while (idx < preds.size() && !domMap.count(preds[idx])) // 寻找第一个不是全集的前驱
                idx++;
            if (idx == preds.size()) continue;
            tmp = domMap[preds.at(idx++)];
            while (idx < preds.size()) {
                NodePtr pred = preds.at(idx++);
                if (domMap.find(pred) != domMap.end()) { // 如果 domMap 不包含 pred 则此时 pred 为全集
                    tmp = MetaUtil::i(tmp, domMap[pred]);
                }
            }
            tmp.insert(u);
            if (tmp != domMap[u]) {
                domMap[u] = tmp;
                flag = true;
            }
        }
    }

}

void naiveidom() {
    using NodePtr = GraphNode<T>*;  
    for (int i = 0; i < this->size(); ++i) {
        NodePtr u = this->nodeVec[i];
        assert(domMap.count(u));
        for (NodePtr v : domMap[u]) {
            assert(domMap.count(v));
            std::unordered_set<NodePtr> tmp = MetaUtil::e(domMap[u], MetaUtil::i(domMap[v], domMap[u]));
            if (tmp.size() == 1 && tmp.count(u)) {
                idomMap[u] = v;
                tree[v].push_back(u);
                printf("INFO: Setting idom %d --> %d\n", v->getData(), u->getData());
                break;
            }
        }
    }
}

void naivefrontier() {
    using NodePtr = GraphNode<T>*; 
    for (NodePtr node : this->nodeVec) {
        std::vector<NodePtr> preds = node->getPred();
        if (preds.size() < 2) continue;
        for (NodePtr pred : preds) {
            NodePtr runner = pred;
            // x 是 node 的前驱的必经结点，但不是 node 的严格必经结点。
            // 则 node 属于 x 的必经节点边界
            while (runner != this->root && runner != idomMap[node]) {
                domFrtMap[runner].insert(node);
                runner = idomMap[runner];
            }
        }
    }
}

public:

DominateTree(const Graph<T>& g) : Graph<T>(g) {
    naivedom();
    naiveidom();
    naivefrontier();
}

DominateTree() : Graph<T>() { }

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

std::vector<T> getDominateFrontierVec(T x) {
    std::unordered_set<GraphNode<T>*>& frontier = domFrtMap[this->getNode(x)];
    std::vector<T> result;
    for (auto node : frontier) result.push_back(node->getData());
    return result;
}

std::unordered_set<T> getDominateFrontierSet(T x) {
    std::unordered_set<GraphNode<T>*>& frontier = domFrtMap[this->getNode(x)];
    std::unordered_set<T> result;
    for (auto node : frontier) result.insert(node->getData());
    return result;
}

T getIDom(T x) {
    GraphNode<T>* node = this->nodeVec[this->idxMap[x]];
    if (idomMap.count(node)) return this->idomMap[node]->getData();
    return NULL;
}

std::vector<T> getNext(T x) {
    return this->unpack(tree[this->getNode(x)]);
}

T getPrev(T x) {
    return idomMap[this->getNode(x)]->getData();
}

std::vector<T> getDoms(T x) {
    std::unordered_set<GraphNode<T>*>& doms = domMap[this->getNode(x)];
    std::vector<T> result;
    for (auto node : doms) result.push_back(node->getData());
    return result;
}

DominateTree<T>& compute() {
    naivedom();
    naiveidom();
    naivefrontier();
    return *this;
}

DominateTree<T>& clear() {
    tree.clear();
    domMap.clear();
    idomMap.clear();
    domFrtMap.clear();
    Graph<T>::clear();
    return *this;
}

DominateTree<T>& dump() {
    Graph<T>::dump();
    
    printf("\nDOM INFO: \n");
    for (int i = 0; i < this->size(); ++i) {
        GraphNode<T>* cur = this->nodeVec[i];
        T data = cur->getData();
        printf("node: id = %d[0x%lx]: [", this->getIdx(data), data);
        for (auto node : domMap[cur])
            printf("id = %d(0x%lx), ", this->getIdx(node->getData()), node->getData());
        printf("]\n");
    }
    printf("\nIDOM INFO: \n");
    for (int i = 0; i < this->size(); ++i) {
        T data = this->nodeVec[i]->getData();
        printf("id = %d[0x%lx] idom id = %d[0x%lx] \n", this->getIdx(getIDom(data)), getIDom(data), this->getIdx(data), data);
    }
    printf("\n");
    printf("\nDOM FRONTITER INFO: \n");
    for (int i = 0; i < this->size(); ++i) {
        GraphNode<T>* cur = this->nodeVec[i];
        T data = cur->getData();
        printf("node: id = %d[0x%lx]: [", this->getIdx(data), data);
        for (auto node : domFrtMap[cur])
            printf("id = %d(0x%lx), ", this->getIdx(node->getData()), node->getData());
        printf("]\n");
    }
    printf("\n");
    return *this;
}

};

} // namespace MetaTrans

