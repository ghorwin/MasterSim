############################################
#
# Global Options (Should be written by some external script and not be commited!)
# This Options File is a ONE of many selection,
# There should never be more then one option defined!!!
#
############################################

# The following define disables the defines that rename the message handler and unit list for mastersim builds
# With the define on, all IBK-based applications use the same message handler. The last one to install the message handler
# will recieve all output.

# With the define off, the singleton class names for MessageHandlerRegistry and UnitLists are defined differently for MasterSim,
# causing a different instance of the message handler to be created compared to other shared libs that depend on IBK.
# Unfortunately, the IBK library is then no longer identical to other IBK libraries and will cause a linker error when
# loading a shared library that was linked against a regular IBK library. This could potentially be fixed by adjusting the
# library name as well.

# A central problem remains, though. When two shared libraries dynamically link against the IBK library, they will always
# share the MessageHandlerRegistry and UnitList singleton objects.
DEFINES += QT_DEBUG_BUILD

# This option enables extensive runtime checks.
#OPTIONS += sanitize_checks

#
# This option enables (default) mpi support and switches to MPICC compiler
#
#OPTIONS += mpi

#
# This option eables explicit mpi + icc support
#
#OPTIONS += mpiICC

#
# This option eables explicit mpi + gcc support
#
#OPTIONS += mpiGCC

#
# This option eables explicit vampire trace + mpi + icc support
#
#OPTIONS += vapireTraceICC


#
# This option eables explicit vampire trace + mpi + gcc support
#
#OPTIONS += vapireTraceGCC


#
# This option enables open mp support in compiler.
#
#OPTIONS += openmp

#
# This option enables vampire openmp support in compiler.
#
#OPTIONS += openmpVapireTraceICC

#
# This option enables vampire openmp support in compiler.
#
#OPTIONS += openmpVapireTraceGCC


#
# This option enables gprof support in compiler.
#
#OPTIONS += gprof

#
# This option enables Lapack/Blas support on Linux
#
#OPTIONS += lapack
