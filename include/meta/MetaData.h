#pragma once
#include <string>
#include <cstdint>

namespace MetaTrans {

class MetaInst;

class MetaFunction;

enum DataType {
    VOID,
    INT,
    FLOAT,
    BOOL
};

typedef struct ColorData {
    int color;
    int type; // 0 addressing 1 data computing 2 control flow
    explicit ColorData(const int& c, const int& t) : color(c), type(t) {}
    bool operator <(const ColorData& obj) const {
        return color < obj.color;
    }
} ColorData;


union DataUnion {
    
    int8_t int_8_val;

    int16_t int_16_val;

    int32_t int_32_val;

    int64_t int_64_val;

    float float_val;

    double double_val;
    
};

class MetaPrimType {
private:

    // 类型 + 位宽即可确定数据类型
    DataType type;
    int width;

    // 指针级别：*=1级，**=二级
    int ptrLevel;

public:

    MetaPrimType();
    MetaPrimType(DataType ty, int width, int ptrLevel);
    MetaPrimType(std::string);

    MetaPrimType& setType(DataType ty);
    MetaPrimType& setWidth(int w);
    MetaPrimType& setPrtLevel(int l);

    DataType getType();
    int getWidth();
    int getPtrLevel();

    std::string toString();
};

// record offset in stack.
class MetaOffset {
private:
protected:

    int offset;

public:

    MetaOffset () : offset(0) { }

    int getOffset();

    void setOffset(int offs);

};

class FuncMetaData {
private:
protected:

    std::string funcName;

    int argAmount;

    DataType outputType;

    MetaFunction* func;
    
public:
    FuncMetaData();

    void setFunctionName(std::string name);

    void setArgAmount(int amt);

    void setOutputType(DataType type);

    void setFunction(MetaFunction* mf);

    std::string getFunctionName();

    int getArgAmount();

    DataType getOutputType();

    MetaFunction* getFunction();

};

class InstMetaData {
private:
protected:

    MetaInst* inst;

    int operandAmount;

public:
    
    InstMetaData();

    void setInst(MetaInst* inst);

    void setOperandAmount(int amt);

    MetaInst* getInst();

    int getOperandAmount();

};

class ConstMetaData {
    private:
    protected:

    public:

        ConstMetaData();

};



}