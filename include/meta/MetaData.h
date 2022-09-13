#include <string>
#include <cstdint>

namespace MetaTrans {

    class MetaInst;
    
    class MetaFunction;

    enum DataType {
        VOID,
        INT,
        FLOAT,
    };

    union DataUnion {
        
        int8_t int_8_val;

        int16_t int_16_val;

        int32_t int_32_val;

        int64_t int_64_val;

        float float_val;

        double double_val;
        
    };

    class MetaData {

    };

    // record offset in stack.
    class MetaOffset : public MetaData {
        private:
        protected:

            int offset;

        public:

            MetaOffset () : offset(0) { }

            int getOffset();

            void setOffset(int offs);

    };

    class FuncMetaData : MetaData {
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

    class InstMetaData : MetaData {
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

    class ConstMetaData : MetaData {
        private:
        protected:

        public:

            ConstMetaData();

    };


    
}