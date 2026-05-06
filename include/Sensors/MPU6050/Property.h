/**
 * @file Property.h
 * @brief C++ Property template for getter/setter access patterns
 * @author Joeman
 * @date 2026-05-06
 * @version 1.0
 * @details Template class implementing property pattern for C++.
 *          Thank you Joeman for this code - http://forums.codeguru.com/showthread.php?459696-GET-SET-in-C
 */

#ifndef _Property_H_
#define _Property_H_

// Thankyou Joeman for this code - http://forums.codeguru.com/showthread.php?459696-GET-SET-in-C

template<typename Type, typename ClassHolder>

class Property {
private:
    Type (ClassHolder::*Get)();
    void (ClassHolder::*Set)(Type Val);
    ClassHolder * Class;

public:
    Property() : Get(NULL), Set(NULL), Class(NULL) {}
    Property(ClassHolder * ClassA, Type (ClassHolder::*GetA)(), void (ClassHolder::*SetA)(Type Val)) : Class(ClassA), Get(GetA), Set(SetA) {}

    void Set_Property_Control(ClassHolder * ClassA, Type (ClassHolder::*GetA)(), void (ClassHolder::*SetA)(Type Val)) {
        Class = ClassA;
        Get = GetA;
        Set = SetA;
    }

    void Set_Get(Type (ClassHolder::*GetA)()) {
        Get = GetA;
    }

    void Set_Set(void (ClassHolder::*SetA)(Type Val)) {
        Set = SetA;
    }

    void Set_Class(ClassHolder * Holder) {
        Class = Holder;
    }

    // set
    Type operator = (const Type& In) {
        if ( Set ) (Class->*Set)(In);
        return In;
    }

    // set
    Type operator = (Property& In) {
        if ( Set ) (Class->*Set)(In);
        return In;
    }

    // get
    operator Type() {
        return Get ? (Class->*Get)() : 0;
    }
};

#endif /* _Property_H_ */