#ifndef l1menu_ISample_h
#define l1menu_ISample_h

#include <memory>

//
// Forward declarations
//
namespace l1menu
{
	class IMenuRate;
	class TriggerMenu;
	class IEvent;
	class ITrigger;
	class ICachedTrigger;
	class MenuRatePlots;
}


namespace l1menu
{
	/** @brief Abstract interface for any sample file format used for L1 menu studies.
	 *
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date sometime around June 2013
	 */
	class ISample
	{
	public:
		virtual ~ISample() {}

		virtual size_t numberOfEvents() const = 0;
		virtual const l1menu::IEvent& getEvent( size_t eventNumber ) const = 0;

		virtual std::unique_ptr<l1menu::ICachedTrigger> createCachedTrigger( const l1menu::ITrigger& trigger ) const = 0;
		/** @brief The rate at which events are occurring. I.e. the trigger rate if every event passed. */
		virtual float eventRate() const = 0;
		virtual void setEventRate( float rate ) = 0;
		/** @brief The sum of every event's weights. */
		virtual float sumOfWeights() const = 0;

		virtual std::shared_ptr<const l1menu::IMenuRate> rate( const l1menu::TriggerMenu& menu ) const = 0;
		/** @brief Overload that will use the MenuRatePlots supplied to calculate errors on the thresholds. */
		virtual std::shared_ptr<const l1menu::IMenuRate> rate( const l1menu::TriggerMenu& menu, const l1menu::MenuRatePlots& ratePlots ) const = 0;
	};

} // end of namespace l1menu

#endif
