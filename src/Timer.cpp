#include "Timer.h"

Timer::Timer() {
    Reset();
    mTimeScale = 1.0f;
}

Timer::~Timer() {
    std::cout << "Timer -- Destroyed" <<std::endl;
}

void Timer::Reset() {
    mStartTicks = SDL_GetTicks();
    mElapsedTicks = 0;
    mDeltaTime = 0.0f;
}

void Timer::TimeScale(double t) {
    mTimeScale = t;
}

double Timer::DeltaTime() {
    return mDeltaTime;
}

double Timer::TimeScale() {
    return mTimeScale;
}

void Timer::Update() {
    mElapsedTicks = SDL_GetTicks() - mStartTicks;

    //Converting milliseconds to seconds
	mDeltaTime = mElapsedTicks * 0.001f;
}

