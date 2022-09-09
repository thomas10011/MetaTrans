#include "meta/utils.h"

namespace MetaTrans {

//===-------------------------------------------------------------------------------===//
/// Yaml Util implementation.

    int YamlUtil::test() {
        YAML::Node config = YAML::LoadFile("ir.yaml");
        // std::cout << "name:" << config["name"].as<std::string>() << std::endl;
        // std::cout << "sex:" << config["sex"].as<std::string>() << std::endl;
        // std::cout << "age:" << config["age"].as<int>() << std::endl;

        // for (auto foo : config["skills"]) {
        //     std::cout << foo.first.as<std::string>() << ":" << foo.second << std::endl;
        // }
        std::cout << "test file config size: " << config.size() << std::endl;
        for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
            if (it->second.Type() == YAML::NodeType::Scalar) {
                std::cout << it->second.as<std::string>() << std::endl;
            }
            else if (it->second.Type() == YAML::NodeType::Sequence) {
                for (auto v : it->second) {
                    std::cout << v << "....\n";
                }
            }
            else if (it->second.Type() == YAML::NodeType::Map) {

            }
        }

        // YamlUtil::parseMapConfig("test.yaml");

        return 0;
    }

    std::unordered_map<std::string, InstType>
    YamlUtil::str_inst_type_map = {
        { "LOAD",           InstType::LOAD       },
        { "STORE",          InstType::STORE      },
        { "COMPARE",        InstType::COMPARE    },
        { "CALL",           InstType::CALL       },
        { "BRANCH",         InstType::BRANCH     },
        { "JUMP",           InstType::JUMP       },
        { "PHI",            InstType::PHI        },
        { "ADD",            InstType::ADD        },
        { "SUB",            InstType::SUB        },
        { "MUL",            InstType::MUL        },
        { "DIV",            InstType::DIV        },
        { "REMAINDER",      InstType::REMAINDER  },
        { "AND",            InstType::AND        },
        { "OR",             InstType::OR         },
        { "XOR",            InstType::XOR        },
        { "SHIFT",          InstType::SHIFT      },
        { "NEG",            InstType::NEG        },
        { "RET",            InstType::RET        },
        { "ALLOCATION",     InstType::ALLOCATION },
        { "ADDRESSING",     InstType::ADDRESSING },
        { "EXCEPTION",      InstType::EXCEPTION  },
        { "SWAP",           InstType::SWAP       },
        { "MIN",            InstType::MIN        },
        { "MAX",            InstType::MAX        },
        { "SQRT",           InstType::SQRT       },
        { "FENCE",          InstType::FENCE      },
        { "CONVERT",        InstType::CONVERT    },
        { "HINT",           InstType::HINT       },
    };

//===-------------------------------------------------------------------------------===//
/// Meta Util implementation.

    // this function is used to print the subclass of a Value in INSTRUCTION level.
    void MetaUtil::printValueType(Value* value) {
        outs() << "value address: " << value << ";";
        if (dyn_cast<Argument>(value)) { 
            outs() << " real type: Argument";
        }
        else if (dyn_cast<BasicBlock>(value)) {
            outs() << " real type: BasicBlock";
        }
        else if (dyn_cast<InlineAsm>(value)) {
            outs() << " real type: nlineAsm";
        }
        else if (dyn_cast<MetadataAsValue>(value)) {
            outs() << " real type: MetadataAsValue";
        }
        else if (dyn_cast<Constant>(value)) {
            outs() << " real type: Constant";
        }
        else if (dyn_cast<MemoryAccess>(value)) {
            outs() << " real type: MemoryAccess";
        }
        else if (dyn_cast<Instruction>(value)) {
            outs() << " real type: Instruction";
        }
        else if (dyn_cast<Operator>(value)) {
            outs() << " real type: Operator";
        }
    }
    
    void MetaUtil::printInstDependencyGraph(MetaBB* bb) {
        std::vector<MetaInst*> instList = bb->getInstList();
        std::unordered_map<MetaOperand*, int> deg;
        std::unordered_map<MetaOperand*, int> op_num;
        for (auto iter = instList.begin(); iter != instList.end(); ++iter) {
            deg[*iter] = 0; op_num[*iter] = (*iter)->getOperandNum();
        }
        for (auto iter = instList.begin(); iter != instList.end(); ++iter) {
            std::vector<MetaOperand*> operandList = (*iter)->getOperandList();
            for (auto op_iter = operandList.begin(); op_iter != operandList.end(); ++op_iter) {
                deg[*op_iter]++;
            }
        }
        for (auto iter = instList.begin(); iter != instList.end(); ++iter) {
            outs() << "degree of " << MetaUtil::typeVecToString((*iter)->getInstType()) \
            << " is " << deg[*iter] << "; oprand number is " << op_num[*iter] << '\n';
        } 

    }

    void MetaUtil::printInstOperand(Instruction* inst) {

    }

    std::string MetaUtil::toString(DataType type) {
        switch (type) {
            case DataType::INT8: return "INT8";
            case DataType::INT16: return "INT16";
            case DataType::INT32: return "INT32";
            case DataType::INT64: return "INT64";
            case DataType::UINT: return "UINT";
            case DataType::FLOAT: return "FLOAT";
            case DataType::DOUBLE: return "DOUBLE";
        }
    }

    std::string MetaUtil::toString(InstType type) {
        switch (type) {
            case InstType::LOAD: return "load";
            case InstType::STORE: return "store";
            case InstType::COMPARE: return "compare";
            case InstType::CALL: return "call";
            case InstType::BRANCH: return "branch";
            case InstType::JUMP: return "jump";
            case InstType::PHI: return "phi";
            case InstType::ADD: return "add";
            case InstType::SUB: return "sub";
            case InstType::MUL: return "mul";
            case InstType::DIV: return "div";
            case InstType::REMAINDER: return "remainder";
            case InstType::AND: return "and";
            case InstType::OR: return "or";
            case InstType::XOR: return "xor";
            case InstType::SHIFT: return "shift";
            case InstType::NEG: return "neg";
            case InstType::RET: return "ret";
            case InstType::ALLOCATION: return "allocation";
            case InstType::ADDRESSING: return "addressing";
            case InstType::EXCEPTION: return "exception";
            case InstType::SWAP: return "swap";
            case InstType::MIN: return "min";
            case InstType::MAX: return "max";
            case InstType::SQRT: return "sqrt";
            case InstType::FENCE: return "fence";
            case InstType::CONVERT: return "convert";
            case InstType::HINT: return "hint";
        }
    }

    std::vector<InstType> MetaUtil::getInstType(Instruction& inst) {
        std::vector<InstType> ty;
        switch (inst.getOpcode()) {
            // Terminators
            case Instruction::Ret:   
            case Instruction::Br:
            case Instruction::Switch:
            case Instruction::IndirectBr:
            case Instruction::Invoke:
            case Instruction::Resume:
            case Instruction::Unreachable:
            case Instruction::CleanupRet:
            case Instruction::CatchRet:
            case Instruction::CatchPad:
            case Instruction::CatchSwitch:
            case Instruction::CallBr:
                ty.push_back(InstType::JUMP);
                break;

            // Standard unary operators...
            case Instruction::FNeg:
                ty.push_back(InstType::NEG);
                break;

            // Standard binary operators...
            case Instruction::Add:
            case Instruction::FAdd:
                ty.push_back(InstType::ADD);
                break;
            case Instruction::Sub:
            case Instruction::FSub:
                ty.push_back(InstType::SUB);
                break;
            case Instruction::Mul:
            case Instruction::FMul:
                ty.push_back(InstType::MUL);
                break;
            case Instruction::UDiv:
            case Instruction::SDiv:
            case Instruction::FDiv:
                ty.push_back(InstType::DIV);
                break;
            case Instruction::URem:
            case Instruction::SRem:
            case Instruction::FRem:
                ty.push_back(InstType::REMAINDER);
                break;

            // Logical operators...
            case Instruction::And:
                ty.push_back(InstType::AND);
                break;
            case Instruction::Or :
                ty.push_back(InstType::OR);
                break;
            case Instruction::Xor:
                ty.push_back(InstType::XOR);
                break;

            // Memory instructions...
            case Instruction::Alloca:
                ty.push_back(InstType::ALLOCATION);
                break;
            case Instruction::Load:
                ty.push_back(InstType::LOAD);
                break;
            case Instruction::Store:
                ty.push_back(InstType::STORE);
                break;
            case Instruction::AtomicCmpXchg:
            case Instruction::AtomicRMW:
            case Instruction::Fence:
            case Instruction::GetElementPtr:
                break;

            // Convert instructions...
            case Instruction::Trunc:
            case Instruction::ZExt:
            case Instruction::SExt:
            case Instruction::FPTrunc:
            case Instruction::FPExt:
            case Instruction::FPToUI:
            case Instruction::FPToSI:
            case Instruction::UIToFP:
            case Instruction::SIToFP:
            case Instruction::IntToPtr:
            case Instruction::PtrToInt:
            case Instruction::BitCast:
            case Instruction::AddrSpaceCast:
                break;
            
            // Other instructions...
            case Instruction::ICmp:
            case Instruction::FCmp:
                ty.push_back(InstType::COMPARE);
                break;
            case Instruction::PHI:
                ty.push_back(InstType::PHI);
                break;
            case Instruction::Select:
                break;
            case Instruction::Call:
                ty.push_back(InstType::JUMP);
                break;
            case Instruction::Shl:
            case Instruction::LShr:
            case Instruction::AShr:
                ty.push_back(InstType::SHIFT);
                break;
            case Instruction::VAArg:
            case Instruction::ExtractElement:
            case Instruction::InsertElement:
            case Instruction::ShuffleVector:
            case Instruction::ExtractValue:
            case Instruction::InsertValue:
            case Instruction::LandingPad:
            case Instruction::CleanupPad:
            case Instruction::Freeze:
                break;

            default:
                break;
        }

        return ty;

   }

}