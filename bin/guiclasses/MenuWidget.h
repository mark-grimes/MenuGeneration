#ifndef guiclasses_MenuWidget_h
#define guiclasses_MenuWidget_h

/*                                     --------------------------
 *                                     ----- **IMPORTANT** ------
 *                                     --------------------------
 *
 * If you modify this file by adding more slots you will need to regenerate the MenuWidget.moc file.
 * This can be done with the commands:
 *
 *     cd $CMSSW_BASE/src/L1Trigger/MenuGeneration/bin/guiclasses
 *     moc MenuWidget.h > MenuWidget.moc
 *
 * "moc" is part of QT. If you have QT installed (which is included with CMSSW so you should have) then
 * moc will be available on the command line.
 *
 */

#include <QtGui>
#include <vector>
#include <memory>

#include "l1menu/TriggerMenu.h"
// classes from the same directory, i.e. guiclasses
#include "TriggerRowEntry.h"
//
// Forward declarations
//
namespace l1menu
{
	class ReducedSample;
	class ISample;
}

namespace l1menu
{
	namespace guiclasses
	{
		/** @brief Widget to display a menu, with editable text boxes for thresholds and checkboxes to enable each trigger.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 23/Jun/2014
		 */
		class MenuWidget : public QWidget
		{
			Q_OBJECT
		public:
			MenuWidget();
			void addTrigger( const l1menu::ITrigger& trigger );
			l1menu::TriggerMenu currentMenu();
			void calculateRates( const l1menu::ISample& sample );

			/** @brief Returns the widget containing text about the total rate.
			 *
			 * The pTotalRateWidget_ is not actually displayed by this widget, so the parent widget can
			 * handle the layout by calling this method and adding the widget to it's own layout.
			 * This widget will take care of filling the text though.
			 */
			QWidget* getRateLabel();
		private:
			std::unique_ptr<QGridLayout> pLayout_;
			std::vector< std::unique_ptr<l1menu::guiclasses::TriggerRowEntry> > tableRow_;
			std::vector< std::unique_ptr<QCheckBox> > checkboxes_; ///< will have one entry for each tableRow_
			std::unique_ptr<QLabel> pTotalRateWidget_;
		};

	} // end of namespace guiclasses

} // end of namespace l1menu

#endif // of ifndef guiclasses_MenuWidget_h
