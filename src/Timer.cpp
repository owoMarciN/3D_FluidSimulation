#include "Timer.h"

Timer::Timer() {
    // Resetuje licznik czasu przy tworzeniu obiektu
    Reset();      

    // Domyślna skala czasu = 1 (normalna prędkość)
    mTimeScale = 1.0f; 
}

Timer::~Timer() {
    std::cout << "Timer -- Destroyed" << std::endl;
}

// Resetuje timer – zeruje czas rozpoczęcia, upłynięte milisekundy i deltaTime
void Timer::Reset() {
    // Pobiera aktualny czas w milisekundach od startu SDL
    mStartTicks = SDL_GetTicks(); 

    // Zeruje upłynięty czas
    mElapsedTicks = 0;   

    // Zeruje deltaTime         
    mDeltaTime = 0.0f;            
}

// Ustawia skalę czasu (np. spowolnienie lub przyspieszenie)
void Timer::TimeScale(double t) {
    mTimeScale = t;
}

// Zwraca deltaTime – czas, który upłynął od ostatniej aktualizacji w sekundach
double Timer::DeltaTime() {
    return mDeltaTime;
}

// Zwraca aktualną skalę czasu
double Timer::TimeScale() {
    return mTimeScale;
}

// Aktualizuje timer – oblicza deltaTime
void Timer::Update() {
    // Oblicza ile milisekund upłynęło od ostatniego resetu
    mElapsedTicks = SDL_GetTicks() - mStartTicks;

    // Konwertuje milisekundy na sekundy
    mDeltaTime = mElapsedTicks * 0.001f;

    // Można zastosować skalę czasu, np. mDeltaTime *= mTimeScale;
}


