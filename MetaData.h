#include <string>


namespace MetaTrans {

    class MetaInst;

    class MetaFunction;

    enum DataType {
        INT8,
        INT16,
        INT32,
        INT64,
        // necessary?
        UINT,
        FLOAT,
        DOUBLE,
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

    };


    
}