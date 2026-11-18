#include "IBK_SolidPhysics.h"

#include "IBK_Exception.h"

namespace IBK {

double Solid::particleDensity(ReferenceMaterial material)
{
	switch (material) {
		case RM_FiredClay: return 2700;
		case RM_ClinkerBrick: return 2700;
		case RM_PortlandCement: return 3150;
		case RM_Gypsum: return 2320;
		case RM_GypsumPlaster: return 2720;
		case RM_AutoclavedAeratedConcrete: return 2500;
		case RM_CalciumSilicate: return 2600;
		case RM_Lime: return 2700;
		case RM_Cementitious: return 2700;
		case RM_LimeCement: return 2700;
		case RM_MineralPlasterMortar: return 2700;
		case RM_EarthClay: return 2650;
		case RM_Quartz: return 2650;
		case RM_Calcite: return 2710;
		case RM_Dolomite: return 2850;
		case RM_Anhydrite: return 2960;
		case RM_Feldspar: return 2650;
		case RM_Muscovite: return 2850;
		case RM_Biotite: return 3100;
		case RM_Hematite: return 5300;
		case RM_Magnetite: return 5200;
		case RM_Granite: return 2700;
		case RM_Basalt: return 2900;
		case RM_Sandstone: return 2650;
		case RM_Limestone: return 2700;
		case RM_Marble: return 2750;
		case RM_Slate: return 2800;
		case RM_Tuff: return 2600;
		case RM_Polyethylene: return 970;
		case RM_Polypropylene: return 910;
		case RM_Polyester: return 1390;
		case RM_Polystyrene: return 1050;
		case RM_Polyurethane: return 1150;
		case RM_Polyisocyanurate: return 1200;
		case RM_PVC: return 1400;
		case RM_EPDM: return 1150;
		case RM_Bitumen: return 1030;
		case RM_Aluminium: return 2700;
		case RM_Steel: return 7860;
		case RM_StainlessSteel: return 8000;
		case RM_Copper: return 8960;
		case RM_Zinc: return 7140;
		case RM_Lead: return 11340;
		case RM_GlassWool: return 2500;
		case RM_StoneWool: return 2850;
		case RM_Perlite: return 2350;
		case RM_Vermiculite: return 2500;
		case RM_FoamGlass: return 2500;
		case RM_Wood: return 1500;
		case RM_Glass: return 2500;
		case RM_Spruce: return 1460;
		case RM_Pine: return 1480;
		case RM_Fir: return 1480;
		case RM_Larch: return 1490;
		case RM_Oak: return 1530;
		case RM_Beech: return 1530;
		case RM_Maple: return 1540;
		case RM_Ash: return 1540;
		case RM_Acacia: return 1530;
		case RM_Cellulose: return 1550;
		case RM_Cork: return 1500;
		case RM_Hemp: return 1500;
		case RM_Flax: return 1500;
		case RM_Straw: return 1500;
		case RM_MineralSoil: return 2650;
		case RM_Sand: return 2650;
		case RM_Silt: return 2650;
		case RM_ClaySoil: return 2700;
		case RM_OrganicSoil: return 1400;
		case RM_MineralGeneric: return 2700;
		case RM_WoodBasedGeneric: return 1500;
		case RM_PolymerGeneric: return 1200;
		case RM_ExperimentalApproximation:
		case RM_Custom:
		case RM_Undefined:
		case RM_Count:
			return 0;
	}
	return 0;
}


Solid::DensityQuality Solid::particleDensityQuality(ReferenceMaterial material)
{
	switch (material) {
		case RM_FiredClay:
		case RM_PortlandCement:
		case RM_Quartz:
		case RM_Calcite:
		case RM_GlassWool:
		case RM_Wood:
		case RM_Polystyrene:
		case RM_Aluminium:
		case RM_Steel:
		case RM_Copper:
		case RM_Zinc:
		case RM_Lead:
			return DQ_A;
		case RM_ClinkerBrick:
		case RM_Gypsum:
		case RM_GypsumPlaster:
		case RM_AutoclavedAeratedConcrete:
		case RM_Anhydrite:
		case RM_Feldspar:
		case RM_Muscovite:
		case RM_Hematite:
		case RM_Magnetite:
		case RM_Granite:
		case RM_Limestone:
		case RM_Marble:
		case RM_Slate:
		case RM_Glass:
		case RM_Spruce:
		case RM_Pine:
		case RM_Fir:
		case RM_Larch:
		case RM_Oak:
		case RM_Beech:
		case RM_Maple:
		case RM_Ash:
		case RM_Acacia:
		case RM_Cellulose:
		case RM_Polyethylene:
		case RM_Polypropylene:
		case RM_Polyester:
		case RM_StainlessSteel:
		case RM_Sand:
		case RM_Silt:
		case RM_MineralSoil:
		case RM_FoamGlass:
			return DQ_B;
		case RM_CalciumSilicate:
		case RM_Lime:
		case RM_Cementitious:
		case RM_LimeCement:
		case RM_MineralPlasterMortar:
		case RM_EarthClay:
		case RM_Dolomite:
		case RM_Biotite:
		case RM_Basalt:
		case RM_Sandstone:
		case RM_Tuff:
		case RM_StoneWool:
		case RM_Perlite:
		case RM_Vermiculite:
		case RM_Polyurethane:
		case RM_Polyisocyanurate:
		case RM_PVC:
		case RM_EPDM:
		case RM_Bitumen:
		case RM_Cork:
		case RM_Hemp:
		case RM_Flax:
		case RM_Straw:
		case RM_ClaySoil:
		case RM_OrganicSoil:
		case RM_MineralGeneric:
		case RM_WoodBasedGeneric:
		case RM_PolymerGeneric:
			return DQ_C;
		case RM_ExperimentalApproximation:
		case RM_Custom:
		case RM_Undefined:
		case RM_Count:
			return DQ_Unknown;
	}
	return DQ_Unknown;
}


Solid::SolidCategory Solid::category(ReferenceMaterial material)
{
	switch (material) {
		case RM_FiredClay:
		case RM_ClinkerBrick:
			return SC_Bricks;
		case RM_Gypsum:
		case RM_GypsumPlaster:
		case RM_MineralPlasterMortar:
		case RM_EarthClay:
			return SC_Plaster;
		case RM_PortlandCement:
		case RM_AutoclavedAeratedConcrete:
		case RM_CalciumSilicate:
		case RM_Cementitious:
		case RM_Lime:
		case RM_LimeCement:
			return SC_Cementitious;
		case RM_Quartz:
		case RM_Calcite:
		case RM_Dolomite:
		case RM_Anhydrite:
		case RM_Feldspar:
		case RM_Muscovite:
		case RM_Biotite:
		case RM_Hematite:
		case RM_Magnetite:
		case RM_Granite:
		case RM_Basalt:
		case RM_Sandstone:
		case RM_Limestone:
		case RM_Marble:
		case RM_Slate:
		case RM_Tuff:
			return SC_NaturalStones;
		case RM_GlassWool:
		case RM_StoneWool:
		case RM_Perlite:
		case RM_Vermiculite:
		case RM_FoamGlass:
			return SC_Insulations;
		case RM_Wood:
		case RM_Spruce:
		case RM_Pine:
		case RM_Fir:
		case RM_Larch:
		case RM_Oak:
		case RM_Beech:
		case RM_Maple:
		case RM_Ash:
		case RM_Acacia:
		case RM_Cellulose:
			return SC_Woodbased;
		case RM_Cork:
		case RM_Hemp:
		case RM_Flax:
		case RM_Straw:
			return SC_NaturalMaterials;
		case RM_MineralSoil:
		case RM_Sand:
		case RM_Silt:
		case RM_ClaySoil:
		case RM_OrganicSoil:
			return SC_Soils;
		case RM_Polyethylene:
		case RM_Polypropylene:
		case RM_Polyester:
		case RM_Polystyrene:
		case RM_Polyurethane:
		case RM_Polyisocyanurate:
		case RM_PVC:
		case RM_EPDM:
		case RM_Bitumen:
		case RM_Aluminium:
		case RM_Steel:
		case RM_StainlessSteel:
		case RM_Copper:
		case RM_Zinc:
		case RM_Lead:
		case RM_Glass:
		case RM_MineralGeneric:
		case RM_WoodBasedGeneric:
		case RM_PolymerGeneric:
			return SC_Miscellaneous;
		case RM_ExperimentalApproximation:
		case RM_Custom:
		case RM_Undefined:
		case RM_Count:
			return SC_Invalid;
	}
	return SC_Invalid;
}


std::string Solid::id(ReferenceMaterial material)
{
	switch (material) {
		case RM_ExperimentalApproximation: return "experimental_approximation";
		case RM_Custom: return "custom";
		case RM_Undefined: return "undefined";
		case RM_FiredClay: return "fired_clay";
		case RM_ClinkerBrick: return "clinker_brick";
		case RM_PortlandCement: return "portland_cement";
		case RM_Gypsum: return "gypsum";
		case RM_GypsumPlaster: return "gypsum_plaster";
		case RM_AutoclavedAeratedConcrete: return "autoclaved_aerated_concrete";
		case RM_CalciumSilicate: return "calcium_silicate";
		case RM_Lime: return "lime";
		case RM_Cementitious: return "cementitious";
		case RM_LimeCement: return "lime_cement";
		case RM_MineralPlasterMortar: return "mineral_plaster_mortar";
		case RM_EarthClay: return "earth_clay";
		case RM_Quartz: return "quartz";
		case RM_Calcite: return "calcite";
		case RM_Dolomite: return "dolomite";
		case RM_Anhydrite: return "anhydrite";
		case RM_Feldspar: return "feldspar";
		case RM_Muscovite: return "muscovite";
		case RM_Biotite: return "biotite";
		case RM_Hematite: return "hematite";
		case RM_Magnetite: return "magnetite";
		case RM_Granite: return "granite";
		case RM_Basalt: return "basalt";
		case RM_Sandstone: return "sandstone";
		case RM_Limestone: return "limestone";
		case RM_Marble: return "marble";
		case RM_Slate: return "slate";
		case RM_Tuff: return "tuff";
		case RM_GlassWool: return "glass_wool";
		case RM_StoneWool: return "stone_wool";
		case RM_Perlite: return "perlite";
		case RM_Vermiculite: return "vermiculite";
		case RM_FoamGlass: return "foam_glass";
		case RM_Wood: return "wood";
		case RM_Polyethylene: return "polyethylene";
		case RM_Polypropylene: return "polypropylene";
		case RM_Polyester: return "polyester";
		case RM_Polystyrene: return "polystyrene";
		case RM_Polyurethane: return "polyurethane";
		case RM_Polyisocyanurate: return "polyisocyanurate";
		case RM_PVC: return "pvc";
		case RM_EPDM: return "epdm";
		case RM_Bitumen: return "bitumen";
		case RM_Aluminium: return "aluminium";
		case RM_Steel: return "steel";
		case RM_StainlessSteel: return "stainless_steel";
		case RM_Copper: return "copper";
		case RM_Zinc: return "zinc";
		case RM_Lead: return "lead";
		case RM_Cellulose: return "cellulose";
		case RM_Cork: return "cork";
		case RM_Hemp: return "hemp";
		case RM_Flax: return "flax";
		case RM_Straw: return "straw";
		case RM_Glass: return "glass";
		case RM_Spruce: return "spruce";
		case RM_Pine: return "pine";
		case RM_Fir: return "fir";
		case RM_Larch: return "larch";
		case RM_Oak: return "oak";
		case RM_Beech: return "beech";
		case RM_Maple: return "maple";
		case RM_Ash: return "ash";
		case RM_Acacia: return "acacia";
		case RM_MineralSoil: return "mineral_soil";
		case RM_Sand: return "sand";
		case RM_Silt: return "silt";
		case RM_ClaySoil: return "clay_soil";
		case RM_OrganicSoil: return "organic_soil";
		case RM_MineralGeneric: return "mineral_generic";
		case RM_WoodBasedGeneric: return "wood_based_generic";
		case RM_PolymerGeneric: return "polymer_generic";
		case RM_Count: return "";
	}
	return "";
}


Solid::ReferenceMaterial Solid::fromId(const std::string &materialId)
{
	for (int i = 0; i < static_cast<int>(RM_Count); ++i) {
		const ReferenceMaterial material = static_cast<ReferenceMaterial>(i);
		if (id(material) == materialId)
			return material;
	}
	return RM_Custom;
}


double Solid::porosity(double bulkDensity, double particleDensityValue)
{
	if (bulkDensity <= 0 || particleDensityValue <= 0 || particleDensityValue <= bulkDensity)
		throw IBK::Exception("Bulk density must be positive and smaller than particle density.",
								"IBK::Solid::porosity");
	return 1.0 - bulkDensity / particleDensityValue;
}

} // namespace IBK
