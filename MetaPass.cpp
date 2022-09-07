#include "meta/MetaPass.h"

namespace MetaTrans {
    
//===-------------------------------------------------------------------------------===//
/// Meta Function pass implementation, Will be invoked by LLVM pass manager.

    MetaFunctionPass::MetaFunctionPass() : FunctionPass(MetaFunctionPass::ID) {
        YamlUtil::test();
    }

    bool MetaFunctionPass::runOnFunction(Function & F) {
        outs() << "running MetaInst pass on function " << F.getName() << " ... " << "\n";
        MetaFunction* metaFunc = builder
                                    .setFunction(&F)
                                    .build();
        metaFuncs.push_back(metaFunc);
        return true;
    }

//===-------------------------------------------------------------------------------===//
/// Meta Function Builder implementation.

    MetaFunctionBuilder& MetaFunctionBuilder::createMetaElements() {
        // create all meta basic block and instructions recursively.
        for (auto bb = F->begin(); bb != F->end(); ++bb) {
            (*this).createMetaBB(*bb, *mF);
        }
        return *this;
    }

    MetaFunctionBuilder::MetaFunctionBuilder() : F(nullptr), mF(nullptr) { }

    MetaFunctionBuilder& MetaFunctionBuilder::clearAuxMaps() {
        bbMap.clear();
        instMap.clear();
        constantMap.clear();
        argMap.clear();
        return *this;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::setFunction(Function* F) {
        this->F = F;
        clearAuxMaps();
        return *this;
    }

    MetaFunction* MetaFunctionBuilder::build() {
        mF = new MetaFunction();
        (*this)
            .createMetaElements()
            .buildGraph();
        return mF;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::buildGraph() {
        // construct graph
        mF->setRoot(bbMap.find(&*(F->begin()))->second);
        for (Function::iterator bb = F->begin(); bb != F->end(); ++bb) {
            // we create graph for each basic block
            MetaBB* curBB = bbMap.find(&*bb)->second;
            
            for (BasicBlock::iterator i = bb->begin(); i != bb->end(); ++i) {
                // add this instruction to current basic block.
                MetaInst* curInst = instMap.find(&*i)->second;
                processOperand(curInst, &*i, curBB);
            }
        }
        
        for (auto bb = mF->bb_begin(); bb != mF->bb_end(); ++bb) {
            outs() << "successor number of " << *bb << " is " << (*bb)->getNextBB().size() << "\n";
            MetaUtil::printInstDependencyGraph(*bb);
        }
        outs() << "\n";
        return *this;
    }
    

    MetaFunctionBuilder& MetaFunctionBuilder::createMetaBB(BasicBlock& b, MetaFunction& f) {
        outs() << "creating instructions in Basic Block " << b;
        MetaBB* newBB = f.buildBB();
        assert(bbMap.insert({&b, newBB}).second);
        bool first_non_phi = true;
        for (auto i = b.begin(); i != b.end(); ++i) {
            outs() << "instruction with type: " << i->getOpcodeName() << "\n";
            (*this)
                .createMetaInst(*i, *newBB)
                .createMetaOperand(*i);
            if (first_non_phi && i->getOpcode() != Instruction::PHI) {
                newBB->setEntry(instMap[&*i]); first_non_phi = false;
            }
        }
        return *this;
    }
    
    MetaFunctionBuilder& MetaFunctionBuilder::createMetaInst(Instruction& i, MetaBB& b) {
        MetaInst* newInst = b.buildInstruction(MetaUtil::getInstType(i));
        assert(instMap.insert({&i, newInst}).second);
        return *this;
    }

    MetaFunctionBuilder& MetaFunctionBuilder::createMetaOperand(Instruction& i) {
        // create all arg and constant.
        for (auto op = i.op_begin(); op != i.op_end(); ++op) {
            Value* value = op->get();
            // Ugly, but works.
            if (Argument* a = dyn_cast<Argument>(value)) {
                if (MetaArgument* mA = MetaUtil::createValue(a, argMap))
                    mF->addArgument(mA);
            }
            else if (Constant* c = dyn_cast<Constant>(value)) {
                if (MetaConstant* mC = MetaUtil::createValue(c, constantMap))
                    mF->addConstant(mC);
            }
        }
        return *this;
    }

    void MetaFunctionBuilder::processOperand(MetaPhi* inst, PHINode* phi, MetaBB* curBB) {
        outs() << "processing meta phi node's operands..." << "\n";
        for (auto bb = phi->block_begin(); bb != phi->block_end(); ++bb) {
            Value* value = phi->getIncomingValueForBlock(*bb);
            MetaOperand* op = nullptr;
            if (Argument* arg = dyn_cast<Argument>(value)) {
                op = (MetaOperand*)argMap.find(arg)->second;
            }
            else if (Constant* c = dyn_cast<Constant>(value)) {
                op = (MetaOperand*)constantMap.find(c)->second;
            }
            else if (Instruction* i = dyn_cast<Instruction>(value)) {
                op = (MetaOperand*)instMap.find(i)->second;
            }
            assert(op);
            inst->addValue(bbMap.find(*bb)->second, op);
            outs() << "basic block: "<< bb << "; value: " << value << "; ";
        }
        outs() << "\n";
        
        for (auto op = phi->op_begin(); op != phi->op_end(); ++op) {
            Value* value = op->get();
            MetaUtil::printValueType(value); 
            outs() << "; operand number: " << op->getOperandNo() << "#" << "\n";
        }
    }

    void MetaFunctionBuilder::processOperand(MetaInst* inst, Instruction* curInst, MetaBB* curBB) {
        if (auto phi = dyn_cast<PHINode>(curInst)) {
            processOperand(inst, phi, curBB); return;
        }
        unsigned num_op = curInst->getNumOperands();
        outs() << "inst type: " << curInst->getOpcodeName() << "\n";
        for (auto op = curInst->op_begin(); op != curInst->op_end(); ++op, --num_op) {
            Value* value = op->get();
            MetaUtil::printValueType(value);
            if (Argument* arg = dyn_cast<Argument>(value)) {
                MetaArgument* metaArg =argMap.find(arg)->second;
                inst->addOperand((MetaOperand*)metaArg);
            }
            // create CFG here.
            else if (BasicBlock* bb = dyn_cast<BasicBlock>(value)) {
                auto it = bbMap.find(bb);
                (*curBB)
                    .addNextBB(it->second)
                    .setTerminator(inst);
            }
            else if (Constant* c = dyn_cast<Constant>(value)) {
                MetaConstant* metaCons = constantMap.find(c)->second;
                inst->addOperand((MetaOperand*)metaCons);
            }
            else if (Instruction* i = dyn_cast<Instruction>(value)) {
                MetaInst* usedInst = instMap.find(i)->second;
                inst->addOperand((MetaOperand*)usedInst);
            }
            outs() << "; operand number: " << op->getOperandNo() << "#" << "\n";
        }
        assert(!num_op);
    }

}

char MetaTrans::MetaFunctionPass::ID = 0;

static RegisterPass<MetaTrans::MetaFunctionPass> X("meta-trans", "MetaTrans Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);