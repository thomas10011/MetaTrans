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

class MetaUnitBuildContext;

class MetaUnit;

class MetaScope;

struct MetaFunctionPass;

class AddrMappingTable;

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

enum COLORTYPE {
    ADDRESSINGCOLOR,
    COMPUTING,
    CONTROLFLOW,
    LOADINST,
    STOREINST
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

class MetaScope {

private:

    MetaScope* parent;

    int id = -1;

protected:
public:
                MetaScope       ()                ;

    MetaScope&  setParentScope  (MetaScope* scope);

    MetaScope&  setID           (int id)          ;

    MetaScope*  getParentScope  ()                ;

    int         getID           ()                ;

    MetaScope* getRootScope     ()                ;

};

class MetaOperand {
private:
    
    MetaScope* parentScope = nullptr;

protected:

    int id = -1;

    // record those instructions that use this operand.
    std::vector<MetaInst*> users;

public:

    MetaOperand& setID(int id);

    MetaOperand& addUser(MetaInst* user);

    // remove user from user list.
    MetaOperand& removeUser(MetaInst* user);

    MetaOperand& setParentScope(MetaScope* scope);

    MetaOperand& registerToMetaUnit();

    MetaOperand& unregisterFromMetaUnit();

    MetaScope* getParentScope();

    MetaUnit& getMetaUnit();

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

    std::string valueStr;
    
    MetaPrimType primaryType;

public:
    
    MetaConstant();

    MetaConstant(MetaUnitBuildContext& context);

    MetaConstant(MetaFunction* p);

    virtual ~MetaConstant();

    MetaConstant(DataType ty);

    DataType getDataType();

    DataUnion getValue();

    MetaConstant& setValue(int8_t v);

    MetaConstant& setValue(int16_t v);
    
    MetaConstant& setValue(int32_t v);

    MetaConstant& setValue(int64_t v);

    MetaConstant& setValue(float v);
    
    MetaConstant& setValue(double v);

    MetaConstant& setName(std::string name);

    MetaConstant& setGlobal(bool v);

    MetaConstant& setImm(bool v);

    MetaConstant& setParentScope(MetaScope* scope);

    MetaConstant& setValueStr(std::string str);

    MetaConstant& setDataType(DataType ty);

    MetaConstant& setWidth(int w);

    MetaConstant& setType(MetaPrimType ty);

    bool isGlobal();

    bool isImm();
    
    virtual bool isMetaConstant() override;

    std::string getName();

    std::string getValueStr();

    std::string virtual toString() override;

};

class MetaArgument : public MetaOperand {
private: 
protected:

    int width;

    int argIndex;

    int offset;

    MetaPrimType type;

public:

    MetaArgument();

    MetaArgument(MetaFunction* p);

    ~MetaArgument();

    MetaArgument(DataType ty); 

    MetaArgument& setArgIndex(int i);

    MetaArgument& setOffset(int o);

    MetaArgument& setDataType(DataType ty);

    MetaArgument& setWidth(int w);

    MetaArgument& setPtrLevel(int l);

    MetaArgument& setParentScope(MetaScope* scope);

    MetaArgument& setType(MetaPrimType ty);

    int getArgIndex();

    int getOffest();

    int getWidth();

    MetaPrimType getType();

    virtual bool isMetaArgument() override;

    std::string virtual toString() override;
};

class MetaInst : public MetaOperand {
private:
    
    std::string originInst;

protected:

    // a vector to indicate the real type of a instruction.
    std::vector<InstType> type;

    std::vector<std::vector<int>> typeSrc;

    std::vector<MetaOperand*> operandList; // when `initOperands`, resize it to MAX_OPERAND_WITH_ARGUMENTS. so [0-2] is rs1-rs3, [3] is imm; [4-11] a0-a7 and [12-19] fa0-fa7 only for jal 

    InstMetaData metaData;

    ColorData color; // color, type(0 addressing 1 data computing 2 control flow)

    std::vector<Path*> paths ; // fitst node of path, (0 addressing 1 data computing 2 control flow)

    int EquivClassTag;    

    bool Matched = false;    

    unsigned long hashCode = 0;

    std::string dataRoot = "";

    std::string globalSymbolName = "";

    std::vector<MetaInst*> MatchedInst;

    int FuseID;

    uint64_t address;

    bool Trained = false;

    llvm::Instruction* TransInst = NULL;

    bool AddrGenFlag = false;

    int NumOperands = 0;

public:

    MetaInst();

    virtual ~MetaInst();

    MetaInst(std::vector<InstType> ty);

    MetaInst& setOriginInst(std::string name);

    MetaInst& setInstType(std::vector<InstType> ty);

    MetaInst& setInstType(InstType ty);

    MetaInst& setTypeSrc(std::vector<std::vector<int>>);

    MetaInst& addInstType(InstType ty);

    MetaInst& setParentScope(MetaScope* scope);

    void dumpPath(int index);

    // 创建固定个数的nullptr操作数
    MetaInst& initOperands(int num);

    // 将所有为nullptr的operand去除
    MetaInst& filterNullOperands();

    MetaInst& checkNullOperands();

    MetaInst& addOperand(MetaOperand* op);

    MetaInst& addOperandAt(MetaOperand* op, int index);

    MetaInst& addOperandAtLast(MetaOperand* op);

    // if nullptr exists in operandList, return false.
    // else return true
    bool checkOperands();

    std::string getOriginInst();

    virtual MetaInst& buildFromJSON(MetaUnitBuildContext& context);

    int getOperandNum();

    std::vector<InstType> getInstType();

    std::vector<MetaOperand*>& getOperandList();

    std::vector<MetaInst*> getOperandOnlyInstList();

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

    bool virtual isMetaCall();

    bool virtual isLoad();

    bool virtual isStore();

    bool virtual isMemOp();

    bool virtual isAddressing();

    bool virtual isComputing();

    bool virtual isControlFlow();

    void setColor(int c, int t);

    ColorData* getColor();

    unsigned long getHashcode();

    MetaInst& setHashcode(unsigned long hashCode);

    uint64_t getAddress();

    MetaInst& setAddress(uint64_t address);

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

    MetaInst* updateMappingTable(std::string mapstr, std::string asmInst, std::string ir, int index, std::string firstASM);

    llvm::Instruction* getTransInst();

    void setTransInst(llvm::Instruction* inst);

    MetaInst& setAddrGen(bool b);

    bool ifAddrGen();

    int getNumOperands();

    void setNumOperands(int i);

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

    virtual MetaInst& buildFromJSON(MetaUnitBuildContext& context) override;

    std::string virtual toString() override;

    bool virtual isLoad() override;

    bool virtual isStore() override;

    std::unordered_map<MetaBB*, MetaOperand*> getMapping();
};

class MetaCall : public MetaInst {
private:
protected:

    std::string funcName;

    MetaFunction* func;

public:

    MetaCall();

    MetaCall& setFuncName(std::string name);

    MetaCall& setMetaFunction(MetaFunction* mF);

    std::string getFuncName();

    MetaFunction* getMetaFunction();

    virtual MetaInst& buildFromJSON(MetaUnitBuildContext& context) override;

    bool virtual isMetaCall() override;

    std::string virtual toString() override;

};

class MetaBB : public MetaScope {
private:
protected:

    std::vector<MetaInst*> instList;
    
    // point to next BB if exists.
    std::vector<MetaBB*> successors;

    // first non phi instruction.
    MetaInst* entry;

    // each bb end with a terminator.
    MetaInst* terminator;

    std::vector<int> features;

    double modular;

public:

    MetaBB();

    MetaBB(MetaFunction* parent);

    ~MetaBB(); 

    // Build a new instruction and append to instList.
    MetaInst* buildInstruction(std::vector<InstType> ty);

    MetaInst* buildInstruction();

    MetaInst* buildCall();

    MetaPhi* buildPhi(bool insertAtHead = false);

    MetaBB& addPhi(MetaPhi* phi, bool insertAtHead = false);

    MetaBB& addInstruction(MetaInst* inst);

    MetaBB& addNextBB(MetaBB* next);

    MetaBB& addFeature(int f);

    MetaBB& setEntry(MetaInst* inst);

    MetaBB& setTerminator(MetaInst* inst);

    MetaBB& setParentScope(MetaScope* scope);

    MetaBB& buildInstFromJSON(MetaUnitBuildContext& context);

    MetaBB& buildInstGraphFromJSON(MetaUnitBuildContext& context);

    MetaBB& registerToMetaUnit();

    MetaBB& setID(int64_t id);

    std::vector<int> getFeature();

    int getNumLoad();

    int getNumStore();

    // 获取 load / store 的总数
    int getNumMemOp();

    int isLinearInCFG();

    int isSelfLoop();

    std::vector<MetaBB*> getNextBB();

    MetaBB* getNextBB(int index);

    MetaInst* getEntry();

    MetaInst* getTerminator();

    std::vector<MetaInst*>& getInstList();

    int getNumInst();

    std::string toString();

    double getModular();

    double similarity(MetaBB& bb);

    double memOpSimilarity(MetaBB* bb);

    Stream<MetaInst*> stream();

    std::vector<MetaInst*>::iterator begin();

    std::vector<MetaInst*>::iterator end();

    std::vector<MetaInst*>::iterator inst_begin();

    std::vector<MetaInst*>::iterator inst_end();

    MetaInst* inst_last();

    std::vector<MetaBB*>::iterator next_begin();

    std::vector<MetaBB*>::iterator next_end();

    MetaBB* trainBB(MetaBB* irbb);

    MetaUnit& getMetaUnit();

    MetaBB& swapSuccessors();

};

class MetaFunction : public MetaScope {
private:
protected:

    // a function should contains a set of constants.
    std::vector<MetaConstant*> constants;
    // arguments, as well.
    std::vector<MetaArgument*> args;
    // basic blocks.
    std::vector<MetaBB*> bbs;

    // CFG root and exit point.
    MetaBB* root;
    MetaBB* exit;

    std::string funcName;

    DataType returnType;
    
    int stackSize;

    int argNum;

public:

    MetaFunction();

    MetaFunction(MetaUnitBuildContext& context);

    MetaFunction& addConstant(MetaConstant* c);
    
    MetaFunction& addArgument(MetaArgument* a);

    MetaFunction& setRoot(MetaBB* rootBB);

    MetaFunction& setExit(MetaBB* rootBB);

    MetaFunction& setFunctionName(std::string name);

    MetaFunction& setStackSize(int s);

    MetaFunction& setReturnType(DataType ty);

    MetaFunction& expandStackSize(int s);

    MetaFunction& setParentScope(MetaScope* scope);

    MetaFunction& registerToMetaUnit();

    MetaArgument* getArgument(int index);

    std::string getFunctionName();

    int getConstNum();

    int getArgNum();

    DataType getReturnType();

    MetaUnit& getMetaUnit();

    // create a new bb at the end of bb list.
    MetaBB* buildBB();

    MetaBB* getRoot();

    MetaBB* getExit();

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


class CodePiece {
private:

std::vector<std::string> instList;

public:

CodePiece();

CodePiece(std::string s);

CodePiece(std::vector<std::string> init);

CodePiece& addInst(std::string inst);

CodePiece& clear();

uint64_t hashCode();

std::string toString();

};

typedef std::pair<CodePiece, CodePiece> CodePiecePair;


// Singleton Pattern
class AddrMappingTable {

private:

    static AddrMappingTable* table;

    std::unordered_map<uint64_t, CodePiecePair> map;
    
    AddrMappingTable();
    
    ~AddrMappingTable();

	AddrMappingTable(const AddrMappingTable&);

	AddrMappingTable& operator=(const AddrMappingTable&);

public:
    
    static AddrMappingTable& getInstanceRef();

    static AddrMappingTable* getInstance();

    AddrMappingTable& update(const std::unordered_map<uint64_t, CodePiecePair>& m);

    AddrMappingTable& update(const std::vector<CodePiecePair>& pairs);

    AddrMappingTable& addMapping(CodePiece asmCodes, CodePiece irCodes);

    AddrMappingTable& flush();

};


class MappingTable {

public:

    // MappingName[0]: TableMeta.mapping
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
    std::map<std::string, int> TableMeta;

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

    MappingTable* updateTableMeta();


};


class MetaUnit : public MetaScope {

private:

    std::vector<MetaOperand*> operands;

    std::vector<MetaInst*> insts;

    std::vector<MetaScope*> scopes;

protected:

    std::vector<MetaFunction*> funcs;

    std::vector<MetaConstant*> globalVar;

public:

    MetaUnit();

    MetaUnit(std::string& JSON);

    ~MetaUnit();

    MetaUnit& addFunc(MetaFunction* f);
    
    MetaUnit& addGlobalVar(MetaConstant* c);
    
    MetaUnit& registerOperand(MetaOperand* operand);

    MetaUnit& unregisterOperand(MetaOperand* operand);

    MetaUnit& registerScope(MetaScope* scope);

    MetaUnit& unregisterScope(MetaScope* scope);

    MetaUnit& fillID(bool fillAddress = true);

    Stream<MetaFunction*> func_stream(); 

    Stream<MetaConstant*> global_stream(); 

    Stream<MetaInst*> inst_stream(); 

    std::vector<MetaFunction*>& getFuncList();

    std::vector<MetaConstant*>& getGlobalVarList();
    
    std::vector<MetaFunction*>::iterator begin();

    std::vector<MetaFunction*>::iterator end();

    std::string toString();


};

class MetaUnitBuildContext {
private:

    std::string JSON;

    llvm::json::Object object;

    std::vector<llvm::json::Object> objectStack;

    std::vector<MetaScope*> scopeStack;
    
    std::unordered_map<int64_t, MetaOperand*> operandMap;

    std::unordered_map<int64_t, MetaScope*> scopeMap;

    MetaScope* scope;
    
public:

    MetaUnitBuildContext(std::string);
    
    llvm::json::Object getHoldObject();

    llvm::json::Object getJsonObject(std::string key);

    llvm::json::Array getJsonArray(std::string key);

    MetaScope* getScope(int64_t id);

    MetaScope* getCurScope();
    
    MetaUnitBuildContext& setCurScope(MetaScope* scope);

    MetaUnitBuildContext& setHoldObject(llvm::json::Object& o);

    MetaUnitBuildContext& setHoldObject(llvm::json::Object* o);

    MetaUnitBuildContext& addMetaOperand(int64_t id, MetaOperand* operand);

    MetaUnitBuildContext& addMetaScope(int64_t id, MetaScope* scope);

    MetaUnitBuildContext& saveContext();

    MetaUnitBuildContext& restoreContext();

    MetaOperand* getMetaOperand(int64_t id);

    MetaConstant* getMetaConstant(int64_t id);

    MetaInst* getMetaInst(int64_t id);

    MetaBB* getMetaBB(int64_t id);

    MetaFunction* getMetaFunction(int64_t id);

};


} // namespace MetaTrans

    