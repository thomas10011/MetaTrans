#pragma once

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/JSON.h"
#include "llvm/IR/DerivedUser.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Operator.h"

#include "MetaData.h"
#include "MetaStream.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>


namespace MetaTrans {

class MetaInst;

class MetaBB;

class MetaFunction;

class MetaFunctionBuilder;   

struct MetaFunctionPass;

enum InstType {
    LOAD,
    STORE,
    COMPARE,
    CALL,
    BRANCH,
    JUMP,
    // phi node.
    PHI,
    ADD,
    SUB,
    MUL,
    DIV,
    REMAINDER,
    AND,
    OR,
    XOR,
    SHIFT,
    NEG,
    RET,
    // represent allocation for a data.
    ALLOCATION,
    ADDRESSING,
    EXCEPTION,
    SWAP,
    MIN,
    MAX,
    SQRT,
    FENCE,
    CONVERT,
    HINT,
    MOV,
    // Arch-specific
    CSR,
    //Sign-inject
    SIGN
};

// struct for path (data compute/addressing/control flow)
struct Path {
    MetaInst *firstNode;
    int type; // (0 data computing 1 addressing 2 control flow)
    int numLoad;
    int numStore;
    int numPHI;
    int numGEP;
    bool operator==(const Path& rhs) const
    {
    return type == rhs.type && numLoad == rhs.numLoad &&
            numStore == rhs.numStore && numPHI == rhs.numPHI &&
            numGEP == rhs.numGEP;
    }
};

class MetaOperand {
private:
protected:

    int id = -1;

    // record those instructions that use this operand.
    std::vector<MetaInst*> users;

public:

    MetaOperand& setID(int id);

    MetaOperand& addUser(MetaInst* user);

    // remove user from user list.
    MetaOperand& removeUser(MetaInst* user);

    std::vector<MetaInst*> getUsers();

    std::vector<MetaInst*>::iterator users_begin();

    std::vector<MetaInst*>::iterator users_end();

    int getID();

    virtual bool isMetaConstant();

    virtual bool isMetaArgument();

    virtual bool isMetaInst();

    virtual ~MetaOperand();

    virtual std::string toString();

};

class MetaConstant : public MetaOperand {
private:
protected:

    std::string name;

    bool global;

    bool imm;

    DataType type;

    DataUnion value;

    MetaFunction* parent;

public:
    
    MetaConstant();

    MetaConstant(MetaFunction* p);

    virtual ~MetaConstant();

    MetaConstant(DataType ty);

    MetaConstant& setDataType(DataType ty);

    DataType getDataType();

    DataUnion getValue();

    MetaFunction* getParent();

    MetaConstant& setValue(int8_t v);

    MetaConstant& setValue(int16_t v);
    
    MetaConstant& setValue(int32_t v);

    MetaConstant& setValue(int64_t v);

    MetaConstant& setValue(float v);
    
    MetaConstant& setValue(double v);

    MetaConstant& setParent(MetaFunction* p);

    MetaConstant& setName(std::string name);

    MetaConstant& setGlobal(bool v);

    MetaConstant& setImm(bool v);

    bool isGlobal();

    bool isImm();
    
    virtual bool isMetaConstant() override;

    std::string getName();

    std::string virtual toString() override;

};

class MetaArgument : public MetaOperand {
private: 
protected:

    int width;

    int argIndex;

    int offset;

    DataType type; 

    MetaFunction* parent;

public:

    MetaArgument();

    MetaArgument(MetaFunction* p);

    ~MetaArgument();

    MetaArgument(DataType ty); 

    MetaArgument& setArgIndex(int i);

    MetaArgument& setOffset(int o);

    MetaArgument& setArgType(DataType ty);

    MetaArgument& setWidth(int w);

    MetaArgument& setParent(MetaFunction* f);

    int getArgIndex();

    int getOffest();

    int getWidth();

    DataType getArgType();

    MetaFunction* getParent();

    virtual bool isMetaArgument() override;

    std::string virtual toString() override;
};

class MetaInst : public MetaOperand {
private:
    
    std::string originInst;

protected:

    MetaBB* parent;

    // a vector to indicate the real type of a instruction.
    std::vector<InstType> type;

    std::vector<std::vector<int>> typeSrc;

    std::vector<MetaOperand*> operandList;

    InstMetaData metaData;

    std::set<ColorData> colors; // color, type(0 data computing 1 addressing 2 control flow)

    std::vector<Path*> paths ; // fitst node of path, (0 data computing 1 addressing 2 control flow)

    int EquivClassTag;    

    bool Matched = false;    

    unsigned long hashCode = 0;

    std::string dataRoot = "";

    std::string globalSymbolName = "";

    std::vector<MetaInst*> MatchedInst;

    int FuseID;

    public:

    MetaInst();

    virtual ~MetaInst();

    MetaInst(std::vector<InstType> ty);

    MetaInst& setOriginInst(std::string name);

    MetaInst& setInstType(std::vector<InstType> ty);

    MetaInst& setInstType(InstType ty);

    MetaInst& setTypeSrc(std::vector<std::vector<int>>);

    MetaInst& addInstType(InstType ty);

    MetaInst& setParent(MetaBB* bb);

    void dumpPath(int index);

    MetaInst& addOperand(MetaOperand* op);

    std::string getOriginInst();

    virtual MetaInst& buildFromJSON(llvm::json::Object JSON, std::unordered_map<int64_t, MetaBB*>& tempBBMap, std::unordered_map<int64_t, MetaOperand*>& tempOperandMap);

    int getOperandNum();

    MetaBB* getParent();

    std::vector<InstType> getInstType();

    std::vector<MetaOperand*>& getOperandList();

    std::vector<MetaOperand*>::iterator op_begin();
    
    std::vector<MetaOperand*>::iterator op_end();
        
    // return true if has absolute same type (same number of types and same type) with i.
    bool hasStrictSameType(MetaInst* i);

    // return true if has one same type with i.
    bool hasRelaxedSameType(MetaInst* i);
    
    // return true if ty is in type vector.
    bool isType(InstType ty);

    // return trie of this instruction only has single type amd same with ty.
    bool isSingleType(InstType ty);

    bool virtual isMetaInst() override;

    std::string virtual toString() override;

    bool virtual isMetaPhi();

    bool virtual isLoad();

    bool virtual isStore();

    void addColor(int c, int t);

    std::set<ColorData>& getColors();

    bool hasColor(int c);

    unsigned long getHashcode();

    MetaInst& setHashcode(unsigned long hashCode);

    std::string getDataRoot();

    std::string getGlobalSymbolName();

    MetaInst& setDataRoot(std::string s);

    MetaInst& setGlobalSymbolName(std::string s);
    
    std::vector<Path *> &getAllPath();

    Path* getPath(int type);

    MetaInst& addToPath(Path* p);
    
    std::vector<MetaInst *> findTheSameInst(MetaBB *bb);

    void buildEquivClass();

    void resetEquivClass();

    bool ifMatched();

    std::vector<MetaInst*> findMatchedInst(std::vector<MetaInst*> irvec);

    MetaInst& trainInst(MetaInst* irinst);

    MetaInst& buildMapping(std::vector<MetaInst*> fused, std::string ASMorIR);

    MetaInst& buildMapping(MetaInst* inst);

    std::string buildOperandMapping(MetaInst* inst);

    std::string buildOperandMapping(std::vector<MetaInst*> fused, std::string ASMorIR);

    int getFuseID();

    MetaInst* QualifiedForMatch();

    MetaInst* trainControlFlow(MetaInst* irinst);

    MetaInst* trainEquivClass(MetaInst* inst);

    std::vector<MetaInst*> getMatchedInst();

    MetaInst* updateMappingTable(std::string mapstr, std::string asmInst, std::string ir, int index);


    


};

/// represent a phi node.
class MetaPhi : public MetaInst {
private:
protected:

    std::unordered_map<MetaBB*, MetaOperand*> bbValueMap;

public:

    MetaPhi(std::vector<InstType> ty); 

    MetaPhi();

    MetaPhi& addValue(MetaBB* bb, MetaOperand* op);

    bool equals(MetaPhi* phi);

    MetaOperand* getValue(MetaBB* bb);

    int getMapSize();

    std::unordered_map<MetaBB*, MetaOperand*>::iterator begin();

    std::unordered_map<MetaBB*, MetaOperand*>::iterator end();

    bool virtual isMetaPhi() override;

    virtual MetaInst& buildFromJSON(llvm::json::Object JSON, std::unordered_map<int64_t, MetaBB*>& tempBBMap, std::unordered_map<int64_t, MetaOperand*>& tempOperandMap) override;

    std::string virtual toString() override;

    bool virtual isLoad() override;

    bool virtual isStore() override;
};

class MetaBB {
private:
protected:

    int id;

    std::vector<MetaInst*> instList;
    
    // point to next BB if exists.
    std::vector<MetaBB*> successors;

    // first non phi instruction.
    MetaInst* entry;

    // each bb end with a terminator.
    MetaInst* terminator;
    
    // record parent scope
    MetaFunction* parent;

    std::vector<int> features;

    double modular;

public:

    MetaBB();

    MetaBB(MetaFunction* parent);

    MetaBB(std::string JSON);

    MetaBB(MetaFunction* parent, std::string JSON);

    ~MetaBB(); 

    // Build a new instruction and append to instList.
    MetaInst* buildInstruction(std::vector<InstType> ty);

    MetaInst* buildInstruction();

    MetaPhi* buildPhi(bool insertAtHead = false);

    MetaBB& addPhi(MetaPhi* phi, bool insertAtHead = false);

    MetaBB& addInstruction(MetaInst* inst);

    MetaBB& addNextBB(MetaBB* next);

    MetaBB& addFeature(int f);

    MetaBB& setEntry(MetaInst* inst);

    MetaBB& setTerminator(MetaInst* inst);

    MetaBB& setParent(MetaFunction* mF);

    MetaBB& setID(int id);

    MetaBB& buildInstFromJSON(llvm::json::Object JSON, std::unordered_map<int64_t, MetaBB*>& tempBBMap, std::unordered_map<int64_t, MetaOperand*>& tempOperandMap);

    MetaBB& buildInstGraphFromJSON(llvm::json::Object JSON, std::unordered_map<int64_t, MetaBB*>& tempBBMap, std::unordered_map<int64_t, MetaOperand*>& tempOperandMap);

    std::vector<int> getFeature();

    std::vector<MetaBB*> getNextBB();

    MetaBB* getNextBB(int index);

    MetaInst* getEntry();

    MetaInst* getTerminator();

    MetaFunction* getParent();

    int getID();

    std::vector<MetaInst*>& getInstList();

    int getInstNum();

    std::string toString();

    double getModular();

    double similarity(MetaBB& bb);

    Stream<MetaInst*> stream();

    std::vector<MetaInst*>::iterator begin();

    std::vector<MetaInst*>::iterator end();

    std::vector<MetaInst*>::iterator inst_begin();

    std::vector<MetaInst*>::iterator inst_end();

    MetaInst* inst_last();

    std::vector<MetaBB*>::iterator next_begin();

    std::vector<MetaBB*>::iterator next_end();

    MetaBB* trainBB(MetaBB* irbb);

};

class MetaFunction {
private:
    
    void init(llvm::json::Object& object);

protected:

    // a function should contains a set of constants.
    std::vector<MetaConstant*> constants;
    // arguments, as well.
    std::vector<MetaArgument*> args;
    // basic blocks.
    std::vector<MetaBB*> bbs;

    // CFG root
    MetaBB* root;

    std::string funcName;

    DataType returnType;
    
    int stackSize;

    int argNum;

public:

    MetaFunction();

    MetaFunction(std::string JSON);

    MetaFunction(llvm::json::Object& JSON);

    MetaFunction& addConstant(MetaConstant* c);
    
    MetaFunction& addArgument(MetaArgument* a);

    MetaFunction& setRoot(MetaBB* rootBB);

    MetaFunction& setFunctionName(std::string name);

    MetaFunction& setStackSize(int s);

    MetaFunction& setReturnType(DataType ty);

    MetaFunction& expandStackSize(int s);

    MetaArgument* getArgument(int index);

    std::string getFunctionName();

    int getConstNum();

    int getArgNum();

    DataType getReturnType();

    // create a new bb at the end of bb list.
    MetaBB* buildBB();

    MetaBB* getRoot();

    MetaArgument* buildArgument();

    MetaConstant* buildConstant();

    std::string toString();

    Stream<MetaBB*> stream();
    
    std::vector<MetaBB*>& getBB();

    std::vector<MetaBB*>::iterator begin();

    std::vector<MetaBB*>::iterator end();

    std::vector<MetaBB*>::iterator bb_begin();

    std::vector<MetaBB*>::iterator bb_end();

    std::vector<MetaConstant*>::iterator const_begin();

    std::vector<MetaConstant*>::iterator const_end();

    std::vector<MetaArgument*>::iterator arg_begin();

    std::vector<MetaArgument*>::iterator arg_end();

};


class MappingTable{


public:

    // MappingName[0]: TableMata.mapping
    // MappingName[1]: 1-N.mapping
    // MappingName[2]: 2-N.mapping
    // MappingName[3]: 3-N.mapping
    // ...
    std::vector<std::string> MappingName;

    // <Inst Name, Bitmap>
    // The bitmap records which mapping table contains such instruction
    // 0001 -> 1-N
    // 0010 -> 2-N
    // 0100 -> 3-N
    // O110 -> 2-N & 3-N
    // 0011 -> 1-N & 2-N
    std::map<std::string, int> TableMata;

    // Max ASM Inst count in a mapping entry
    int max = 5;


    // MTable[0]: Empty
    // MTable[1]: 1-N Mapping
    // MTable[2]: 2-N Mapping
    // ...
    std::vector<std::map<std::string, std::string>> MTable;

    MappingTable* setName(std::string path);
    
    MappingTable* initTableMeta();

    int locateMappingTable(std::string InstName);

    MappingTable* loadMappingTable();

    std::string getTableName(int id);


};


class MetaUnit {

private:
protected:

    std::vector<MetaFunction*> funcs;

    std::vector<MetaConstant*> globalVar;


public:

    MetaUnit();

    MetaUnit& addFunc(MetaFunction* f);
    
    MetaUnit& addGlobalVar(MetaConstant* c);

    Stream<MetaFunction*> stream(); 

    std::vector<MetaFunction*>& getFuncList();

    std::vector<MetaConstant*>& getGlobalVarList();
    
    std::vector<MetaFunction*>::iterator begin();

    std::vector<MetaFunction*>::iterator end();

    std::string toString();


};



} // namespace MetaTrans

    