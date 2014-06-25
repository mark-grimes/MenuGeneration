#include <stdexcept>
#include <iostream>

#include <QtGui>
#include "l1menu/ReducedSample.h"
#include "guiclasses/MainWidget.h"
#include "guiclasses/AnalyseReducedSampleWidget.h"


int main( int argc, char **argv )
{
	if( argc!=2 )
	{
		std::cerr << "The only argument should be the filename of the ReducedSample" << std::endl;
		return 0;
	}

	QApplication app( argc, argv );

	l1menu::ReducedSample sample( argv[1] );

	l1menu::guiclasses::MainWidget myMainWidget( sample );
	myMainWidget.show();


	l1menu::guiclasses::AnalyseReducedSampleWidget testWidget( sample );
	testWidget.show();

	return app.exec();
}





