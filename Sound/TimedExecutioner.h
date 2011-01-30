// OpenAL sound manager implementation.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _OPENENGINE_TIMED_EXECUTIONER_H_
#define _OPENENGINE_TIMED_EXECUTIONER_H_

#include <Core/EngineEvents.h>
#include <Core/IListener.h>
#include <Logging/Logger.h>

#include <Sound/RWValue.h>
#include <Utils/Timer.h>
#include <Utils/DateTime.h>

#include <list>

using namespace OpenEngine::Utils;

namespace OpenEngine {
namespace Sound {
    
template <class T>
class InternalStruct {
 public:
    RWValue<T>* rwvalue;
    T from;
    T to;
    Utils::Time duration;
    Utils::Time time;
 InternalStruct(RWValue<T>* rwvalue, T from, T to, Time duration, Time time)
     : rwvalue(rwvalue), from(from), to(to), duration(duration), time(time) { }
};

template <class T>
    class TimedExecutioner : public OpenEngine::Core::IListener<OpenEngine::Core::ProcessEventArg> {
 protected:
    Utils::Timer timer;
    std::list<InternalStruct<T>*> internals;
 public:
    TimedExecutioner() { 
        timer.Start();
    }

    void Add(RWValue<T>* rwvalue, T from, T to, Utils::Time duration) {
        Utils::Time time = timer.GetElapsedTime();
        internals.push_back( new InternalStruct<T>(rwvalue, from, to, duration, time) );
    }

    void Handle(OpenEngine::Core::ProcessEventArg arg) {
        Utils::Time currentTime = timer.GetElapsedTime();
        std::list<InternalStruct<T>*> deletionList;

        for (typename std::list<InternalStruct<T> * >::iterator iter = internals.begin(); 
             iter != internals.end(); iter++) {
            InternalStruct<T>* currentElement = *iter;

            Utils::Time stopTime = currentElement->time + currentElement->duration;
            if (stopTime > currentTime) {
                deletionList.push_back(currentElement);
            } else {
                if ( currentTime < currentElement->time ) {
                    throw Core::Exception("current time is in the past!");
                } else {
                    float interpolator = (float)(currentTime - currentElement->time).AsInt64() / (float)currentElement->duration.AsInt64();
                    logger.info << "Setting value to: " << interpolator << logger.end;
                    RWValue<T>* rwvalue = currentElement->rwvalue;
                    T from = currentElement->from;
                    T to = currentElement->to;
                    rwvalue->Set((to-from)*interpolator+from);
                }
            }
        }

        for (typename std::list<InternalStruct<T>*>::iterator iter = deletionList.begin(); 
             iter != deletionList.end(); iter++) {
            InternalStruct<T>* currentElement = *iter;
            internals.remove(currentElement);
        }
    }
};

} // NS Sound
} // NS OpenEngine

#endif
