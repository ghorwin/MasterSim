/*	FMI Interface for the MasterSim Test Cases
  Written by Andreas Nicolai (2018), andreas.nicolai@gmx.net

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "fmi2common/fmi2Functions.h"
#include "fmi2common/fmi2FunctionTypes.h"
#include "LotkaVolterraPrey.h"

#include <cmath>

// FMI interface variables

#define FMI_OUTPUT_X 1
#define FMI_INPUT_Y 2


// *** Variables and functions to be implemented in user code. ***

// *** GUID that uniquely identifies this FMU code
const char * const InstanceData::GUID = "{471a3b52-4923-44d8-ab4a-fcdb813c7352}";

// *** Factory function, creates model specific instance of InstanceData-derived class
InstanceData * InstanceData::create() {
	return new LotkaVolterraPrey; // caller takes ownership
}


// create a model instance
LotkaVolterraPrey::LotkaVolterraPrey() :
	InstanceData()
{
	// initialize input variables
	m_realInput[FMI_INPUT_Y] = 0;
	// initialize output variables
	m_realOutput[FMI_OUTPUT_X] = 10;
}


LotkaVolterraPrey::~LotkaVolterraPrey() {
}


// create a model instance
void LotkaVolterraPrey::init() {
	logger(fmi2OK, "progress", "Starting initialization.");

	if (m_modelExchange) {
		// initialize states
		m_yInput.resize(1);
		m_ydot.resize(1);

		m_yInput[0]	= 10;	// = x
		m_ydot[0]	= 0;	// = \dot{x}
	}
	else {
		// initialize states, these are used for our internal time integration
		m_yInput.resize(1);
		m_yInput[0] = 10;			// = y, initial value
		// initialize integrator for co-simulation
		m_currentTimePoint = 0;
	}

	logger(fmi2OK, "progress", "Initialization complete.");
}


#define A 0.1
#define B 0.02

// for ModelExchange
void LotkaVolterraPrey::updateIfModified() {
	if (!m_externalInputVarsModified)
		return;
	double y = m_realInput[FMI_INPUT_Y];
	double x = m_yInput[0];

	// compute time derivative
	m_ydot[0] = x*(A - B*y);

	// output variable is the same as the conserved quantity
	m_realOutput[FMI_OUTPUT_X] = m_yInput[0];

	// reset externalInputVarsModified flag
	m_externalInputVarsModified = false;
}


// only for Co-simulation
void LotkaVolterraPrey::integrateTo(double tCommunicationIntervalEnd) {

	// state of FMU before integration:
	//   m_currentTimePoint = t_IntervalStart;
	//   m_y[0] = x(t_IntervalStart)
	//   m_realInput[FMI_INPUT_Y] = y(t_IntervalStart...tCommunicationIntervalEnd) = const

	// compute time step size
	double dt = tCommunicationIntervalEnd - m_currentTimePoint;
	double y = m_realInput[FMI_INPUT_Y];
	double x = m_yInput[0];
	double x_end = x*std::exp( (A - B*y)*dt);
	m_yInput[0] = x_end;

	m_tInput = tCommunicationIntervalEnd;
	m_realOutput[FMI_OUTPUT_X] = x_end;
	m_currentTimePoint = tCommunicationIntervalEnd;
}

#undef A
#undef B

void LotkaVolterraPrey::computeFMUStateSize() {
	// distinguish between ModelExchange and CoSimulation
	if (m_modelExchange) {
		// store time, y and ydot, and output
		m_fmuStateSize = sizeof(double)*4;
	}
	else {
		// store time, y and output
		m_fmuStateSize = 3*sizeof(double);
	}
}


void LotkaVolterraPrey::serializeFMUstate(void * FMUstate) {
	double * dataStart = (double*)FMUstate;
	if (m_modelExchange) {
		*dataStart = m_tInput;
		++dataStart;
		*dataStart = m_yInput[0];
		++dataStart;
		*dataStart = m_ydot[0];
		++dataStart;
		*dataStart = m_realOutput[FMI_OUTPUT_X];
	}
	else {
		*dataStart = m_currentTimePoint;
		++dataStart;
		*dataStart = m_yInput[0];
		++dataStart;
		*dataStart = m_realOutput[FMI_OUTPUT_X];
	}
}


void LotkaVolterraPrey::deserializeFMUstate(void * FMUstate) {
	const double * dataStart = (const double*)FMUstate;
	if (m_modelExchange) {
		m_tInput = *dataStart;
		++dataStart;
		m_yInput[0] = *dataStart;
		++dataStart;
		m_ydot[0] = *dataStart;
		++dataStart;
		m_realOutput[FMI_OUTPUT_X] = *dataStart;
		m_externalInputVarsModified = true;
	}
	else {
		m_currentTimePoint = *dataStart;
		++dataStart;
		m_yInput[0] = *dataStart;
		++dataStart;
		m_realOutput[FMI_OUTPUT_X] = *dataStart;
	}
}


