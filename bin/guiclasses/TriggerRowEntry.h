#ifndef guiclasses_TriggerRowEntry_h
#define guiclasses_TriggerRowEntry_h

#include <QtGui>
#include <vector>
#include <memory>

// Can't use a forward declaration for this because the unique_ptr delete needs to know
// the size.
#include "l1menu/ITrigger.h"
//
// Forward declarations
//
namespace l1menu
{
	class ITriggerDescription;
	class ITriggerRate;
}

namespace l1menu
{
	namespace guiclasses
	{
		/** @brief Class to hold the widgets for a trigger and convert between online and offline thresholds
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 20/Jun/2014
		 */
		class TriggerRowEntry : public QObject
		{
			Q_OBJECT
		public:
			TriggerRowEntry( const l1menu::ITrigger& trigger );

			size_t numberOfFreeThresholds() const;
			size_t numberOfThresholds() const;
			float onlineThreshold( size_t thresholdIndex ) const; // Returns the online threshold
			float offlineThreshold( size_t thresholdIndex ) const; // Returns the offline threshold

			void setFromOnlineThreshold( size_t thresholdIndex, float threshold );
			void setFromOfflineThreshold( size_t thresholdIndex, float threshold );

			const l1menu::ITriggerDescription& getTrigger() const;

			QWidget* nameWidget();
			std::vector<QWidget*> thresholdWidgets();
			QWidget* rateLabelWidget();
			QWidget* rateErrorLabelWidget();
			QWidget* pureRateLabelWidget();
			QWidget* pureRateErrorLabelWidget();

			void enable();
			void disable();
			void setRate( const l1menu::ITriggerRate* pRate );
		private slots:
			void setEnabled( int state );
			void thresholdWidgetChanged( double value );
		private:
			/// Online to offline scaling for each free threshold. "first" is the slope, "second" is the offset.
			std::vector< std::pair<double,double> > thresholdOnlineToOfflineScaling_;
			std::unique_ptr<l1menu::ITrigger> pTrigger_;

			std::unique_ptr<QLabel> pNameLabel_;
			std::vector< std::unique_ptr<QDoubleSpinBox> > freeThresholds_;
			std::vector< std::pair<std::unique_ptr<QDoubleSpinBox>,double> > scaledThresholds_;
			std::unique_ptr<QLabel> pRateLabel_;
			std::unique_ptr<QLabel> pRateErrorLabel_;
			std::unique_ptr<QLabel> pPureRateLabel_;
			std::unique_ptr<QLabel> pPureRateErrorLabel_;

			bool widgetsShowOnlineThresholds_;
			void setTriggerFromWidgets();
			void setWidgetsFromTrigger( bool onlySetScaledThresholds=false );
		};

	} // end of namespace guiclasses

} // end of namespace l1menu

#endif // of ifndef guiclasses_TriggerRowEntry_h
