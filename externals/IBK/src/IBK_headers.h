// Precompiled header file.
// Only frequently used headers should be in here.

// This is for c++ only builds.

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

#include "IBK_algorithm.h"
#include "IBK_ArgParser.h"
#include "IBK_array.h"
#include "IBK_assert.h"
#include "IBK_bitfield.h"
#include "IBK_BuildFlags.h"
#include "IBK_Color.h"
#include "IBK_configuration.h"
#include "IBK_Constants.h"
#include "IBK_crypt.h"
#include "IBK_CSVReader.h"
#include "IBK_cuboid.h"
#include "IBK_Differ.h"
#include "IBK_EOLStreamBuffer.h"
#include "IBK_Exception.h"
#include "IBK_FileReader.h"
#include "IBK_FileUtils.h"
#include "IBK_Flag.h"
#include "IBK_FluidPhysics.h"
#include "IBK_FormatString.h"
#include "IBK_geographic.h"
#include "IBK_InputOutput.h"
#include "IBK_IntPara.h"
#include "IBK_Isopleths.h"
#include "IBK_LinearSpline.h"
#include "IBK_LinearSplineArray.h"
#include "IBK_Line.h"
#include "IBK_Logfile.h"
#include "IBK_math.h"
#include "IBK_matrix_3d.h"
#include "IBK_matrix.h"
#include "IBK_matrix_view.h"
#include "IBK_memory_usage.h"
#include "IBK_MessageHandler.h"
#include "IBK_MessageHandlerRegistry.h"
#include "IBK_messages.h"
#include "IBK_MultiLanguageString.h"
#include "IBK_MultiSpline.h"
#include "IBK_NotificationHandler.h"
#include "IBK_openMP.h"
#include "IBK_Parameter.h"
#include "IBK_Path.h"
#include "IBK_physics.h"
#include "IBK_point.h"
#include "IBK_ptr_list.h"
#include "IBK_Quantity.h"
#include "IBK_QuantityManager.h"
#include "IBK_rectangle.h"
#include "IBK_ScalarFunction.h"
#include "IBK_SimpleString.h"
#include "IBK_SolverArgsParser.h"
#include "IBK_StopWatch.h"
#include "IBK_StringUtils.h"
#include "IBK_system.h"
#include "IBK_Time.h"
#include "IBK_UnitData.h"
#include "IBK_Unit.h"
#include "IBK_UnitList.h"
#include "IBK_UnitVector.h"
#include "IBK_UTM.h"
#include "IBK_Version.h"
#include "IBK_WaitOnExit.h"
#include "IBK_WorldCoordinateOrigin.h"




