#include "Timer.hh"
#include "Exceptions.hh"

namespace {
  Duration delta(const Timepoint& t1, const Timepoint& t2) {
    return std::chrono::duration<float, std::milli>(t1 - t2);
  }
}

void Timer::start() {
  if (isRunning)
    throw BadTimerCallSequence();
  isRunning = true;
  this->startPoint = Clock::now();
}

void Timer::pause() {
  // Pause always updates accumulated time.
  if (not isRunning)
    throw BadTimerCallSequence();
  isRunning = false;
  auto endPoint = Clock::now();
  // printf("D:  timer paused\n");
  accumulated += delta(endPoint, startPoint).count();
  // printf("D:  accumulated time: %f ms\n", accumulated);
}

void Timer::resume() {
  if (isRunning)
    throw BadTimerCallSequence();
  isRunning = true;
  startPoint = Clock::now();
}

void Timer::reset() {
  // A reset stops the timer and clears the accumulated time.
  // Hence, to restart the timer, we also need a call to start().
  isRunning = false;
  accumulated = 0;
}

float Timer::display() {
  // Display just reads from the timer.
  // Hence, it doesn't affect the timer state.
  // If the timer is paused (i.e., not running),
  // only the accumulated time is displayed.
  if (not isRunning)
    return accumulated;
  // If the timer is running,
  // We pause the timer, read the accumulated time, and resume.
  this->pause();
  auto result = accumulated;
  this->resume();
  return result;
}
