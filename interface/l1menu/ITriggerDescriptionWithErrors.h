#ifndef l1menu_ITriggerDescriptionWithErrors_h
#define l1menu_ITriggerDescriptionWithErrors_h

#include <string>

#include "l1menu/ITriggerDescription.h"


namespace l1menu
{
	/** @brief Extension of ITriggerDescription that includes errors
	 *
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 06/Mar/2014
	 */
	class ITriggerDescriptionWithErrors : public l1menu::ITriggerDescription
	{
	public:
		virtual ~ITriggerDescriptionWithErrors() {}
		//
		// These are the methods inherited from ITriggerDescription.
		//
		virtual const std::string name() const = 0;
		virtual unsigned int version() const = 0;
		virtual const std::vector<std::string> parameterNames() const = 0;
		virtual const float& parameter( const std::string& parameterName ) const = 0;

		//
		// These are the additional methods for handling errors
		//
		virtual bool parameterErrorsAreAvailable( const std::string& parameterName ) const = 0;
		virtual const float& parameterErrorLow( const std::string& parameterName ) const = 0;
		virtual const float& parameterErrorHigh( const std::string& parameterName ) const = 0;
	};

} // end of namespace l1menu

#endif
