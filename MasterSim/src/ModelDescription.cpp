/* --------------------------------------------------------------
 * Copyright (c) 2013, AIT Austrian Institute of Technology GmbH.
 * All rights reserved. See file FMIPP_LICENSE for details.
 * --------------------------------------------------------------*/

/**
 * \file ModelDescription.cpp
 */

#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

//#include "common/fmi_v1.0/fmiModelTypes.h"

#include "ModelDescription.h"
//#include "import/base/include/ModelManager.h"
//#include "import/base/include/PathFromUrl.h"


using namespace std;
using namespace ModelDescriptionUtilities;


//
//   Implementation of class ModelDescription.
//

ModelDescriptionAIT::ModelDescriptionAIT() {
}


void ModelDescriptionAIT::read( const string& xmlDescriptionFilePath ) {
	/// \FIXME Before parsing, it should be checked whether the file exists!

	try {
		using namespace boost::property_tree::xml_parser;
		read_xml( xmlDescriptionFilePath, data_, trim_whitespace | no_comments );
	} catch( ... ) {
		isValid_ = false;
		return;
	}

	try {
		/// Sanity check.
		isValid_ = hasChild( data_, "fmiModelDescription" );
	} catch ( ... ) {
		isValid_ = false;
		return;
	}

	// get the fmi version
	const Properties& attributes = getChildAttributes( data_, "fmiModelDescription" );

	/// \TODO: check whether the fmiVersion attribute exists.

	double version = attributes.get<double>( "fmiVersion" );

	if ( version == 1.0 ){
		isCSv1_ = hasChild( data_, "fmiModelDescription.Implementation" );
		isMEv1_ = !isCSv1_;
		isCSv2_ = isMEv2_ = false;
		isValid_ = true;
	}
	else if ( version == 2.0 ){
		// isCSv2_ and isMEv2_ might be both true
		isCSv2_ = hasChild( data_, "fmiModelDescription.CoSimulation" );
		isMEv2_ = hasChild( data_, "fmiModelDescription.ModelExchange" );
		isCSv1_ = isMEv1_ = false;
		isValid_ = isMEv2_ || isCSv2_;
	}
	else{
		isValid_ = false;
	}
}

ModelDescriptionAIT::ModelDescriptionAIT( const string& modelDescriptionURL, bool& isValid )
{
	isValid = false;
	std::string xmlDescriptionFilePath = modelDescriptionURL;
	isValid_ = true; // PathFromUrl::getPathFromUrl( modelDescriptionURL, xmlDescriptionFilePath );
	if ( !isValid_ )
		return;

	try {
		using namespace boost::property_tree::xml_parser;
		read_xml( xmlDescriptionFilePath, data_, trim_whitespace | no_comments );
	} catch( ... ) {
		isValid_ = false;
		return;
	}

	try {
		/// Sanity check.
		isValid_  = hasChild( data_, "fmiModelDescription" );
	} catch ( ... ) {
		isValid_ = false;
		return;
	}

	// get the fmi version
	const Properties& attributes = getChildAttributes( data_, "fmiModelDescription" );

	/// \TODO: check whether the fmiVersion attribute exists.

	double version = attributes.get<double>( "fmiVersion" );

	if ( version == 1.0 ){
		isCSv1_ = hasChild( data_, "fmiModelDescription.Implementation" );
		isMEv1_ = !isCSv1_;
		isCSv2_ = isMEv2_ = false;
		isValid_ = true;
	}
	else if ( version == 2.0 ){
		// isCSv2_ and isMEv2_ might be both true
		isCSv2_ = hasChild( data_, "fmiModelDescription.CoSimulation" );
		isMEv2_ = hasChild( data_, "fmiModelDescription.ModelExchange" );
		isCSv1_ = isMEv1_ = false;
		isValid_ = isMEv2_ || isCSv2_;
	}
	else{
		isValid_ = false;
	}
	isValid = true;
}


// Check if XML model description file has been parsed successfully.
bool
ModelDescriptionAIT::isValid() const
{
	return isValid_;
}


// Get attributes of FMI model description (FMI version, GUID, model name, etc.).
const Properties&
ModelDescriptionAIT::getModelAttributes() const
{
	return data_.get_child( "fmiModelDescription.<xmlattr>" );
}


// Get unit definitions.
const Properties&
ModelDescriptionAIT::getUnitDefinitions() const
{
	return data_.get_child( "fmiModelDescription.UnitDefinitions" );
}


// Get type definitions.
const Properties&
ModelDescriptionAIT::getTypeDefinitions() const
{
	return data_.get_child( "fmiModelDescription.TypeDefinitions" );
}


// Get the available entries from the defaultExperiment node.
//const void
//ModelDescription::getDefaultExperiment( fmiReal& startTime, fmiReal& stopTime,
//					fmiReal& tolerance, fmiReal& stepSize) const
//{
//	// return tolerance = inf if tolerance is not available, etc.
//	startTime = stopTime = tolerance = stepSize = std::numeric_limits<double>::quiet_NaN();

//	// return here if the fmu version is 1.0 ???

//	// get the attributes since there are no other childs of DefaultExperient
//	// documented in the fmi standard
//	Properties defaultExperiment = getChildAttributes( data_, "fmiModelDescription.DefaultExperiment" );

//	// read the childattributes defined in the documentation
//	if ( hasChild( defaultExperiment, "startTime" ) )
//		startTime = defaultExperiment.get<double>( "startTime" );
//	if ( hasChild( defaultExperiment, "stopTime" ) )
//		stopTime =  defaultExperiment.get<double>( "stopTime" );
//	if ( hasChild( defaultExperiment, "tolerance" ) )
//		tolerance = defaultExperiment.get<double>( "tolerance" );
//	if ( hasChild( defaultExperiment, "stepsize" ) )
//		 stepSize = defaultExperiment.get<double>( "stepsize" );
//}


// Get vendor annotations.
const Properties&
ModelDescriptionAIT::getVendorAnnotations() const
{
	return data_.get_child( "fmiModelDescription.VendorAnnotations" );
}


// Get description of model variables.
const Properties&
ModelDescriptionAIT::getModelVariables() const
{
	return data_.get_child( "fmiModelDescription.ModelVariables" );
}


// Get information concerning implementation of co-simulation tool (FMI CS feature).
const Properties&
ModelDescriptionAIT::getImplementation() const
{
	return data_.get_child( "fmiModelDescription.Implementation" );
}


// Get the verion of the FMU (1.0 or 2.0) as integer.
const int
ModelDescriptionAIT::getVersion() const
{
	if ( isMEv1_ || isCSv1_ )
		return 1;
	else if ( isMEv2_ || isCSv2_ )
		return 2;
	else
		return 0;
}


// Check if model description has unit definitions element.
bool
ModelDescriptionAIT::hasUnitDefinitions() const
{
	return hasChild( data_, "fmiModelDescription.UnitDefinitions" );
}


// Check if model description has type definitions element.
bool
ModelDescriptionAIT::hasTypeDefinitions() const
{
	return hasChild( data_, "fmiModelDescription.TypeDefinitions" );
}


// Check if model description has default experiment element.
bool
ModelDescriptionAIT::hasDefaultExperiment() const
{
	return hasChildAttributes( data_, "fmiModelDescription.DefaultExperiment" );
}


// Check if model description has vendor annotations element.
bool
ModelDescriptionAIT::hasVendorAnnotations() const
{
	return hasChild( data_, "fmiModelDescription.VendorAnnotations" );
}


// Check if model description has model variables element.
bool
ModelDescriptionAIT::hasModelVariables() const
{
	return hasChild( data_, "fmiModelDescription.ModelVariables" );
}


// Check if a Jacobian can be computed
bool
ModelDescriptionAIT::providesJacobian() const
{
	if ( isMEv1_ || isCSv1_ )
		return false;
	// if the flag providesDirectionalDerivative exists, and is true, return true...
	const Properties& attributes = getChildAttributes( data_, "fmiModelDescription.ModelExchange" );
	if ( hasChild( attributes, "providesDirectionalDerivative" ) ){
		if ( attributes.get<string>( "providesDirectionalDerivative" ) == "true" )
			return true;
	}
	// ...otherwise return false
	return false;
}


// Check if model description has implementation element.
bool
ModelDescriptionAIT::hasImplementation() const
{
	return hasChild( data_, "fmiModelDescription.Implementation" );
}


// Get model identifier from description.
string
ModelDescriptionAIT::getModelIdentifier() const
{
	if ( isMEv1_ || isCSv1_ ){
		const Properties& attributes = getChildAttributes( data_, "fmiModelDescription" );
		return attributes.get<string>( "modelIdentifier" );
	}
	else if ( isMEv2_ ){
		const Properties& attributes = getChildAttributes( data_, "fmiModelDescription.ModelExchange" );
		return attributes.get<string>( "modelIdentifier" );
	} else{
		// \TODO: test
		const Properties& attributes = getChildAttributes( data_, "fmiModelDescription.CoSimulation" );
		return attributes.get<string>( "modelIdentifier" );
	}
}


// Get GUID from description.
string
ModelDescriptionAIT::getGUID() const
{
	const Properties& attributes = getChildAttributes( data_, "fmiModelDescription");
	return attributes.get<string>( "guid" );
}


// Get MIME type from description (FMI CS feature).
string
ModelDescriptionAIT::getMIMEType() const
{
	string type;

	if ( false == isCSv1_ ) return type;

	if ( hasChild( data_, "fmiModelDescription.Implementation.CoSimulation_Tool.Model" ) )
	{
		const Properties& attributes =
			getChildAttributes( data_, "fmiModelDescription.Implementation.CoSimulation_Tool.Model" );

		type = attributes.get<string>( "type" );
	}

	return type;
}


// Get entry point from description (FMI CS feature).
string
ModelDescriptionAIT::getEntryPoint() const
{
	string type;

	if ( false == isCSv1_ ) return type;

	if ( hasChild( data_, "fmiModelDescription.Implementation.CoSimulation_Tool.Model" ) )
	{
		const Properties& attributes =
			getChildAttributes( data_, "fmiModelDescription.Implementation.CoSimulation_Tool.Model" );

		type = attributes.get<string>( "entryPoint" );
	}

	return type;
}


// Get number of continuous states from description.
int
ModelDescriptionAIT::getNumberOfContinuousStates() const
{
	if ( isMEv1_ || isCSv1_ ){
		const Properties& attributes = getChildAttributes( data_, "fmiModelDescription");
		return attributes.get<int>( "numberOfContinuousStates" );
	}

	// in the 2.0 specification, the entry number OfContinuousStattes has been removed because of redundancy
	// to get the number of continuous states, count the number of derivatives
	if ( !hasChild( data_, "fmiModelDescription.ModelStructure.Derivatives" ) )
		return 0;

	const Properties& derivatives = data_.get_child("fmiModelDescription.ModelStructure.Derivatives");
	int cnt = 0;
	BOOST_FOREACH( const Properties::value_type &v, derivatives ){
		cnt++;
		continue;
	}
	return cnt;
}


// Get number of event indicators from description.
int
ModelDescriptionAIT::getNumberOfEventIndicators() const
{
	const Properties& attributes = getChildAttributes( data_, "fmiModelDescription");
	return attributes.get<int>( "numberOfEventIndicators" );
}


// Get number of variables of type fmiReal, fmiInteger, fmiBoolean and fmiString.
void
ModelDescriptionAIT::getNumberOfVariables( size_t& nReal, size_t& nInt,
					size_t& nBool, size_t& nString ) const
{
	// Define XML tags to search for.
	const string xmlRealTag( "Real" );
	const string xmlIntTag( "Integer" );
	const string xmlBoolTag( "Boolean" );
	const string xmlStringTag( "String" );

	// Reset counters.
	nReal = 0;
	nInt = 0;
	nBool = 0;
	nString = 0;

	const Properties& modelVariables = getModelVariables();

	BOOST_FOREACH( const Properties::value_type &v, modelVariables )
	{
		if ( v.second.find( xmlRealTag ) != v.second.not_found() ) { ++nReal; continue; }
		else if ( v.second.find( xmlIntTag ) != v.second.not_found() ) { ++nInt; continue; }
		else if ( v.second.find( xmlBoolTag ) != v.second.not_found() ) { ++nBool; continue; }
		else if ( v.second.find( xmlStringTag ) != v.second.not_found() ) { ++nString; continue; }
		else {
			string error( "[ModelDescription::getNumberOfVariables] unknown type: " );
			error += v.second.back().first;
			throw runtime_error( error );
		}
	}
}


// Get a vector of value references for all derivatives
//void
//ModelDescription::getStatesAndDerivativesReferences( fmiValueReference* state_ref, fmiValueReference* der_ref ) const
//{
//	int i = 0;
//	const Properties& derivatives = data_.get_child( "fmiModelDescription.ModelStructure.Derivatives" );

//	// get the index attribute from all derivatives
//	BOOST_FOREACH( const Properties::value_type &v, derivatives )
//		{
//			der_ref[i]= v.second.get<unsigned int>( "<xmlattr>.index" );
//			i++;
//			continue;
//		}
//	const Properties& modelVariables = getModelVariables();
//	i = 0;
//	unsigned int j = 1;

//	// use the indices to get all value references. suppose the index vector is monotone
//	BOOST_FOREACH( const Properties::value_type & v, modelVariables )
//		{
//			if ( der_ref[i] == j ){
//				der_ref[i] = v.second.get<unsigned int>( "<xmlattr>.valueReference" );
//				state_ref[i] = v.second.get<unsigned int>( "Real.<xmlattr>.derivative" );
//				i++;
//				}
//			j++;
//			continue;
//		}
//	i = 0;
//	j = 1;
//	BOOST_FOREACH( const Properties::value_type & v, modelVariables )
//		{
//			if ( state_ref[i] == j ){
//				state_ref[i] = v.second.get<unsigned int>( "<xmlattr>.valueReference" );
//				i++;
//			}
//			j++;
//		}
//}


//
//  Implementation of functionalities from namespace ModelDescriptionUtilities.
//


// Check for attributes.
bool
ModelDescriptionUtilities::hasAttributes( const Properties& p )
{
	boost::optional<const Properties&> c = p.get_child_optional( "<xmlattr>" );
	return ( !c ) ? false : true;
}


// Check for attributes.
bool
ModelDescriptionUtilities::hasAttributes( const Properties::iterator& it )
{
	boost::optional<Properties&> c = it->second.get_child_optional( "<xmlattr>" );
	return ( !c ) ? false : true;
}


// Check for attributes.
bool
ModelDescriptionUtilities::hasAttributes( const Properties::const_iterator& it )
{
	boost::optional<const Properties&> c = it->second.get_child_optional( "<xmlattr>" );
	return ( !c ) ? false : true;
}


// Get attributes of current node.
const Properties&
ModelDescriptionUtilities::getAttributes( const Properties& p )
{
	return p.get_child( "<xmlattr>" );
}


// Get attributes of current node.
const Properties&
ModelDescriptionUtilities::getAttributes( const Properties::iterator& it )
{
	return it->second.get_child( "<xmlattr>" );
}


// Get attributes of current node.
const Properties&
ModelDescriptionUtilities::getAttributes( const Properties::const_iterator& it )
{
	return it->second.get_child( "<xmlattr>" );
}


// Check child node for attributes.
bool
ModelDescriptionUtilities::hasChildAttributes( const Properties& p,
						   const string& childName )
{
	if ( hasChild( p, childName ) ) {
		boost::optional<const Properties&> c =
			p.get_child( childName ).get_child_optional( "<xmlattr>" );
		return ( !c ) ? false : true;
	}
	return false;
}


// Check child node for attributes.
bool
ModelDescriptionUtilities::hasChildAttributes( const Properties::iterator& it,
						   const string& childName )
{
	if ( hasChild( it, childName ) ) {
		boost::optional<Properties&> c =
			it->second.get_child( childName ).get_child_optional( "<xmlattr>" );
		return ( !c ) ? false : true;
	}
	return false;
}


// Check child node for attributes.
bool
ModelDescriptionUtilities::hasChildAttributes( const Properties::const_iterator& it,
						   const string& childName )
{
	if ( hasChild( it, childName ) ) {
		boost::optional<const Properties&> c =
			it->second.get_child( childName ).get_child_optional( "<xmlattr>" );
		return ( !c ) ? false : true;
	}
	return false;
}


// Get attributes of child node.
const Properties&
ModelDescriptionUtilities::getChildAttributes( const Properties& p,
						   const string& childName )
{
	return p.get_child( childName ).get_child( "<xmlattr>" );
}


// Get attributes of child node.
const Properties&
ModelDescriptionUtilities::getChildAttributes( const Properties::iterator& it,
						   const string& childName )
{
	return it->second.get_child( childName ).get_child( "<xmlattr>" );
}


// Get attributes of child node.
const Properties&
ModelDescriptionUtilities::getChildAttributes( const Properties::const_iterator& it,
						   const string& childName )
{
	return it->second.get_child( childName ).get_child( "<xmlattr>" );
}


// Check for child node.
bool
ModelDescriptionUtilities::hasChild( const Properties& p,
					 const string& childName )
{
	boost::optional<const Properties&> c = p.get_child_optional( childName );
	return ( !c ) ? false : true;
}


// Check for child node.
bool
ModelDescriptionUtilities::hasChild( const Properties::iterator& it,
					 const string& childName )
{
	boost::optional<Properties&> c = it->second.get_child_optional( childName );
	return ( !c ) ? false : true;
}


// Check for child node.
bool
ModelDescriptionUtilities::hasChild( const Properties::const_iterator& it,
					 const string& childName )
{
	boost::optional<const Properties&> c = it->second.get_child_optional( childName );
	return ( !c ) ? false : true;
}
