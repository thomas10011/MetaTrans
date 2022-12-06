#include "meta/MetaUtils.h"

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
            case DataType::INT:    return "INT"   ;
            case DataType::FLOAT:  return "FLOAT" ;
            case DataType::VOID:   return "VOID"  ;
        }
    }

    DataType MetaUtil::stringToDataType(std::string str) {
        if (str == "INT" ) return DataType::INT;
        else if (str == "FLOAT") return DataType::FLOAT;
        else if (str == "VOID") return DataType::VOID;
    }

    std::string MetaUtil::upper(std::string s) {
        s.erase(remove_if(s.begin(), s.end(), ::isspace), s.end());
        transform(s.begin(), s.end(), s.begin(), ::toupper);
        return s;
    }

    InstType MetaUtil::stringToInstType(std::string str) {
        str = upper(str);
        return YamlUtil::str_inst_type_map[str];
    }

    std::string MetaUtil::toString(InstType type) {
        switch (type) {
            case InstType::LOAD:       return "load"      ;
            case InstType::STORE:      return "store"     ;
            case InstType::COMPARE:    return "compare"   ;
            case InstType::CALL:       return "call"      ;
            case InstType::BRANCH:     return "branch"    ;
            case InstType::JUMP:       return "jump"      ;
            case InstType::PHI:        return "phi"       ;
            case InstType::ADD:        return "add"       ;
            case InstType::SUB:        return "sub"       ;
            case InstType::MUL:        return "mul"       ;
            case InstType::DIV:        return "div"       ;
            case InstType::REMAINDER:  return "remainder" ;
            case InstType::AND:        return "and"       ;
            case InstType::OR:         return "or"        ;
            case InstType::XOR:        return "xor"       ;
            case InstType::SHIFT:      return "shift"     ;
            case InstType::NEG:        return "neg"       ;
            case InstType::RET:        return "ret"       ;
            case InstType::ALLOCATION: return "allocation";
            case InstType::ADDRESSING: return "addressing";
            case InstType::EXCEPTION:  return "exception" ;
            case InstType::SWAP:       return "swap"      ;
            case InstType::MIN:        return "min"       ;
            case InstType::MAX:        return "max"       ;
            case InstType::SQRT:       return "sqrt"      ;
            case InstType::FENCE:      return "fence"     ;
            case InstType::CONVERT:    return "convert"   ;
            case InstType::HINT:       return "hint"      ;
        }
    }

    std::string MetaUtil::toString(std::vector<InstType> type) {
        if (type.size() == 0) return "[]";

        std::string str = "[";
        for (InstType t : type) { str = str + "\"" + MetaUtil::toString(t) + "\"" + ","; }
        str[str.length() - 1] = ']';

        return str;
    }

    DataType MetaUtil::extractDataType(Type& dataType) {
        switch (dataType.getTypeID())
        {
            case Type::HalfTyID:
            case Type::BFloatTyID:
            case Type::FloatTyID:
            case Type::DoubleTyID:
            case Type::X86_FP80TyID:
            case Type::FP128TyID:
            case Type::PPC_FP128TyID:
                return DataType::FLOAT;
            case Type::VoidTyID:
                return DataType::VOID;
            case Type::LabelTyID:
            case Type::MetadataTyID:
            case Type::X86_MMXTyID:
            case Type::X86_AMXTyID:
            case Type::TokenTyID:
                break;
            case Type::IntegerTyID:
                return DataType::INT;
            case Type::FunctionTyID:
            case Type::PointerTyID:
            case Type::StructTyID:
            case Type::ArrayTyID:
            case Type::FixedVectorTyID:
            case Type::ScalableVectorTyID:
                break;
            default:
                break;
        }
        // TODO
        return DataType::VOID;
    }


    int MetaUtil::extractDataWidth(Type& dataType) {
        // TODO
        switch (dataType.getTypeID())
        {
            case Type::HalfTyID:
            case Type::BFloatTyID:
                return 16;
            case Type::FloatTyID:
                return 32;
            case Type::DoubleTyID:
                return 64;
            case Type::X86_FP80TyID:
                return 80;
            case Type::FP128TyID:
                return 128;
            case Type::PPC_FP128TyID:
                return 128;
            case Type::VoidTyID:
            case Type::LabelTyID:
            case Type::MetadataTyID:
            case Type::X86_MMXTyID:
            case Type::X86_AMXTyID:
            case Type::TokenTyID:
                break;
            case Type::IntegerTyID:
                return dataType.getIntegerBitWidth();
            case Type::FunctionTyID:
            case Type::PointerTyID:
            case Type::StructTyID:
            case Type::ArrayTyID:
            case Type::FixedVectorTyID:
            case Type::ScalableVectorTyID:
                break;
            default:
                break;
        }
        return 0;
    }

    void MetaUtil::paintInsColorRecursive(MetaInst* inst, int color, int type, int depth) {
        inst->addColor(color, type);
        for(int i = -1; i < depth; i++) {std::cout << "  ";}
        printf("%x(%d) ", inst, color);
        std::vector<InstType> types = inst->getInstType();
        for(int i = 0; i < types.size(); i++) {
            std::cout << InstTypeName[types[i]] << ", ";
        }
        printf(" -> \n");
        if(!inst->isType(InstType::LOAD) && !inst->isType(InstType::PHI)) { // Paint until `load` or `phi` or no upstream instruction
            std::vector<MetaOperand*> ops = inst->getOperandList();
            for(int i = 0; i < ops.size(); i++) {
                if(ops[i]->isMetaInst()) {
                    paintInsColorRecursive((MetaInst*)(ops[i]), color, type, depth + 1);
                }
            }
        }else {
            return;
        }
    }
    
    void MetaUtil::paintColor(MetaFunction* mF, int startColor) {
        std::vector<std::string> name = {"Data Compute", "Addressing", "Control Flow"};
        std::cout << "\n\n<<== Coloring TIR for CFG: " << " ==>>" << "\n";
        for_each(mF->bb_begin(), mF->bb_end(), [&] (MetaBB* bb) {
            std::cout << "-- Coloring Meta BB: " << " --" << "\n";
            for_each(bb->inst_begin(), bb->inst_end(), [&] (MetaInst* inst) { 
                if (!inst->isMetaPhi())
                    if(inst->isType(InstType::STORE)){
                        std::vector<MetaOperand*> ops = inst->getOperandList();
                        std::cout << "IsStore " << ops.size() <<  std::endl;
                        for(int i = 0; i < ops.size(); i++) {
                            if(ops[i]->isMetaInst()) {
                                inst->addColor(startColor, i);
                                printf("Color: %d, Type: %s\n", startColor, name[i].c_str());
                                printf("%x(%d) STORE,  -> \n", inst, startColor);
                                paintInsColorRecursive((MetaInst*)(ops[i]), startColor, i, 0);
                                startColor++;
                            }
                        }
                    }else if(inst->isType(InstType::BRANCH)){
                        std::cout << "IsBranch" << std::endl;
                        std::vector<MetaOperand*> ops = inst->getOperandList();
                        for(int i = 0; i < ops.size(); i++) {
                            if(ops[i]->isMetaInst()) {
                                inst->addColor(startColor, 2);
                                printf("Color: %d, Type: Control Flow\n", startColor);
                                printf("%x(%d) BRANCH,  -> \n", inst, startColor);
                                paintInsColorRecursive((MetaInst*)(ops[i]), startColor, 2, 0);
                                startColor++;
                            }
                        }
                    }
                else {
                    // TODO
                }
            });
        });
        std::cout << "\n\n<<== Coloring TIR for CFG End! " << " ==>>" << "\n";
    }


}