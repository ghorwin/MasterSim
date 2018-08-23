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
#include "LotkaVolterraPredator.h"

#include <cmath>

// FMI interface variables

#define FMI_OUTPUT_Y 1
#define FMI_INPUT_X 2


// *** Variables and functions to be implemented in user code. ***

// *** GUID that uniquely identifies this FMU code
const char * const InstanceData::GUID = "{471a3b52-4923-44d8-ab4a-fcdb813c7353}";

// *** Factory function, creates model specific instance of InstanceData-derived class
InstanceData * InstanceData::create() {
	return new LotkaVolterraPredator; // caller takes ownership
}


// create a model instance
LotkaVolterraPredator::LotkaVolterraPredator() :
	InstanceData()
{
	// initialize input variables
	m_realInput[FMI_INPUT_X] = 0;
	// initialize output variables
	m_realOutput[FMI_OUTPUT_Y] = 10;
}


LotkaVolterraPredator::~LotkaVolterraPredator() {
}


// create a model instance
void LotkaVolterraPredator::init() {
	logger(fmi2OK, "progress", "Starting initialization.");

	if (m_modelExchange) {
		// initialize states
		m_yInput.resize(1);
		m_ydot.resize(1);

		m_yInput[0]	= 10;	// = y
		m_ydot[0]	= 0;	// = \dot{y}
	}
	else {
		// initialize states, these are used for our internal time integration
		m_yInput.resize(1);
		m_yInput[0] = 10;			// = x, initial value
		// initialize integrator for co-simulation
		m_currentTimePoint = 0;
	}

	logger(fmi2OK, "progress", "Initialization complete.");
}

#define C 0.4
#define D 0.02


// for ModelExchange
void LotkaVolterraPredator::updateIfModified() {
	if (!m_externalInputVarsModified)
		return;
	double x = m_realInput[FMI_INPUT_X];
	double y = m_yInput[0];

	// compute time derivative
	m_ydot[0] = y*(D*x - C);

	// output variable is the same as the conserved quantity
	m_realOutput[FMI_OUTPUT_Y] = m_yInput[0];

	// reset externalInputVarsModified flag
	m_externalInputVarsModified = false;
}


// only for Co-simulation
void LotkaVolterraPredator::integrateTo(double tCommunicationIntervalEnd) {
	// state of FMU before integration:
	//   m_currentTimePoint = t_IntervalStart;
	//   m_y[0] = y(t_IntervalStart)
	//   m_realInput[FMI_INPUT_X] = x(t_IntervalStart...tCommunicationIntervalEnd) = const

	// compute time step size
	double dt = tCommunicationIntervalEnd - m_currentTimePoint;
	double x = m_realInput[FMI_INPUT_X];
	double y = m_yInput[0];
	double y_end = y*std::exp( (D*x - C)*dt);
	m_yInput[0] = y_end;

	m_tInput = tCommunicationIntervalEnd;
	m_realOutput[FMI_OUTPUT_Y] = y_end;
	m_currentTimePoint = tCommunicationIntervalEnd;
}

#undef C
#undef B

void LotkaVolterraPredator::computeFMUStateSize() {
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


void LotkaVolterraPredator::serializeFMUstate(void * FMUstate) {
	double * dataStart = (double*)FMUstate;
	if (m_modelExchange) {
		*dataStart = m_tInput;
		++dataStart;
		*dataStart = m_yInput[0];
		++dataStart;
		*dataStart = m_ydot[0];
		++dataStart;
		*dataStart = m_realOutput[FMI_OUTPUT_Y];
	}
	else {
		*dataStart = m_currentTimePoint;
		++dataStart;
		*dataStart = m_yInput[0];
		++dataStart;
		*dataStart = m_realOutput[FMI_OUTPUT_Y];
	}
}


void LotkaVolterraPredator::deserializeFMUstate(void * FMUstate) {
	const double * dataStart = (const double*)FMUstate;
	if (m_modelExchange) {
		m_tInput = *dataStart;
		++dataStart;
		m_yInput[0] = *dataStart;
		++dataStart;
		m_ydot[0] = *dataStart;
		++dataStart;
		m_realOutput[FMI_OUTPUT_Y] = *dataStart;
		m_externalInputVarsModified = true;
	}
	else {
		m_currentTimePoint = *dataStart;
		++dataStart;
		m_yInput[0] = *dataStart;
		++dataStart;
		m_realOutput[FMI_OUTPUT_Y] = *dataStart;
	}
}


