#ifndef _TIMER_H_
#define _TIMER_H_

#include "main.h"

class Timer {

    public:

        static Timer& Instance() {
            static Timer sInstance;
            return sInstance;
        }

    private:

        Uint64 mStartTicks;

        Uint64 mElapsedTicks;

        double mDeltaTime;

        double mTimeScale;

    public: 

        void Reset();

        void TimeScale(double t = 1.0f);

        double DeltaTime();

        double TimeScale();

        void Update();

    private:

        Timer();
        ~Timer();

};

#endif