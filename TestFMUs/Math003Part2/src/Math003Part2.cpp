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
#include "Math003Part2.h"

// FMI interface variables

#define FMI_INPUT_X1 1
#define FMI_INPUT_X2 2
#define FMI_INPUT_X4 4
#define FMI_OUTPUT_X3 3

// *** Variables and functions to be implemented in user code. ***

// *** GUID that uniquely identifies this FMU code
const char * const InstanceData::GUID = "{471a3b52-4923-44d8-ab4a-fcdb813c7323}";

// *** Factory function, creates model specific instance of InstanceData-derived class
InstanceData * InstanceData::create() {
	return new Math003Part2; // caller takes ownership
}


Math003Part2::Math003Part2() :
	InstanceData()
{
	// initialize input variables
	m_realInput[FMI_INPUT_X1] = 0;
	m_realInput[FMI_INPUT_X2] = 0;
	m_realInput[FMI_INPUT_X4] = 0;

	// initialize output variables
	m_realOutput[FMI_OUTPUT_X3] = 0;
}


Math003Part2::~Math003Part2() {
}


void Math003Part2::updateIfModified() {
	if (!m_externalInputVarsModified)
		return;
	// retrieve input variables
	double x1 = m_realInput[FMI_INPUT_X1];
	double x2 = m_realInput[FMI_INPUT_X2];
	double x4 = m_realInput[FMI_INPUT_X4];

	if (x1 == 1 && x2 < 0.01 && x4 < 2.5)
		m_realOutput[FMI_OUTPUT_X3] = 3;
	else if (x1 < 0.001 && x2 > 0 && x4 > -2.5)
		m_realOutput[FMI_OUTPUT_X3] = -3;
	else
		m_realOutput[FMI_OUTPUT_X3] = 0;

	// reset externalInputVarsModified flag
	m_externalInputVarsModified = false;
}


// only for Co-simulation
void Math003Part2::integrateTo(double tCommunicationIntervalEnd) {
	m_tInput = tCommunicationIntervalEnd;
	m_externalInputVarsModified = true;
	updateIfModified();
}


void Math003Part2::computeFMUStateSize() {
	m_fmuStateSize = sizeof(double); // only x3 variable
}


void Math003Part2::serializeFMUstate(void * FMUstate) {
	double * dataStart = (double*)FMUstate;
	*dataStart = m_realOutput[FMI_OUTPUT_X3];
}


void Math003Part2::deserializeFMUstate(void * FMUstate) {
	const double * dataStart = (const double*)FMUstate;
	m_realOutput[FMI_OUTPUT_X3] = *dataStart;
}


