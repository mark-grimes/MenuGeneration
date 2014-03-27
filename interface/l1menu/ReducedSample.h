#ifndef l1menu_ReducedSample_h
#define l1menu_ReducedSample_h

#include <string>
#include <memory>
#include <map>

#include "l1menu/ReducedEvent.h"
#include "l1menu/ISample.h"

// Forward declarations
namespace l1menu
{
	class FullSample;
	class TriggerMenu;
	class ITrigger;
}


namespace l1menu
{
	/** @brief A simplified sample that has been reduced to just storing the thresholds required for each trigger to pass.
	 *
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 28/May/2013
	 */
	class ReducedSample : public l1menu::ISample
	{
	public:
		/** @brief Load from a file in protobuf format. */
		ReducedSample( const std::string& filename );
		ReducedSample( const l1menu::FullSample& originalSample, const l1menu::TriggerMenu& triggerMenu );
		ReducedSample( const l1menu::TriggerMenu& triggerMenu );
		virtual ~ReducedSample();

		void addSample( const l1menu::FullSample& originalSample );

		/** @brief Save to a file in protobuf format (protobuf in src/protobuf/l1menu.proto). */
		void saveToFile( const std::string& filename ) const;

		const l1menu::TriggerMenu& getTriggerMenu() const;
		bool containsTrigger( const l1menu::ITrigger& trigger, bool allowOlderVersion=false ) const;
		const std::map<std::string,ReducedEvent::ParameterID> getTriggerParameterIdentifiers( const l1menu::ITrigger& trigger, bool allowOlderVersion=false ) const;

		//
		// Implementations required for the ISample interface
		//
		virtual size_t numberOfEvents() const;
		virtual const l1menu::IEvent& getEvent( size_t eventNumber ) const;
		virtual std::unique_ptr<l1menu::ICachedTrigger> createCachedTrigger( const l1menu::ITrigger& trigger ) const;
		virtual float eventRate() const;
		virtual void setEventRate( float rate );
		virtual float sumOfWeights() const;
		virtual std::shared_ptr<const l1menu::IMenuRate> rate( const l1menu::TriggerMenu& menu ) const;
		virtual std::shared_ptr<const l1menu::IMenuRate> rate( const l1menu::TriggerMenu& menu, const l1menu::MenuRatePlots& ratePlots ) const;

	private:
		std::unique_ptr<class ReducedSamplePrivateMembers> pImple_;
	}; // end of class ReducedSample

} // end of namespace l1menu

#endif
