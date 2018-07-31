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

#include <memory>

const char * const InstanceData::GUID = "{471a3b52-4923-44d8-ab4a-fcdb813c7322}";

InstanceData * createInstanceData() {
	return new Math003Part1;
}


Math003Part1::Math003Part1() :
	InstanceData()
{
}


Math003Part1::~Math003Part1() {
}


// create a model instance
void Math003Part1::init() {
	const char * const FUNC_ID = "[Math003Part1init]";

	logger(fmi2OK, "progress", "Starting initialization.");

	if (m_modelExchange) {
		// initialize states
	}
	else {
		// initialize integrator for co-simulation
	}

	logger(fmi2OK, "progress", "Initialization complete.");
}


void Math003Part1::updateIfModified() {
//	const char * const FUNC_ID = "[Math003Part1::updateIfModified]";

//	IBK::IBK_Message(IBK::FormatString("Model was modified for model time: %1 and master time %2.\n")
//				   .arg(m_model.m_t).arg(m_tInput), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);
	// reset externalInputVarsModified flag
	m_externalInputVarsModified = false;
	// copy computed derivatives to cached vector
//	m_ydot = m_theraklesModel.m_ydot;
}


// only for Co-simulation
void Math003Part1::integrateTo(double tCommunicationIntervalEnd) {
	const char * const FUNC_ID = "[Math003Part1::integrateTo]";

	double tCommunicationIntervalStart = 0; //
	// reset is not implemented, yet
	if (tCommunicationIntervalStart > tCommunicationIntervalEnd) {
		throw IBK::Exception(IBK::FormatString("Error performing integration step: "
			"reset from integrator time point %1 to the new communication interval end %2 is not "
			" supported, yet!")
			.arg(tCommunicationIntervalStart)
			.arg(tCommunicationIntervalEnd),
			FUNC_ID);
	}

	/* 2.) Set a new tStop value (= tCommunicationIntervalEnd) in integrator*/
//	SOLFRA::IntegratorInterface *integrator		 = m_theraklesModel.integratorInterface();

//	SOLFRA::IntegratorErrorControlled* integratorErrorControlled =
//		dynamic_cast<SOLFRA::IntegratorErrorControlled*> (integrator);
//	// stop value is still only defined for error controled integrators
//	if(integratorErrorControlled != NULL)
//		integratorErrorControlled->m_stopTime = tCommunicationIntervalEnd;

//	/* 3.) integration loop until communication interval has been reached:*/
//	SOLFRA::OutputScheduler * outputScheduler = &m_theraklesModel;

	// Use this interrupt to stop execution and allow debugger to attach
	//__asm int 3;
#if 0
	try {

		// outputs are only written if we start the simulation from begin, therefore
		// we pass t0 and y0 for the initial model evaluation within writeOutputs()
		if (tCommunicationIntervalStart == m_theraklesModel.t0()) {
			m_theraklesModel.writeOutputs( m_theraklesModel.t0(), m_theraklesModel.y0());
		}

		// reset outputs written past communication interval start
		m_theraklesModel.startCommunicationInterval(tCommunicationIntervalStart);

		// update FMI input quantities
		if (m_externalInputVarsModified) {
			m_theraklesModel.updateFromModifiedInputQuantities();
			m_externalInputVarsModified = false;
		}

		// integration loop
		double t = tCommunicationIntervalStart;
		double tOutput = outputScheduler->nextOutputTime(t);

		while (t < tCommunicationIntervalEnd) {

			// (contains parallel code)
			SOLFRA::IntegratorInterface::StepResultType res =integrator->step();
			if (res != SOLFRA::IntegratorInterface::StepSuccess) {
				throw IBK::Exception("Integrator step failed!", FUNC_ID);
			}

			// get new time point and time step size
			t = integrator->t();
			//logger(fmi2OK, "Progress", IBK::FormatString("   step end at t=%1 s").arg(t).str().c_str());

			// notify model of completed step (contains parallel code)
			const double * y_current = integrator->yOut( t );

			m_theraklesModel.stepCompleted(t, y_current);

			while (tOutput <= t) {

				// retrieve interpolated states at output time point (contains parallel code)
				const double * yOutput = integrator->yOut(tOutput);
				double tOutputNext = 0.0;

				// tell m_model to write outputs
				m_theraklesModel.writeOutputs( tOutput, yOutput);

				// retrieve new output time point
				tOutputNext = outputScheduler->nextOutputTime(tOutput);
				if (tOutputNext <= tOutput)
					throw IBK::Exception(IBK::FormatString("Output scheduler returned output time %1, which is <= last output time %2.")
						.arg(tOutputNext).arg(tOutput), FUNC_ID);

				tOutput = tOutputNext;

				// if t_outNext is > t_end (including some rounding error compensation), we break the loop
				if (tOutputNext > tCommunicationIntervalEnd*(1+1e-10))
					break;

			} // while (tOutput <= t) {

		} // while (t < tCommunicationIntervalEnd)
		m_theraklesModel.completeCommunicationInterval();
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
	}
	catch (std::exception & ex) {
		IBK::IBK_Message( IBK::FormatString("Exception caught: %1").arg(ex.what()), IBK::MSG_ERROR);
	}
#endif
}


void Math003Part1::computeFMUStateSize() {
#if 0
	const char * const FUNC_ID = "[Math003Part1computeFMUStateSize]";
	IBK_ASSERT(!m_modelExchange);
	// ask all components of the integration framework for size
	SOLFRA::IntegratorInterface *integrator = m_theraklesModel.integratorInterface();
	SOLFRA::LESInterface *lesSolver = m_theraklesModel.lesInterface();
	SOLFRA::PrecondInterface  *precond  = m_theraklesModel.preconditionerInterface();
	SOLFRA::JacobianInterface *jacobian = m_theraklesModel.jacobianInterface();

	m_fmuStateSize = 8; // 8 bytes for leading size header

	size_t s = integrator->serializationSize();
	if (s == 0)
		throw IBK::Exception("Integrator does not support serialization.", FUNC_ID);
	m_fmuStateSize += s;

	if (lesSolver != NULL) {
		s = lesSolver->serializationSize();
		if (s == 0)
			throw IBK::Exception("LES solver does not support serialization.", FUNC_ID);
		m_fmuStateSize += s;
	}

	if (jacobian != NULL) {
		s = jacobian->serializationSize();
		if (s == 0)
			throw IBK::Exception("Jacobian matrix generator does not support serialization.", FUNC_ID);
		m_fmuStateSize += s;
	}

	if (precond != NULL) {
		s = precond->serializationSize();
		if (s == 0)
			throw IBK::Exception("Preconditioner does not support serialization.", FUNC_ID);
		m_fmuStateSize += s;
	}
#endif
}


void Math003Part1::serializeFMUstate(void * FMUstate) {
#if 0
	IBK_ASSERT(!m_modelExchange);

	// ask all components of the integration framework for size
	SOLFRA::IntegratorInterface *integrator = m_theraklesModel.integratorInterface();
	SOLFRA::LESInterface *lesSolver = m_theraklesModel.lesInterface();
	SOLFRA::PrecondInterface  *precond  = m_theraklesModel.preconditionerInterface();
	SOLFRA::JacobianInterface *jacobian = m_theraklesModel.jacobianInterface();

	void * dataStart = (char*)FMUstate + 8;
	integrator->serialize(dataStart);
	if (lesSolver != NULL)
		lesSolver->serialize(dataStart);
	if (jacobian != NULL)
		jacobian->serialize(dataStart);
	if (precond != NULL)
		precond->serialize(dataStart);
#endif
}


void Math003Part1::deserializeFMUstate(void * FMUstate) {
#if 0
	IBK_ASSERT(!m_modelExchange);

	// ask all components of the integration framework for size
	SOLFRA::IntegratorInterface *integrator = m_theraklesModel.integratorInterface();
	SOLFRA::LESInterface *lesSolver = m_theraklesModel.lesInterface();
	SOLFRA::PrecondInterface  *precond  = m_theraklesModel.preconditionerInterface();
	SOLFRA::JacobianInterface *jacobian = m_theraklesModel.jacobianInterface();

	void * dataStart = (char*)FMUstate + 8;
	integrator->deserialize(dataStart);
	if (lesSolver != NULL)
		lesSolver->deserialize(dataStart);
	if (jacobian != NULL)
		jacobian->deserialize(dataStart);
	if (precond != NULL)
		precond->deserialize(dataStart);
#endif
}


