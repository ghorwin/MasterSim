/* --------------------------------------------------------------
 * Copyright (c) 2013, AIT Austrian Institute of Technology GmbH.
 * All rights reserved. See file FMIPP_LICENSE for details.
 * --------------------------------------------------------------*/

#ifndef _FMIPP_ModelDescription_H
#define _FMIPP_ModelDescription_H


/**
 *  \file ModelDescription.h
 *  \class ModelDescription ModelDescription.h
 *  Parses and stores FMI model descriptions.
 *
 *  The FMI standard defines an XML model description scheme. This class
 *  provides the utilities to parse and store this information dynamically
 *  during run-time. It uses Boost's PropertyTree internally.
 */

#include <string>
#include <boost/property_tree/ptree.hpp>

//#include "common/FMIPPConfig.h"
#include "fmi/fmi2TypesPlatform.h"

#define __FMI_DLL

class __FMI_DLL ModelDescriptionAIT
{

public:

	typedef boost::property_tree::ptree Properties;

public:
	/// Constructor
	ModelDescriptionAIT();

	void read(const std::string& xmlDescriptionFilePath );

	// Second constructor using URL
	ModelDescriptionAIT( const std::string& modelDescriptionURL, bool& isValid );

	/// Check if XML model description file has been parsed successfully.
	bool isValid() const;

	/// Get attributes of FMI model description (FMI version, GUID, model name, etc.).
	const Properties& getModelAttributes() const;

	/// Get unit definitions.
	const Properties& getUnitDefinitions() const;

	/// Get type definitions.
	const Properties& getTypeDefinitions() const;

	/// Get description of model variables.
//	const void getDefaultExperiment( fmiReal& startTime, fmiReal& stopTime,
//					 fmiReal& tolerance, fmiReal& stepSize ) const;

	/// Get vendor annotations.
	const Properties& getVendorAnnotations() const;

	/// Get description of model variables.
	const Properties& getModelVariables() const;

	/// Get information concerning implementation of co-simulation tool (FMI CS feature).
	const Properties& getImplementation() const;

	/// Get the version of the FMU (1.0 or 2.0) as integer.
	const int getVersion() const;


	/// Check if model description has unit definitions element.
	bool hasUnitDefinitions() const;

	/// Check if model description has type definitions element.
	bool hasTypeDefinitions() const;

	/// Check if model description has default experiment element.
	bool hasDefaultExperiment() const;

	/// Check if model description has vendor annotations element.
	bool hasVendorAnnotations() const;

	/// Check if model description has model variables element.
	bool hasModelVariables() const;

	/// Check if model description has implementation element.
	bool hasImplementation() const;

	/// Check if a Jacobian can be computed
	bool providesJacobian() const;


	/// Get model identifier from description.
	std::string getModelIdentifier() const;

	/// Get GUID from description.
	std::string getGUID() const;

	/// Get MIME type from description (FMI CS feature).
	std::string getMIMEType() const;

	/// Get entry point from description (FMI CS feature).
	std::string getEntryPoint() const;

	/// Get number of continuous states from description.
	int getNumberOfContinuousStates() const;

	/// Get number of event indicators from description.
	int getNumberOfEventIndicators() const;

	/// Get number of variables of type fmiReal, fmiInteger, fmiBoolean and fmiString.
	void getNumberOfVariables( size_t& nReal, size_t& nInt,
				   size_t& nBool, size_t& nString ) const;

	/// Get the value references for all states and derivatives
//	void getStatesAndDerivativesReferences( fmiValueReference* state_ref, fmiValueReference* der_ref ) const;


protected:

	Properties data_; ///< This data structure (a Boost PropertyTree) holds the parsed model description.

	bool isValid_; ///< True if the XML model description file has been parsed successfully.

	bool isMEv1_; ///< Flag to indicated whether this FMU is ME (v1.0).
	bool isCSv1_; ///< Flag to indicated whether this FMU is CS (v1.0).

	bool isMEv2_; ///< Flag to indicated whether this FMU is ME (v2.0).
	bool isCSv2_; ///< Flag to indicated whether this FMU is CS (v2.0).
};




/// Namespace containing helper functions for dealing with class ModelDescription.
namespace ModelDescriptionUtilities
{
	typedef ModelDescriptionAIT::Properties Properties;

	__FMI_DLL bool hasAttributes( const Properties& p ); ///< Check for attributes.
	__FMI_DLL bool hasAttributes( const Properties::iterator& it ); ///< Check for attributes.
	__FMI_DLL bool hasAttributes( const Properties::const_iterator& it ); ///< Check for attributes.

	__FMI_DLL const Properties& getAttributes( const Properties& p ); ///< Get attributes of current node.
	__FMI_DLL const Properties& getAttributes( const Properties::iterator& it ); ///< Get attributes of current node.
	__FMI_DLL const Properties& getAttributes( const Properties::const_iterator& it ); ///< Get attributes of current node.

	__FMI_DLL bool hasChildAttributes( const Properties& p,
					   const std::string& childName ); ///< Check child node for attributes.
	__FMI_DLL bool hasChildAttributes( const Properties::iterator& it,
					   const std::string& childName ); ///< Check child node for attributes.
	__FMI_DLL bool hasChildAttributes( const Properties::const_iterator& it,
					   const std::string& childName ); ///< Check child node for attributes.

	__FMI_DLL const Properties& getChildAttributes( const Properties& p,
							const std::string& childName ); ///< Get attributes of child node.
	__FMI_DLL const Properties& getChildAttributes( const Properties::iterator& it,
							const std::string& childName ); ///< Get attributes of child node.
	__FMI_DLL const Properties& getChildAttributes( const Properties::const_iterator& it,
							const std::string& childName ); ///< Get attributes of child node.

	__FMI_DLL bool hasChild( const Properties& p,
				 const std::string& childName ); ///< Check for child node.
	__FMI_DLL bool hasChild( const Properties::iterator& it,
				 const std::string& childName ); ///< Check for child node.
	__FMI_DLL bool hasChild( const Properties::const_iterator& it,
				 const std::string& childName ); ///< Check for child node.
}


#endif
