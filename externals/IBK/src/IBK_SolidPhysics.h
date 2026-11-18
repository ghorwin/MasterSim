#ifndef IBK_SolidPhysicsH
#define IBK_SolidPhysicsH

#include <string>

namespace IBK {

/*! Provides reference data and basic calculations for solid materials. */
class Solid {
public:
	enum ReferenceMaterial {
		RM_ExperimentalApproximation,
		RM_Custom,
		RM_Undefined,
		RM_FiredClay,
		RM_ClinkerBrick,
		RM_PortlandCement,
		RM_Gypsum,
		RM_GypsumPlaster,
		RM_AutoclavedAeratedConcrete,
		RM_CalciumSilicate,
		RM_Lime,
		RM_Cementitious,
		RM_LimeCement,
		RM_MineralPlasterMortar,
		RM_EarthClay,
		RM_Quartz,
		RM_Calcite,
		RM_Dolomite,
		RM_Anhydrite,
		RM_Feldspar,
		RM_Muscovite,
		RM_Biotite,
		RM_Hematite,
		RM_Magnetite,
		RM_Granite,
		RM_Basalt,
		RM_Sandstone,
		RM_Limestone,
		RM_Marble,
		RM_Slate,
		RM_Tuff,
		RM_Glass,
		RM_GlassWool,
		RM_StoneWool,
		RM_Perlite,
		RM_Vermiculite,
		RM_FoamGlass,
		RM_Wood,
		RM_Polyethylene,
		RM_Polypropylene,
		RM_Polyester,
		RM_Polystyrene,
		RM_Polyurethane,
		RM_Polyisocyanurate,
		RM_PVC,
		RM_EPDM,
		RM_Bitumen,
		RM_Aluminium,
		RM_Steel,
		RM_StainlessSteel,
		RM_Copper,
		RM_Zinc,
		RM_Lead,
		RM_Cellulose,
		RM_Cork,
		RM_Hemp,
		RM_Flax,
		RM_Straw,
		RM_Spruce,
		RM_Pine,
		RM_Fir,
		RM_Larch,
		RM_Oak,
		RM_Beech,
		RM_Maple,
		RM_Ash,
		RM_Acacia,
		RM_MineralSoil,
		RM_Sand,
		RM_Silt,
		RM_ClaySoil,
		RM_OrganicSoil,
		RM_MineralGeneric,
		RM_WoodBasedGeneric,
		RM_PolymerGeneric,
		RM_Count
	};

	enum DensityQuality {
		DQ_Unknown,
		DQ_A,
		DQ_B,
		DQ_C
	};

	enum SolidCategory {
		SC_Invalid,
		SC_Coating,
		SC_Plaster,
		SC_Bricks,
		SC_NaturalStones,
		SC_Cementitious,
		SC_Insulations,
		SC_BuildingBoards,
		SC_Woodbased,
		SC_NaturalMaterials,
		SC_Soils,
		SC_CladdingSystems,
		SC_Foils,
		SC_Miscellaneous,
		SC_Count
	};

	/*! Returns the particle density in kg/m3. */
	static double particleDensity(ReferenceMaterial material);

	/*! Returns the quality classification of the particle density value. */
	static DensityQuality particleDensityQuality(ReferenceMaterial material);

	/*! Returns the primary material category. */
	static SolidCategory category(ReferenceMaterial material);

	/*! Returns a stable, language-independent identifier for a material. */
	static std::string id(ReferenceMaterial material);

	/*! Converts a stable material identifier to a reference material. */
	static ReferenceMaterial fromId(const std::string &materialId);

	/*! Calculates porosity from dry bulk density and particle density. */
	static double porosity(double bulkDensity, double particleDensity);
};

} // namespace IBK

#endif // IBK_SolidPhysicsH
