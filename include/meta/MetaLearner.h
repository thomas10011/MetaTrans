#include "MetaTrans.h"
#include <string>
#include <unordered_map>


namespace MetaTrans {

class MetaLearner {
private:

MetaTrans::MappingTable* MapTable;

std::unordered_map<MetaInst*, int> equivClassTagMap;
std::unordered_map<MetaInst*, int> matchedMap;
std::unordered_map<MetaInst*, int> trainedMap;
std::unordered_map<MetaInst*, int> fusedMap;
std::unordered_map<MetaInst*, std::vector<MetaInst*>> matchedInstMap;


public:

std::vector<MetaInst*> findTheSameInst(MetaInst* inst, MetaBB *bb);

void setMapTable(MetaTrans::MappingTable* table);

MetaBB* trainLoad(MetaBB* cur, MetaBB* irbb);
MetaBB* trainBB(MetaBB* cur, MetaBB* irbb);

// 为inst构建Equivalent Class
void buildEquivClass(MetaInst* inst);
void resetEquivClass(MetaInst* inst);

bool ifMatched(MetaInst* inst);
void setTypeSrc(MetaInst* inst, std::vector<std::vector<int>> src);
MetaInst* fuseInst(std::vector<InstType> vec, MetaInst* inst, std::vector<MetaInst*> &fused);

int ifFind(MetaInst* inst, std::vector<MetaInst*> vec);
int ifFind(MetaOperand* inst, std::vector<MetaOperand*> vec);
int ifFind(MetaOperand* inst, std::vector<MetaInst*> vec);

void dumpMapping(std::string mapping);

// used to traverse MT Meta
MetaInst* updateMappingTable(MetaInst* cur, std::string mapstr, std::string asmInst, std::string irInst,  int index, std::string firstASM);

MetaInst& buildMapping(MetaInst* cur, std::vector<MetaInst*> fused, std::string ASMorIR);
MetaInst& buildMapping(MetaInst* cur, MetaInst* inst);

std::string findDuplicateOperands(std::vector<MetaOperand*> vec);

std::string buildOperandMapping(MetaInst* cur, std::vector<MetaInst*> Fuse, std::string ASMorIR);
std::string buildOperandMapping(MetaInst* cur, MetaInst* inst);

MetaInst& trainInst(MetaInst* cur, MetaInst* irinst);
std::vector<MetaInst*> findMatchedInst(MetaInst* cur, std::vector<MetaInst*> irvec);
int getFuseID(MetaInst* cur);

MetaInst* trainEquivClass(MetaInst* cur, MetaInst* irinst);
MetaInst* QualifiedForMatch(MetaInst* cur);
std::vector<MetaInst*> getMatchedInst(MetaInst* cur);
MetaInst* trainControlFlow(MetaInst* cur, MetaInst* irinst);

void pushMatchedInst(MetaInst* key, MetaInst* value);
bool ifOrderMatters(InstType type);

};

} // end namespace MetaTrans
