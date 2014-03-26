#ifndef l1menu_implementation_TriggerDescriptionWithErrorsFromXML_h
#define l1menu_implementation_TriggerDescriptionWithErrorsFromXML_h

#include "l1menu/ITriggerDescriptionWithErrors.h"

#include <map>

//
// Forward declarations
//
namespace l1menu
{
	namespace tools
	{
		class XMLElement;
	}
}


namespace l1menu
{
	namespace implementation
	{
		/** @brief Implementation of the ITriggerDescriptionWithErrors that can be created from an XMLElement.
		 *
		 * No check is made to make sure the description is valid, e.g. that a trigger with the given name
		 * and version actually exists or that it has the specified parameters.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 06/Mar/2014
		 */
		class TriggerDescriptionWithErrorsFromXML : public l1menu::ITriggerDescriptionWithErrors
		{
		public:
			TriggerDescriptionWithErrorsFromXML( const l1menu::tools::XMLElement& xmlDescription );
			TriggerDescriptionWithErrorsFromXML( const l1menu::ITriggerDescriptionWithErrors& otherDescription );
			TriggerDescriptionWithErrorsFromXML( const l1menu::ITriggerDescription& otherDescription );

			//
			// Methods required by the ITriggerDescriptionWithErrors interface
			//
			virtual const std::string name() const;
			virtual unsigned int version() const;
			virtual const std::vector<std::string> parameterNames() const;
			virtual const float& parameter( const std::string& parameterName ) const;
			virtual bool parameterErrorsAreAvailable( const std::string& parameterName ) const;
			virtual const float& parameterErrorLow( const std::string& parameterName ) const;
			virtual const float& parameterErrorHigh( const std::string& parameterName ) const;

			//
			// Extra methods for this implementation
			//
			void setParameterErrors( const std::string& parameterName, float errorLow, float errorHigh );
		protected:
			std::string name_;
			unsigned int version_;
			std::map<std::string,float> parameters_;
			std::map<std::string,float> parameterErrorsHigh_;
			std::map<std::string,float> parameterErrorsLow_;
		};


	} // end of the implementation namespace
} // end of the l1menu namespace
#endif
