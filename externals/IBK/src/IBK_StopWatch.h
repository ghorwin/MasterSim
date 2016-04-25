#ifndef IBK_StopWatchH
#define IBK_StopWatchH

#include <string>

namespace IBK {

class StopWatchImpl;

/*! The StopWatch class can be used to measure the time used during execution of
	certain program parts.
	Every StopWatch object remembers its time and state. So you can use several
	stop watches simultaneously (for instance a global watch and a watch for
	certain subroutines).

	\code
	StopWatch w;
	someLengthyFunction();
	std::cout << w.diff_str() << " needed for execution of someLengthyFunction()" << std::endl;
	\endcode

	\note This stopwatch can be used also for OpenMP parallelized code. On Unix-Systems
	it uses gettimeofday().

	We use the P-impl pattern to hide the plattform specific includes and code from
	users.

	Interval functionality:
	\code
	StopWatch w;
	// set notification interval in seconds
	w.setIntervalLength(10);
	// start lengthy function
	while (true) {
		if (w.intervalCompleted())
			std::cout << "Still busy..." << std::endl;

		// ...
	}

	\endcode

*/
class StopWatch {
public:
	/*! Default constructor, creates a stop watch and starts it. */
	StopWatch();
	/*! Cleanup of p-impl object. */
	~StopWatch();
	/*! Restarts the clock. */
	void start();
	/*! Stops the clock and remembers the elapsed time.
		\return Returns the elapsed time from start in [ms].
	*/
	double stop();
	/*! Returns the time so far in milli seconds [ms]. */
	double difference() const;
	/*! Returns the time so far as a string with unit (either in milli seconds or seconds).
		\code
		StopWatch w;
		// ...
		std::string s = w.diff_str(); // gives a string of format "1.23 s";
		\endcode
	*/
	const std::string diff_str() const;
	/*! Returns the time so far as a string but allows the specification of the width of
		the resulting string. The time value will be align to the right.
		\param width The field with of the number part.
		\code
		StopWatch w;
		// ...
		std::string s = w.diff_str(6);
		// gives a string of format "  1.23 s";
		\endcode
	*/
	const std::string diff_str(std::size_t width) const;

	/*! Sets interval length.
		\param intervalLength Interval length in [s].
	*/
	void setIntervalLength(double intervalLength);

	/*! Returns true when interval has completed and increases the interval counter.
		Call this function repeatedly in your code to check if the selected interval has
		passed.
	*/
	bool intervalCompleted();

protected:
	/*! Timer check interval in  [s]. */
	double m_intervalLength;

	/*! Time elapsed since start of timer and until last interval in [s]. */
	double m_lastIntervalDiff;

private:
	/*! Pointer to private implementation. */
	StopWatchImpl	*m_p;
};

} // namespace IBK

/*! \file IBK_StopWatch.h
	\brief Contains the declaration of the class StopWatch.

	\example StopWatch.cpp
	This is an example of how to use the class IBK::StopWatch.
*/

#endif // IBK_StopWatchH
