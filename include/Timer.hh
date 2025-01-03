#ifndef TIMER_HH
#define TIMER_HH

#include <chrono>

typedef std::chrono::steady_clock Clock;
typedef std::chrono::duration<float, std::milli> Duration;
typedef Clock::time_point Timepoint;

class Timer {
public:
  Timer() = default;
  void start();
  void pause();
  void resume();
  void reset();
  float display();
private:
  Timepoint startPoint;
  double accumulated = 0;
  bool isRunning = false;
};

#endif
