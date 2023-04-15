#include "MetaTrans.h"

#include <unordered_map>

namespace MetaTrans {


class InstMatchResult {
private:
protected:

std::unordered_map<MetaInst*, MetaInst*> map;

public:

InstMatchResult();

InstMatchResult& add(MetaInst* u, MetaInst* v);

MetaInst* get(MetaInst* key);

std::unordered_map<MetaInst*, MetaInst*>::iterator begin();

std::unordered_map<MetaInst*, MetaInst*>::iterator end();

};

class BBMatchResult {
private:
protected:

std::unordered_map<MetaBB*, MetaBB*> map;

public:

BBMatchResult();

BBMatchResult& add(MetaBB* u, MetaBB* v);

MetaBB* get(MetaBB* key);

std::unordered_map<MetaBB*, MetaBB*>::iterator begin();

std::unordered_map<MetaBB*, MetaBB*>::iterator end();
    
};

// Matching addressing code for two load instruction.
class MetaAddressMatcher {
private:

    MetaAddressMatcher& matchFor(MetaInst* ir);

protected: 

    MetaInst *asb, *ir;

    MetaBB* irbb;

    std::unordered_map<MetaInst*, MetaInst*> instMap;

    std::unordered_set<MetaInst*> matchedSet;
    
    std::unordered_map<uint64_t, CodePiecePair> codeMap;

public:
    
    MetaAddressMatcher();

    MetaAddressMatcher& setAsmInst(MetaInst* i);

    MetaAddressMatcher& setIrInst(MetaInst* i);

    MetaAddressMatcher& setIrBB(MetaBB* bb);

    virtual MetaAddressMatcher& match();

    std::vector<CodePiecePair> getResult();

    std::unordered_map<uint64_t, CodePiecePair>& getResultMap();
    
    bool matched(MetaInst* inst);
    
};

class MetaBBMatcher {

protected:

MetaFunction *x, *y;

std::unordered_map<MetaBB*, MetaBB*> bbMap;

std::unordered_map<MetaBB*, MetaBB*> visitedMap;

std::unordered_set<MetaBB*> matchedSet;

std::unordered_set<MetaBB*> visitedSet;


public:

MetaBBMatcher();

MetaBBMatcher& setAsmMetaFunc(MetaFunction* f);

MetaBBMatcher& setIrMetaFunc(MetaFunction* f);

virtual MetaBBMatcher& match() = 0;

std::unordered_map<MetaBB*, MetaBB*>& getResult();

};


class LinearMetaBBMatcher : public MetaBBMatcher {

private:

LinearMetaBBMatcher& matchNextBB(int& i, int& j, std::vector<MetaBB*>& bbs_x, std::vector<MetaBB*>& bbs_y, std::unordered_set<MetaBB*>& visited_x, std::unordered_set<MetaBB*>& visited_y);

std::pair<MetaInst*, MetaInst*> matchInstGraph(MetaBB& u, MetaBB& v);

protected:
public:

LinearMetaBBMatcher();

MetaBBMatcher& match() override;

LinearMetaBBMatcher& matchInst();

};


class CraphBasedBBMatcher : public MetaBBMatcher {

private:

void match(MetaBB* u, MetaBB* v, MetaBB* x, MetaBB* y);

MetaBB* findJoint(MetaBB* split, MetaBB* merge);

bool isPartOfLoop(MetaBB* bb);

bool isStartOfLoop(MetaBB* bb);

bool isEndOfLoop(MetaBB* bb);

MetaBB* next(MetaBB* bb);

public:

CraphBasedBBMatcher();

MetaBBMatcher& match() override;

};


} // namespace MetaTrans