#define DEBUG_TYPE "opcodeCounter" // debug name

#include <iostream>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include <typeinfo>
#include <algorithm>

#include <set>
#include <map>

using namespace llvm;

namespace {

    struct CountOpcode : public FunctionPass {

        CountOpcode () : FunctionPass(ID) {

        }

        /* data */
        std::map<std::string, int> opcodeCounter;
        std::set<Value*> operandSet;
        
        static char ID;

        bool runOnFunction(Function &F) override {
            outs() << "current function is: " << F.getName() << "\n";
            int sum = 0;
            for (Function::iterator bb = F.begin(); bb != F.end(); ++bb) {
                for (BasicBlock::iterator i = bb->begin(); i != bb->end(); ++i) {
                    if (opcodeCounter.find(i->getOpcodeName()) == opcodeCounter.end()) {
                        opcodeCounter[i->getOpcodeName()] = 1;
                    }
                    else {
                        opcodeCounter[i->getOpcodeName()]++;
                    }
                    Use* edges = i->getOperandList();
                    while (edges) {
                        Value* value = edges->get();
                        Type* valueType = value->getType();
                        operandSet.insert(value);
                        edges = edges->getNext();
                        outs() << "reference to type: ";
                        valueType->print(outs());
                        outs() << "\n";
                        outs() << "reference to Value Type: " << Value::ValueTy(value->getValueID()) << "\n";
                        if (auto *Inst = dyn_cast<Instruction>(value)) {
                            sum++;
                        }
                    }
                }
            }
            std::map<std::string, int>::iterator i = opcodeCounter.begin();
            std::map<std::string, int>::iterator e = opcodeCounter.end();
            int opCodeSum = 0;
            while (i != e) {
                opCodeSum += i->second;
                outs() << i->first << " : " << i->second << "\n";
                i++; 
            }
            outs() << "the number of opcode is: " << opCodeSum << "\n";
            outs() << "the number of operand is: " << operandSet.size() << "\n";
            outs() << "the number of instruction is: " << sum << "\n";
            opcodeCounter.clear();
        }
    };
    
}


char CountOpcode::ID = 0;

static RegisterPass<CountOpcode> X("count-opcode", "count opcode pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);