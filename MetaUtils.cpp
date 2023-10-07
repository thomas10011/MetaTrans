#include "meta/MetaUtils.h"
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <regex>


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
        { "MOV",            InstType::MOV        },
        { "CSR",            InstType::CSR        },
        { "SIGN",           InstType::SIGN       },
        { "COMPLEX",        InstType::COMPLEX    },
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
            outs() << "degree of " << (*iter) << " " << MetaUtil::typeVecToString((*iter)->getInstType()) \
            << " is " << deg[*iter] << "; oprand number is " << op_num[*iter] << '\n';
        } 

    }

    void MetaUtil::printInstOperand(Instruction* inst) {

    }

    bool MetaUtil::contain(std::string s, std::string& text) {
        int n = s.length(), m = text.length();
        int next[n];
        for (int i = 1, j = next[0] = -1; i < n; ) {
            if (j < 0 || s[i - 1] == s[j]) next[i++] = ++j;
            else j = next[j];
        }
        for (int i = 0, j = 0; i < m; ) {
            if (j == n - 1 && s[j] == text[i]) {
                return true;
            }
            else if (j < 0 || s[j] == text[i]) {
                ++i; ++j;
            }
            else j = next[j];
        }
        return false;
    }

    bool MetaUtil::isFuncPtrDeclaration(std::string& str) {
        std::regex funcPtrPattern("[a-zA-Z0-9_]*\\s*\\(\\s*\\*\\s*[a-zA-Z0-9_]*\\s*\\)\\s*\\(.*\\)");
        std::smatch matches;
        if (std::regex_match(str, matches, funcPtrPattern)) {
            return true;
        }
        return false;
    }

    std::vector<int> MetaUtil::find(std::string s, std::string& text) {
        std::vector<int> ret;
        int n = s.length(), m = text.length();
        int next[n];
        for (int i = 1, j = next[0] = -1; i < n; ) {
            if (j < 0 || s[i - 1] == s[j]) next[i++] = ++j;
            else j = next[j];
        }
        for (int i = 0, j = 0; i < m; ) {
            if (j == n - 1 && s[j] == text[i]) {
                ret.push_back(i - n + 1);
                j = next[j];
            }
            else if (j < 0 || s[j] == text[i]) {
                ++i; ++j;
            }
            else j = next[j];
        }
        return ret;
    }

    bool MetaUtil::isEmpty(std::string s) {
        if (s.length() == 0) return true;
        for (int i = 0; i < s.length(); ++i) {
            if (s[i] != ' ' || s[i] != '\n' || s[i] != '\t') return false;
        }
        return true;
    }

    void MetaUtil::strip(std::string s, std::string& text) {
        int n = s.length();
        int m = text.length();

        int i = 0;
        int j = m;

        while (i < j) {
            int k = 0;
            for (; k < n && i + k < j; ++k) {
                if (s[k] != text[i + k]) {
                    break;
                }
            }
            if (k < n) {
                break;
            }
            i += k;
        }

        while (i < j) {
            int k = 0;
            for (; k < n && i < j - k - 1; ++k) {
                if (s[n - k - 1] != text[j - k - 1]) {
                    break;
                }
            }
            if (k < n) {
                break;
            }
            j -= k;
        }

        text = text.substr(i, j - i);
    }



    std::vector<std::string> MetaUtil::split(std::string delimiter, const std::string& text) {
        int n = delimiter.length(), m = text.length();
        int next[n];
        for (int i = 1, j = next[0] = -1; i < n; ) {
            if (j < 0 || delimiter[i - 1] == delimiter[j]) next[i++] = ++j;
            else j = next[j];
        }
        std::vector<std::string> ret; 
        int start = 0;
        for (int i = 0, j = 0; i < m; ) {
            if (j == n - 1 && delimiter[j] == text[i]) {
                ret.push_back(text.substr(start, i - j - start));
                start = i + 1; j = next[j]; 
            }
            else if (j < 0 || delimiter[j] == text[i]) {
                ++i; ++j;
            }
            else j = next[j];
        }
        ret.push_back(text.substr(start, m));
        return ret;
    }


    std::string MetaUtil::join(std::string sep, const std::vector<std::string>& list) {
        if (list.size() == 0) return "";
        if (list.size() == 1) return list[0];
        std::string ret = list[0];
        for (int i = 1; i < list.size(); ++i) {
            ret += sep + list[i];
        }
        return ret;
    }

    std::string MetaUtil::trim(const std::string& str) {
        // remove the space in the head and tail
        if (str.length() == 0) {
            return str;
        }
        else if(str.length() == 1 && str[0] == ' ') {
            return "";
        }
        std::string ans = str;
        std::string blanks("\f\v\r\t\n ");
        ans.erase(0, str.find_first_not_of(blanks));
        ans.erase(str.find_last_not_of(blanks) + 1);
        return ans;
    }



    bool MetaUtil::startwith(std::string s, const std::string& text) {
        if (s.length() > text.length()) return false;
        for (int i = 0; i < s.length(); ++i) if (s[i] != text[i]) return false;
        return true;
    }

    bool MetaUtil::isNumber(std::string s) {
        std::regex reg("[-+]?([0-9]*\.[0-9]+|[0-9]+)");
        return std::regex_match(s, reg);
    }

    bool MetaUtil::same(std::string x, std::string y) {
        // Convert both strings to lowercase for comparison
        std::string lowerX = x;
        std::string lowerY = y;

        std::transform(lowerX.begin(), lowerX.end(), lowerX.begin(), ::tolower);
        std::transform(lowerY.begin(), lowerY.end(), lowerY.begin(), ::tolower);

        // Perform the comparison
        return lowerX == lowerY;
    }

    std::string MetaUtil::int2hex(int64_t num) {
        std::stringstream stream;
        stream << std::hex << num;
        return stream.str();
    }

    int64_t MetaUtil::hex2int(std::string str) {
        int64_t result;
        std::stringstream stream;
        stream << std::hex << str; // Convert hexadecimal string to integer using stringstream
        stream >> result; // Extract the integer from the stringstream
        return result;
    }

    std::string MetaUtil::toString(DataType type) {
        switch (type) {
            case DataType::INT:    return "int"   ;
            case DataType::FLOAT:  return "float"  ;
            case DataType::VOID:   return "void"  ;
            case DataType::BOOL:   return "bool"  ;
        }
    }

    DataType MetaUtil::stringToDataType(std::string str) {
        if      (str == "int" )  return DataType::INT   ;
        else if (str == "float")  return DataType::FLOAT ;
        else if (str == "void")  return DataType::VOID  ;
        else if (str == "bool")  return DataType::VOID  ;
    }

    std::string MetaUtil::upper(std::string s) {
        s.erase(remove_if(s.begin(), s.end(), ::isspace), s.end());
        transform(s.begin(), s.end(), s.begin(), ::toupper);
        return s;
    }

    std::string MetaUtil::lower( std::string& str) {
        std::string result(str.size(), ' ');
        std::transform(str.begin(), str.end(), result.begin(),
                 [](unsigned char c) { return std::tolower(c); });
        return result;
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
            case InstType::MOV:        return "mov"       ;
            case InstType::CSR:        return "csr"       ;
            case InstType::SIGN:       return "sign"      ;
            case InstType::COMPLEX:    return "complex"   ;
        }
    }

    std::string MetaUtil::toString(std::vector<InstType> type) {
        if (type.size() == 0) return "[]";

        std::string str = "[";
        for (InstType t : type) { str = str + "\"" + MetaUtil::toString(t) + "\"" + ","; }
        str[str.length() - 1] = ']';

        return str;
    }

    void MetaUtil::writeMapping(std::string data, std::string file) {
        std::ofstream out;
        out.open(file,std::ios::app);
        out << data << std::endl;
        out.close();
    }

    void MetaUtil::writeToFile(std::string data, std::string file) {
        std::ofstream out;
        out.open(file);
        out << data;
        out.close();
    }

    std::string MetaUtil::readFromFile(std::string file) {
        std::ifstream in;
        std::string data;
        in.open(file);
        if (!in.good()) return "";
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(in, line)) {
            lines.push_back(line);
        }
        in.close();
        return MetaUtil::join("\n", lines);
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

    DataType MetaUtil::extractDataType(std::string& src) {
        MetaUtil::strip(" ", src);
        if (MetaUtil::startwith("char", src)) return DataType::INT;
        if (MetaUtil::startwith("int", src)) return DataType::INT;
        if (src[src.size() - 1] == '*') return DataType::INT;
        if (MetaUtil::startwith("float", src)) return DataType::FLOAT;
        if (MetaUtil::startwith("double", src)) return DataType::FLOAT;
        if (MetaUtil::startwith("bool", src)) return DataType::BOOL;
        // TODO
        printf("WARN: Didn't find type of %s\n", src.c_str());
        return DataType::VOID;
    }


    int MetaUtil::extractDataWidth(std::string& src) {
        MetaUtil::strip(" ", src);
        if (MetaUtil::startwith("char", src)) return 8;
        if (MetaUtil::startwith("int", src)) return 32;
        if (MetaUtil::startwith("long", src)) return 32;
        if (MetaUtil::startwith("long long", src)) return 64;
        if (MetaUtil::startwith("float", src)) return 32;
        if (MetaUtil::startwith("double", src)) return 64;
        if (MetaUtil::startwith("bool", src)) return 8;
        if (src[src.size() - 1] == '*') return 32;
        printf("WARN: Didn't find length of %s\n", src.c_str());
        return 0;
    }

    int MetaUtil::extractPtrLevel(std::string& src) {
        int count = 0;
        for (int i = 0; i < src.length(); ++i) if (src[i] == '*') ++count;
        return count;
    }

    // if type is COMPUTING, each time check whether inst is addrGen; if is, type change to ADDRESSING next time;
    // if type is ADDRESSING, color inst as ADDRESSING until 'lui'
    // return last color used
    int MetaUtil::paintInsColorRecursive(MetaInst* inst, int color, int type, int depth, Path* p, std::unordered_set<MetaInst*> &visited) {
        if (visited.count(inst) && type != COLORTYPE::ADDRESSINGCOLOR && !inst->ifAddrGen()) return color;
        if (inst->isMetaPhi()) return color;
        if (inst->isType(InstType::JUMP)) return color;

        if (type == COLORTYPE::ADDRESSINGCOLOR) {
            printf("INFO: Coloring inst(%s, %d) for addressing. \n", inst->getOriginInst().c_str(), inst->getID());
            type = COLORTYPE::ADDRESSINGCOLOR;
        }
        if(type != COLORTYPE::ADDRESSINGCOLOR && inst->ifAddrGen()) {
            color++;
            type = COLORTYPE::ADDRESSINGCOLOR;
        }
        // if(type == COLORTYPE::ADDRESSINGCOLOR && !inst->ifAddrGen()) {
        //     color++;
        //     type = COLORTYPE::COMPUTING;
        // }

        inst->setColor(color, type);

        for(int i = -1; i < depth; i++) {std::cout << "  ";}
        printf("0x%x(%d, %d) %s ", inst->getAddress(), color, inst->getColor()->type, inst->getOriginInst().c_str());
        std::vector<InstType> types = inst->getInstType();
        // 处理Path相关
        for (int i = 0; i < types.size(); i++) {
            std::cout << InstTypeName[types[i]] << ", ";
            if (inst->isType(InstType::LOAD)) {
                p->numLoad++;
            }
            else if (inst->isType(InstType::STORE)) {
                p->numStore++;
            }
            else if (inst->isType(InstType::PHI)) {
                p->numPHI++;
            }
            // else if(inst->isType(InstType::GEP)) { // GEP not implemented yet

            // }
        }

        printf(" -> \n");

        if (inst->getOriginInst() == "lui" || inst->getOriginInst() == "adrp" || inst->isMemOp()) return color;

        // Paint until `lui` or no upstream instruction
        visited.insert(inst);
        std::vector<MetaOperand*> ops = inst->getOperandList();
        for(int i = 0; i < ops.size(); i++) {
            if (!ops[i] || !(ops[i]->isMetaInst())) continue;

            int newColor = paintInsColorRecursive((MetaInst*)(ops[i]), color, type, depth + 1, p, visited);
            if(newColor != color) {
                // printf("oldColor %d  newColor  %d \n", oldColor, newColor);
                color = newColor + 1;
            }

        }

        return color;
    }

    int MetaUtil::paintColor(MetaFunction* mF, int startColor) {
        std::tuple<int, int, int> counts(0,0,0); // Store, Load, Branch
        std::vector<int> colorTypes = {COLORTYPE::ADDRESSINGCOLOR, COLORTYPE::COMPUTING, COLORTYPE::CONTROLFLOW};
        std::cout << "\n\n<<== Coloring TIR for CFG: " << mF->getFunctionName() << " ==>>" << "\n";
        for (auto func_iter = mF->bb_begin(); func_iter != mF->bb_end(); ++func_iter) {
            MetaBB* bb = *func_iter;
            std::cout << "-- Coloring Meta BB: " << " --" << "\n";
            for (auto bb_iter = bb->inst_begin(); bb_iter != bb->inst_end(); ++bb_iter) { 
                MetaInst* inst = *bb_iter;

                printf("INFO: coloring for inst addr = %x, ", inst->getAddress());
                printf("type = %d\n", inst->getInstType().at(0));
                

                if (inst->isType(InstType::STORE)) {
                    std::get<0>(counts) = std::get<0>(counts) + 1;
                    std::vector<MetaOperand*> ops = inst->getOperandList();
                    std::cout << "Store Coloring... "  <<  std::endl;
                    inst->setColor(startColor, COLORTYPE::MEMOP);
                    for (int i = 0; i < ops.size(); i++) {
                        if(!ops[i] || !(ops[i]->isMetaInst())) continue;
                        MetaInst* mi = (MetaInst*)(ops[i]);
                        Path* p = new Path{(MetaInst*)(mi), i, 0, 0, 0, 0};
                        inst->addToPath(p);
                        printf("%x(%d) STORE,  -> \n", inst->getAddress(), startColor);
                        std::unordered_set<MetaInst*> set;
                        COLORTYPE colorType = i == 0 ? (inst->ifAddrGen() ? COLORTYPE:: ADDRESSINGCOLOR : COLORTYPE::COMPUTING) : COLORTYPE::COMPUTING;
                        startColor = paintInsColorRecursive((MetaInst*)(mi), startColor, colorType, 0, p, set);
                        printf("%x, type: %d, numLoad: %d, numStore: "
                                "%d, numPHI: %d, numGEP: %d\n",
                                p->firstNode, p->type, p->numLoad,
                                p->numStore, p->numPHI, p->numGEP);
                        startColor++;
                    }
                    std::vector<MetaInst*> vecForHash;
                    vecForHash.push_back(inst);
                    if(ops.size() > 0 && ops[0]->isMetaInst()) vecForHash.push_back((MetaInst*)(ops[0])); // only push_back data compute
                    auto hashCode = MetaUtil::hashCode(vecForHash);
                    printf("hashCode: %d\n", hashCode);
                    inst->setHashcode(hashCode);
                }
                else if (inst->isType(InstType::LOAD)) {
                    std::get<1>(counts) = std::get<1>(counts) + 1;
                    std::vector<MetaInst*> ops = inst->getOperandOnlyInstList();
                    printf("INFO: Painting color for load addr = %x\n", inst->getAddress());
                    std::cout << "IsLoad " << ops.size() <<  std::endl;
                    inst->setColor(startColor, COLORTYPE::MEMOP);
                    for(int i = 0; i < ops.size(); i++) {
                        Path* p = new Path{(MetaInst*)(ops[i]), 1, 0, 0, 0, 0};
                        inst->addToPath(p);
                        printf("%x(%d) LOAD,  -> \n", inst->getAddress(), startColor);
                        std::unordered_set<MetaInst*> set;
                        COLORTYPE colorType = inst->ifAddrGen() ? COLORTYPE:: ADDRESSINGCOLOR : COLORTYPE::COMPUTING;
                        startColor = paintInsColorRecursive((MetaInst*)(ops[i]), startColor, colorType, 0, p, set);
                        printf("%x, type: %d, numLoad: %d, numStore: "
                                "%d, numPHI: %d, numGEP: %d\n",
                                p->firstNode, p->type, p->numLoad,
                                p->numStore, p->numPHI, p->numGEP);
                        startColor++;
                    }
                    std::vector<MetaInst*> vecForHash;
                    vecForHash.push_back(inst);
                    std::vector<MetaInst*> users = inst->getUsers();
                    for(int i = 0; i < users.size(); i++) {
                        vecForHash.push_back((MetaInst*)(users[i]));
                    }
                    auto hashCode = MetaUtil::hashCode(vecForHash);
                    printf("hashCode: %d\n", hashCode);
                    inst->setHashcode(hashCode);
                }
                else if(inst->isType(InstType::BRANCH) || inst->isMetaPhi()) {
                    std::get<2>(counts) = std::get<2>(counts) + 1;
                    std::cout << "IsBranch" << std::endl;
                    std::vector<MetaInst*> ops = inst->getOperandOnlyInstList();
                    inst->setColor(startColor, COLORTYPE::CONTROLFLOW);
                    for(int i = 0; i < ops.size(); i++) {
                        Path* p = new Path{(MetaInst*)(ops[i]), 2, 0, 0, 0, 0};
                        inst->addToPath(p);
                        printf("Color: %d, Type: Control Flow\n", startColor);
                        printf("%x(%d) BRANCH,  -> \n", inst->getAddress(), startColor);
                        std::unordered_set<MetaInst*> set;
                        startColor = paintInsColorRecursive((MetaInst*)(ops[i]), startColor, COLORTYPE::COMPUTING, 0, p, set);
                        printf("%x, type: %d, numLoad: %d, numStore: "
                                "%d, numPHI: %d, numGEP: %d\n",
                                p->firstNode, p->type, p->numLoad,
                                p->numStore, p->numPHI, p->numGEP);
                        startColor++;
                    }
                }else if(inst->isType(InstType::CALL) || inst->isMetaCall()) {
                    std::cout << "IsCall" << std::endl;
                    std::vector<MetaInst*> ops = inst->getOperandOnlyInstList();
                    // call应该当成computing翻译而不是control flow
                    inst->setColor(startColor, COLORTYPE::COMPUTING);
                    for(int i = 0; i < ops.size(); i++) {
                        Path* p = new Path{(MetaInst*)(ops[i]), 2, 0, 0, 0, 0};
                        printf("Color: %d, Type: Call\n", startColor);
                        printf("%x(%d) CALL,  -> \n", inst->getAddress(), startColor);
                        std::unordered_set<MetaInst*> set;
                        startColor = paintInsColorRecursive((MetaInst*)(ops[i]), startColor, COLORTYPE::COMPUTING, 0, p, set);
                        startColor++;
                    }
                }else if(inst->isType(InstType::JUMP)) {
                    std::cout << "IsJump" << std::endl;
                    inst->setColor(startColor, COLORTYPE::CONTROLFLOW);
                }
            };
            std::cout << "\n\n-- Coloring Meta BB End! S/L/B= " << std::get<0>(counts) << ", " << std::get<1>(counts) << ", " << std::get<2>(counts) << " --" << "\n";
            counts = std::make_tuple(0, 0, 0);
        };
        std::cout << "\n\n<<== Coloring TIR for CFG End! " << " ==>>" << "\n";
        return startColor;
    }

    int MetaUtil::paintColorCheck(MetaFunction* mF) {
        int count = 0, addrcount = 0, computecount = 0;
        std::cout << "\n\n<<== paintColorCheck for CFG: " << mF->getFunctionName() << " ==>>" << "\n";
        for (auto func_iter = mF->bb_begin(); func_iter != mF->bb_end(); ++func_iter) {
            MetaBB* bb = *func_iter;
            for (auto bb_iter = bb->inst_begin(); bb_iter != bb->inst_end(); ++bb_iter) {
                MetaInst *inst = *bb_iter;
                if(inst->getColor()->type == -1) {
                    count++;
                    if(inst->ifAddrGen()) {addrcount++; inst->setColor(-1, COLORTYPE::ADDRESSINGCOLOR);}
                    else if(inst->isLoad()) {inst->setColor(-1, COLORTYPE::MEMOP);}
                    else if(inst->isStore()) {inst->setColor(-1, COLORTYPE::MEMOP);}
                    else if(inst->isType(InstType::BRANCH) || inst->isType(InstType::JUMP) || inst->isType(InstType::RET)) {inst->setColor(-1, COLORTYPE::CONTROLFLOW);}
                    else {computecount++; inst->setColor(-1, COLORTYPE::COMPUTING);}
                    printf(
                        "WARN: Found uncolored inst(%s, %d, %s), isAddrGen = %d.\n", 
                        inst->getOriginInst().c_str(), 
                        inst->getID(), 
                        MetaUtil::typeVecToString(inst->getInstType()).c_str(),
                        inst->ifAddrGen()
                    );
                }
            }
        }
        std::cout << std::dec << "Un-colored number : " << count << ", addr number : " << addrcount << ", compute number : " << computecount  << std::endl;
    }

    unsigned long MetaUtil::hashCode(std::vector<MetaInst*> instList) {
        // instLis[0] is load/store, the remain elements are instructions depend on load, or store data
        // Type of each inst will generate a hash (may including 1~n types)
        // Then hash all the Each inst's hash as hash of instList
        std::vector<std::string> stringList;
        unsigned long ans = 5381;
        for(MetaInst* inst : instList) {
            unsigned long hash = 5381;
            auto types = inst->getInstType();
            std::string typeName = "";
            for(int j = 0; j < types.size(); j++) {
                typeName = typeName + InstTypeName.at(types.at(j));
            }
            stringList.push_back(typeName);
        }
        // Hash stringList
        std::vector<unsigned long> hashes;
        for(std::string str : stringList) {
            unsigned long hash = 5381;
            for(auto c : str) {
                hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
            }
            hashes.push_back(hash);
        }
        // Hash hashes in an order-independent way
        for(auto ha : hashes) {
            ans += ha;
        }
        return ans;
    }

    /// @brief Get the available memory in MB
    /// @return 
    unsigned long long MetaUtil::getAvailableMemory() {
        long long pages = sysconf(_SC_AVPHYS_PAGES);
        long long page_size = sysconf(_SC_PAGE_SIZE); // 以MB为单位
        return (pages * page_size);
    }

    void MetaUtil::setDataRoot(MetaFunction* mF) {
        for (auto func_iter = mF->bb_begin(); func_iter != mF->bb_end(); ++func_iter) {
            MetaBB* bb = *func_iter;
            std::cout << "-- setDataRoot for MetaFunction: " << " --" << "\n";
            for (auto bb_iter = bb->inst_begin(); bb_iter != bb->inst_end(); ++bb_iter) { 
                MetaInst* inst = *bb_iter;
                std::string ans = MetaUtil::findDataRoot(inst);
                if(ans != "") {
                    std::cout << "\t -- setDataRoot for inst: " << inst << ", ans -- " << ans << "\n";
                    inst->setDataRoot("TIR_GLOBAL");
                    inst->setGlobalSymbolName(ans);
                }
            }
        }
    }

    std::string MetaUtil::findDataRoot(MetaInst* inst) {
        std::unordered_set<MetaOperand*> s;
        std::string ans = MetaUtil::findDataRootRecursive(inst, s);
        return ans;
    }

    std::string MetaUtil::findDataRootRecursive(MetaOperand* inst, std::unordered_set<MetaOperand*> set) {
        if(!inst || set.count(inst)) return "";
        set.insert(inst);
        if(inst->isMetaConstant()) {
            if(((MetaConstant*)inst)->isGlobal()) {
                return ((MetaConstant*)inst)->getName();
            }
        }
        std::string ans = "";
        if(inst->isMetaInst()) {
            auto list = ((MetaInst*)inst)->getOperandList();
            for(auto it = list.begin(); it != list.end(); it++) {
                ans = findDataRootRecursive(*it, set);
                if(ans != "") return ans;
            }
        }
        set.erase(inst);
        return ans;
    }

    // A function that returns true if a character is a format flag
    bool is_flag(char c) {
    return c == '+' || c == '-' || c == '0' || c == '#' || c == ' ';
    }

    // A function that returns true if a character is a length modifier
    bool is_length(char c) {
    return c == 'h' || c == 'l' || c == 'j' || c == 'z' || c == 't' || c == 'L';
    }

    // A function that counts the number of arguments based on the format string printf/scanf
    int MetaUtil::count_args(const char* fmt) {
        int count = 0; // The number of arguments
        bool percent = false; // A flag to indicate if we are parsing a format specifier
        bool width = false; // A flag to indicate if we are parsing a field width
        bool precision = false; // A flag to indicate if we are parsing a precision
        char c; // The current character

        while ((c = *fmt++) != '\0') { // Loop until the end of the string
            if (percent) { // If we are parsing a format specifier
            if (c == '%') { // If we see an escaped percent sign
                percent = false; // Reset the flag
            } else if (is_flag(c)) { // If we see a format flag
                // Do nothing, just skip it
            } else if (c == '*') { // If we see a variable field width or precision
                count++; // Increment the argument count
                if (width) { // If we are parsing a field width
                width = false; // Reset the flag
                } else if (precision) { // If we are parsing a precision
                precision = false; // Reset the flag
                }
            } else if (c >= '1' && c <= '9') { // If we see a digit
                if (!width && !precision) { // If we are not parsing a field width or precision
                width = true; // Set the flag for field width
                }
            } else if (c == '.') { // If we see a decimal point
                precision = true; // Set the flag for precision
            } else if (is_length(c)) { // If we see a length modifier
                // Do nothing, just skip it
            } else { // If we see anything else
                count++; // Increment the argument count
                percent = false; // Reset the flag
            }
            } else { // If we are not parsing a format specifier
            if (c == '%') { // If we see a percent sign
                percent = true; // Set the flag for format specifier
            }
            }
        }

        return count; // Return the number of arguments
    }


    bool MetaUtil::test() {
        printf("INFO: testing Graph API.\n");
        Graph<int> g;
        g.add(1, {2, 3, 4});
        g.setRoot(1);
        g.add(2, 5);
        g.add(5, 3);
        g.add(3, 5);
        g.add(5, 6);
        g.add(6, 4);
        g.add(6, 7);
        std::vector<int> seq = g.getDFSseq();
        for (int i = 0; i < seq.size(); ++i) {
            printf("%d ", seq[i]);
        }
        printf("\n");


        seq = g.getBFSseq();
        for (int i = 0; i < seq.size(); ++i) {
            printf("%d ", seq[i]);
        }
        printf("\n");
    }

    std::pair<int, std::vector<int>> MetaUtil::getReturnAndArgumentRegFromFuncPtrString(std::string funcPtrStr) {
        std::vector<int> leftBracketPos  = MetaUtil::find("(", funcPtrStr);
        std::vector<int> rightBracketPos = MetaUtil::find(")", funcPtrStr);

        assert(leftBracketPos.size() == rightBracketPos.size() && leftBracketPos.size() == 2);

        std::string retTypeStr = MetaUtil::trim(funcPtrStr.substr(0, leftBracketPos[0]));
        std::string argStr = funcPtrStr.substr(leftBracketPos[1] + 1, rightBracketPos[1] - leftBracketPos[1] - 1);

        printf("INFO: getFuncPtrType.\n");
        printf("INFO: retTypeStr = %s\n", retTypeStr.c_str());
        printf("INFO: argStr = %s\n", argStr.c_str());

        std::pair<int, std::vector<int>> ret;
        ret.first = retTypeStr == "void" ? 0 : (retTypeStr == "float" || retTypeStr == "double" ? 2 : 1);

        std::vector<std::string> argStrs = MetaUtil::split(",", argStr);
        int a0count = ARG_START_REG, fa0count = FARG_START_REG;
        std::vector<int> ans;
        for (auto str : argStrs) {
            if(str.find("*") != std::string::npos) {
                if(a0count > ARG_END_REG) ans.push_back(-1);
                else ans.push_back(a0count++);
            }
            else if(str.find("double")!= std::string::npos || str.find("float")!= std::string::npos){
                if(fa0count > FARG_END_REG) ans.push_back(-2);
                else ans.push_back(fa0count++);
            }
            else if(str == "...") {
                break;  
            }
            else{
                if(a0count > ARG_END_REG) ans.push_back(-1);
                else ans.push_back(a0count++);
            }
        }
        ret.second = ans;
        return ret;
    }
}