#ifndef _TIMER_H_
#define _TIMER_H

#include <chrono>
using namespace std::chrono;

/**
 * @brief Very simple class for recording execution time.
 *
 * A call to `start()` will start the timer. A call to `stop()` will return
 * the time (in seconds) since the last call to `start()`.
 *
 */
class Timer
{
private:
  duration<double> start_time;

public:
  Timer()
  {
  }

  /** Starts the timer. */
  void start()
  {
    start_time = steady_clock::now().time_since_epoch();
  }

  /** Returns the time since the last call to `start()` in seconds. */
  double stop()
  {
    duration<double> time = steady_clock::now().time_since_epoch();
    duration<double> time_difference = time - start_time;
    return time_difference.count();
  }
};

#endif
