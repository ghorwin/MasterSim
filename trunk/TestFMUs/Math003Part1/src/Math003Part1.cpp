/*	FMI Interface for the MasterSim Test Cases

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

/*! Model identifier to construct dll function names. 1 stands for first version of FMI. */
#define MODEL_IDENTIFIER Math003Part1

#include "fmi2common/fmi2Functions.h"
#include "fmi2common/fmi2FunctionTypes.h"
#include "Math003Part1.h"

// FMI interface variables

#define FMI_OUTPUT_X1 1
#define FMI_OUTPUT_X2 2


// *** Variables and functions to be implemented in user code. ***

// *** GUID that uniquely identifies this FMU code
const char * const InstanceData::GUID = "{471a3b52-4923-44d8-ab4a-fcdb813c7322}";

// *** Factory function, creates model specific instance of InstanceData-derived class
InstanceData * InstanceData::create() {
	return new Math003Part1; // caller takes ownership
}


Math003Part1::Math003Part1() :
	InstanceData()
{
}


Math003Part1::~Math003Part1() {
}


// create a model instance
void Math003Part1::init() {
	logger(fmi2OK, "progress", "Starting initialization.");

	// we only need one output variable, with ID 1
	m_realOutput[FMI_OUTPUT_X1] = 0; // initial value
	m_realOutput[FMI_OUTPUT_X2] = 0; // initial value

	logger(fmi2OK, "progress", "Initialization complete.");
}


void Math003Part1::updateIfModified() {
	if (m_tInput < 1 || (m_tInput >= 2 && m_tInput < 5))
		m_realOutput[FMI_OUTPUT_X1] = 0;
	else
		m_realOutput[FMI_OUTPUT_X1] = 1;

	if (m_tInput < 3 || (m_tInput >= 4 && m_tInput < 6))
		m_realOutput[FMI_OUTPUT_X2] = 0;
	else
		m_realOutput[FMI_OUTPUT_X2] = 1;

	// mark variables as updated
	m_externalInputVarsModified = false;
}


// only for Co-simulation
void Math003Part1::integrateTo(double tCommunicationIntervalEnd) {
	m_tInput = tCommunicationIntervalEnd;
	updateIfModified();
}


void Math003Part1::computeFMUStateSize() {
	m_fmuStateSize = sizeof(double); // only time point
}


void Math003Part1::serializeFMUstate(void * FMUstate) {
	double * dataStart = (double*)FMUstate;
	*dataStart = m_tInput;
}


void Math003Part1::deserializeFMUstate(void * FMUstate) {
	const double * dataStart = (const double*)FMUstate;
	m_tInput = *dataStart;
}


