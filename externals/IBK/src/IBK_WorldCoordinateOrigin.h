#ifndef WorldCoordinateOriginH
#define WorldCoordinateOriginH

#include "IBK_point.h"

namespace IBK {

/*! Origin point in UTM-coordniates and corresponding UTM Zone, that define a origin point in world coordinates */
class WorldCoordinateOrigin {
public:
	WorldCoordinateOrigin() = default;

	/*! Point in UTM coordinates */
	IBK::point3D<double>		m_origin = IBK::point3D<double>(0,0,0);

	/*! UTM zone
		\attention has to be between 0 and 59
	*/
	int							m_utmZone = 32;

	/*! If true, we have a north utm zone, else it is south */
	bool						m_north = true;
};

} // namespace IBK

#endif // WorldCoordinateOriginH
