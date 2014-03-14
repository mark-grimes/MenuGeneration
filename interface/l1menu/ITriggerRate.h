#ifndef l1menu_ITriggerRate_h
#define l1menu_ITriggerRate_h

#include <vector>
#include <string>

//
// Forward declarations
//
namespace l1menu
{
	class ITriggerDescriptionWithErrors;
	namespace tools
	{
		class XMLElement;
	}
}

namespace l1menu
{
	/** @brief Interface to get information about the trigger rate. Read only.
	 *
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 24/Jun/2013
	 */
	class ITriggerRate
	{
	public:
		virtual ~ITriggerRate() {}

		/** @brief The trigger that gives the rate, which can be queried for thresholds etcetera */
		virtual const l1menu::ITriggerDescriptionWithErrors& trigger() const = 0;

		/** @brief The fraction of events that this trigger passed, so before applying any scaling. */
		virtual float fraction() const = 0;
		virtual float fractionError() const = 0;

		/** @brief The rate, so fraction multiplied by the scaling. */
		virtual float rate() const = 0;
		virtual float rateError() const = 0;

		/** @brief The fraction of events that pass only this trigger, with no scaling. */
		virtual float pureFraction() const = 0;
		virtual float pureFractionError() const = 0;

		/** @brief The pure rate, so pureFraction multiplied by the scaling. */
		virtual float pureRate() const = 0;
		virtual float pureRateError() const = 0;

//		/** @brief Adds an XML element to the one provided that describes this object. */
//		virtual void convertToXML( l1menu::tools::XMLElement& parentElement ) const = 0;
	};

} // end of namespace l1menu

#endif
