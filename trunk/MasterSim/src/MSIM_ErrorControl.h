#ifndef MSIM_ERRORCONTROL_H
#define MSIM_ERRORCONTROL_H

namespace MASTER_SIM {

class MasterSim;

/*! This class handles everything related to error control. */
class ErrorControl {
public:
	ErrorControl(MasterSim * master);

	/*! This function starts with an integration step completed.
		It remembers the state at the old time point t, the newly
		computed states at time t + stepsize. Then it resets all
		slaves back to the point t and takes two steps with size
		stepsize/2. The comparison of the first (long) step result
		and the result obtained with the two steps makes up the error
		test.
		If test is successful, the result of the two smaller steps
		is stored in the m_xxxNext variables and m_stepSize is set
		to the total step size. So, the calling function can complete
		the step just as if checkLocalError() was never called.

		In case of false, the calling function should decide whether
		a reset of the step is meaningful or if the integration shall
		continue with the errorenous solution.
	*/
	bool checkLocalError();

private:
	MasterSim	*m_master;
};

} // namespace MASTER_SIM

#endif // MSIM_ERRORCONTROL_H
