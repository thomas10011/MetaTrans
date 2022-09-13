#include <vector>

namespace MetaTrans {
    
    class Subject {
        protected:

            std::vector<Observer*> observers;
        
        public:

            Subject& registerObserver(Observer* obs);


    };


    class Observer {
        
        public:
            
            virtual void update(Subject& sub);

    };
}