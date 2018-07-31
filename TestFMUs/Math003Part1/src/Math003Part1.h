#ifndef Math003Part1H
#define Math003Part1H

#include "fmi2common/InstanceData.h"

#include "fmi2common/fmi2FunctionTypes.h"

/*! This class wraps all data needed for a single instance of the FMU. */
class Math003Part1 : public InstanceData {
public:
	/*! Initializes empty instance. */
	Math003Part1();

	/*! Destructor, writes out cached results from Therakles. */
	~Math003Part1();

	/*! Initializes Math003Part1 */
	void init();

	/*! This function triggers a state-update of the embedded model whenever our cached input
		data differs from the input data in the model.
	*/
	void updateIfModified();

	/*! Called from fmi2DoStep(). */
	virtual void integrateTo(double tCommunicationIntervalEnd);

	// Functions for getting/setting the state

	/*! This function computes the size needed for full serizalization of
		the FMU and stores the size in m_fmuStateSize.
		\note The size includes the leading 8byte for the 64bit integer size
		of the memory array (for testing purposes).
	*/
	virtual void computeFMUStateSize();

	/*! Copies the internal state of the FMU to the memory array pointed to by FMUstate.
		Memory array always has size m_fmuStateSize.
	*/
	virtual void serializeFMUstate(void * FMUstate);

	/*! Copies the content of the memory array pointed to by FMUstate to the internal state of the FMU.
		Memory array always has size m_fmuStateSize.
	*/
	virtual void deserializeFMUstate(void * FMUstate);

}; // class Math003Part1

#endif // Math003Part1H
