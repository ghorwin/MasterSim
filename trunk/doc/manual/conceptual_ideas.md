
Initial Conditions
------------------------

Basically, consistent initial conditions can be obtained by repeatedly setting inputs and retrieving outputs of slaves **without** calling `doStep()` in between. Hence, the states of the slaves do not have to be stored and retrieved, so initial condition iteration is also possible with FMI v1.

The master algorithm implementations can be recycled, when the calls to the following functions

- `doStep()`
- `currentState()`   (only FMI2)
- `setState()` (only FMI2)

are disabled by a flag.

Unconnected inputs need to be set to their start-values only once. Also, parameters need to be set by the master only once *before* the initial condition iteration.

Convergence Test
--------------------------

For all iterative algorithms, convergence is found when:

- all integer values match
- all boolean values match
- all string values match
- the WRMS norm of all real values match

Weighted root mean square norm (WRMS-norm) is computed as follows:

~~~
#!code.cpp
	double norm = 0;
	for (unsigned i=0; i<m_realytNextIter.size(); ++i) {
		double diff = m_realytNextIter[i] - m_realytNext[i];
		double absValue = std::fabs(m_realytNextIter[i]);
		double weight = absValue*m_project.m_relTol + m_project.m_absTol;
		diff /= weight;
		norm += diff*diff;
	}

	norm = std::sqrt(norm);
~~~


The paper can be downloaded from the MASTERSIM Webpage -> documentation section.

Meaningful Combinations of Algorithms
---------------------------------------------------------

### 1. Gauss-Jacobi (non-iterating, no error test) ###

* works with FMI v1 and v2
* no iteration
* no error test
* fixed step size

Limitations on error and stability. Optionally supports parallelization (see performance tweaks discussion).

### 2. Gauss-Seidel (non-iterating, no error test) ###

* works with FMI v1 and v2
* no iteration
* no error test
* fixed step size

Limitations on error and stability. A bit better than Gauss-Jacobi.

### 3. Gauss-Seidel (non-iterating, with error test) ###

* works with FMI v2
* no iteration
* error test
* adaptive step size, step size is adjusted based on convergence rate

### 4. Gauss-Seidel (iterating, fixed step, with optional a priori error test) ###

* requires FMI v2
* uses iteration
* optional error test, failure to pass error test will stop the master
* fixed step size, failure to converge will stop the master

Fixing the time step allows performance comparison with other algorithms.

### 5. Gauss-Seidel (iterating with optional error test) ###

* requires FMI v2
* uses iteration
* optional error test, failure to pass error test will stop the master
* adaptive step size (reduction on convergence failure, and increase on fast convergence, reduction on error test failure)

Stable due to time step reduction, error control possible.

### 6. Newton (iterating with optional error test) ###

* requires FMI v2
* uses iteration
* optional error test
* adaptive step size (reduction on convergence failure, and increase on fast convergence, reduction on error test failure)

Stable due to time step reduction, error control possible. Convergence rate and success improved over Gauss-Seidel.

## FMU Requirements ##

Depending on the selected algorithmic options, FMU must have certain capabilities:

### Using time step adjustment ###

* FMU can handle variable time steps

### Using iteration ###

* FMU can be set back (FMI v2)

### Using Error Control with step size adaptation ###

* FMU can be set back (FMI v2)
* FMU can handle variable time steps



Master Algorithms
--------------------------

### Starting Conditions ###

All algorithms start with the following conditions:

* all slaves are at time level t
* output-variable cache of slaves are updated to time level t
* global variable vectors are updated to time level t

When iteration is enabled *and* step adjustment is enabled, the FMU slave states are expected to be stored already, otherwise they will be stored at the begin of an iterating algorithm. 

### Gauss-Jacobi ###
**Note:** MasterSim only implements the non-iterative Gauss-Jacobi algorithm because and two Gauss-Jacobi-Iterations correspond two non-iterative steps with half the step size - and the latter version will be even more accurate and stable. Therefore Gauss-Jacobi is implemented always without iteration, error checking and time step adjustment.

~~~
loop all cycles:
  
  loop all slaves in cycle:

    set inputs for slave using variables from time level t
    advance slave in time (doStep() and caching of outputs)
    sync cached outputs with variables vector for time level t+1

copy variables from time level t+1 to variables vector of time level t
~~~

### Gauss-Seidel Iterative ###

Whether iterative or non-iterative version is used, is determined by iteration limit (==1, no iteration).

Solves is the fixed-point iteration

     x* = S(x)

where `S(x)` is the result of the evaluation of all slaves and mapping of outputs to inputs.


~~~
copy variables from time level t to variables vector of time level t+1

loop all cycles:

  loop iteration < maxIterations:
  
    if iterating:
      copy variables from time level t+1 to backup vector (for convergence check)
      if iteration > 1:
        restoreSlaveStates()
        
    loop all slaves in cycle:

      set inputs for slave using variables from time level t+1 (partially updated from previous slaves)
      advance slave in time (doStep() and caching of outputs)
      sync cached outputs with variables vector for time level t+1

  if iterating and numSlaves > 1:
    doConvergenceTest()
    if success:
      break

copy variables from time level t+1 to variables vector of time level t
~~~

Specific features of Gauss-Seidel implementation:

- for cycles with one slave no iteration is done, so no rollback is needed
- if slaves report discontinous outputs (from state events/time events) Gauss-Seidel may not converge, if the outer time step reduction loop reduces the time step below a certain limit the algorithm falls back to *non iterative Gauss-Seidel*

### Newton ###
The Newton algorithm is always iterative. The algorithm employs a modified Newton method, where the Jacobian is updated only once at the begin of each step.

The root finding problem stems from the re-arranged fixed-point iteration:

    0 = x - S(x) = G(x)
    
with the Jacobian

    dG/dx = I - dS/dx

in the algorithm, each cycle is treated individually. Cycles with a single slave are not iterated. Cycles with no coupling variables (that means, not really a cycle) are not treated as Newton.

~~~



~~~

Time Step Adjustment Method
-------------------------------------------

Variable communication steps are implemented only for FMUs v2 with rollback capabilities.

Principle algorithm:

~~~
loop until t > tEnd:
  loop until converged and error test has passed:
    take step
    if not converged:
      reduce step and try again
    do error test
    if error too large:
      reduce step and try again
  
  estimate step size for next step
~~~

### Error Estimation ###

The error test is done with the step-doubling technique. First a step is computed (it has converged in case of iterative algorithm). The same step is then calculated again but with two steps of halved size, yielding a solution of higher order.
The difference between method orders is used to estimate the local truncation error. This is done in analogy to Backward Euler which has method order 1. Suppose you have solution yh as the solution with the original step and yh2 as solution with halved steps, the error is estimated by:

    error = 2 || yh - yh2 ||
    
Since we deal with vectors, a suitable norm (WRMS norm) is used to compute the scalars yh and yh2.


### Implementation ###

The error test is optional and done right after a solution has converged. In this case the new solution (for time point t + stepsize) is stored in temporary vectors xxxNext. 

The error test algorithm copies this solution to vectors xxxFirst. It then resets the slaves to time point t and computes two steps of size stepsize/2. If any of the steps fails to converge during this attempt, the step size is **not** adjusted but the error test is marked as failed (when the long step converges, the smaller ones must converge as well, unless something significant has happened in the middle of the long step).

### Rounding Error Considerations ###

When taking two steps of size h2 = h/2 rounding errors can lead to states ending up at time `2*h2 != h`.  Since at end of error test slaves are positioned at time `t + 2*h2` we recompute the actual long step size  `h = 2*h2` and store that as long step size.

## Improving Estimated Solution with Richardson Extrapolation ##

For the error test we have to take 3 communication steps instead of 1 for a single interval. This extra work can also be used to generate a better estimate for the solution by combining the solution of the single step with the dual step solution (of higher order). With the Richardson extrapolation both solution are combined to create a better estimate for the result.



