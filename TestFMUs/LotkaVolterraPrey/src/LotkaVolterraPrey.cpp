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

// FMI interface variables

#define FMI_OUTPUT_X1 1
#define FMI_OUTPUT_X2 2


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
	// initialize output variables
	m_realOutput[FMI_OUTPUT_X1] = 0;
	m_realOutput[FMI_OUTPUT_X2] = 0;
}


LotkaVolterraPrey::~LotkaVolterraPrey() {
}


// for ModelExchange
void LotkaVolterraPrey::updateIfModified() {
	if (!m_externalInputVarsModified)
		return;

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
void LotkaVolterraPrey::integrateTo(double tCommunicationIntervalEnd) {
	m_tInput = tCommunicationIntervalEnd;
	m_externalInputVarsModified = true;
	updateIfModified();
}


void LotkaVolterraPrey::computeFMUStateSize() {
	m_fmuStateSize = 3*sizeof(double); // time point and both output variables
}


void LotkaVolterraPrey::serializeFMUstate(void * FMUstate) {
	double * dataStart = (double*)FMUstate;
	*dataStart = m_tInput;
	++dataStart;
	*dataStart = m_realOutput[FMI_OUTPUT_X1];
	++dataStart;
	*dataStart = m_realOutput[FMI_OUTPUT_X2];
}


void LotkaVolterraPrey::deserializeFMUstate(void * FMUstate) {
	const double * dataStart = (const double*)FMUstate;
	m_tInput = *dataStart;
	++dataStart;
	m_realOutput[FMI_OUTPUT_X1] = *dataStart;
	++dataStart;
	m_realOutput[FMI_OUTPUT_X2] = *dataStart;
	m_externalInputVarsModified = true;
}


