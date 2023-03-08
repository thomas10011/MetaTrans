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


class MetaBBMatcher {

protected:

MetaFunction *x, *y;

std::unordered_map<MetaBB*, MetaBB*> bbMap;

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


} // namespace MetaTrans