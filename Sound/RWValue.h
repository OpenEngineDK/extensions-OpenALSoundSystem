// OpenAL sound manager implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENENGINE_SOUND_RW_VALUE_CALL_H_
#define _OPENENGINE_SOUND_RW_VALUE_CALL_H_

namespace OpenEngine {
namespace Sound {

template <class T>
class RWValue {
public:
    RWValue() {}
    virtual T Get() = 0;
    virtual void Set(T) = 0;
};    

template <class C, class T> 
class RWValueCall : public RWValue<T> {
public:

    C& instance;
    T (C::*getFunc)();
    void (C::*setFunc)(T);

    RWValueCall(C& instance, 
                T (C::*gFun)(),
                void (C::*sFun)(T))
        : instance(instance), 
          getFunc(gFun),
          setFunc(sFun)
    {}
    T Get() { return (instance.*getFunc)();  }
        
    void Set(T v) {  (instance.*setFunc)(v); }
};

} // NS Sound
} // NS OpenEngine

#endif
