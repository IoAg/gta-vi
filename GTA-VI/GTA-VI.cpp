
#include "../arffHelper/Arff.h"
#include "VideoWidget.h"
#include "MainWindow.h"

#include <QtGui>
#include <iostream>

using namespace std;
 
int main(int argc, char *argv[]) 
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("GTA-VI (Ground Truth Annotation-Visualization Interface)");
    QCoreApplication::setApplicationVersion("version 2.0");

    // initialize parser
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption videoFileOption(QStringList() << "vf" << "video-file",
            QCoreApplication::translate("main", "Video file."),
            QCoreApplication::translate("main", "file"));
    parser.addOption(videoFileOption);

    QCommandLineOption arffFileOption(QStringList() << "af" << "arff-file",
            QCoreApplication::translate("main", "ARFF file."),
            QCoreApplication::translate("main", "file"));
    parser.addOption(arffFileOption);

    QCommandLineOption saveFileOption(QStringList() << "sf" << "save-file",
            QCoreApplication::translate("main", "Save file."),
            QCoreApplication::translate("main", "file"));
    parser.addOption(saveFileOption);

    QCommandLineOption primaryLabelOption(QStringList() << "pl" << "primary-label",
            QCoreApplication::translate("main", "Primary hand labelling attribute name."),
            QCoreApplication::translate("main", "name"));
    parser.addOption(primaryLabelOption);

    QCommandLineOption primaryLabelValueOption(QStringList() << "plv" << "primary-label-value",
            QCoreApplication::translate("main", "(Optional) Create a nominal primary labelling attribute. Ex. \"{fix,sacc,sp}\"."),
            QCoreApplication::translate("main", "name"),
            "");
    parser.addOption(primaryLabelValueOption);

    QCommandLineOption secondaryLabelOption(QStringList() << "sl" << "secondary-label",
            QCoreApplication::translate("main", "(Optional) Secondary hand labelling attribute name."),
            QCoreApplication::translate("main", "name"));
    parser.addOption(secondaryLabelOption);

    QCommandLineOption secondaryLabelValueOption(QStringList() << "slv" << "secondary-label-value",
            QCoreApplication::translate("main", "(Optional) Create a nominal secondary labelling attribute. Ex. \"{fix,sacc,sp}\"."),
            QCoreApplication::translate("main", "name"),
            "");
    parser.addOption(secondaryLabelValueOption);

    QCommandLineOption speedFilterOption(QStringList() << "sw" << "speed-window",
            QCoreApplication::translate("main", "The duration of the window over which we filter the speed signal. The default value is 100000 us."),
            QCoreApplication::translate("main", "double value"),
            "100000");
    parser.addOption(speedFilterOption);

    QCommandLineOption fullScreenOption(QStringList() << "f" << "full-screen",
            QCoreApplication::translate("main", "Start window in full screen mode."));
    parser.addOption(fullScreenOption);

    QCommandLineOption fovFileOption(QStringList() << "fov" << "field-of-view",
            QCoreApplication::translate("main", "Convert Equirectangular video to Field Of View"));
    parser.addOption(fovFileOption);

    QCommandLineOption headFileOption(QStringList() << "head" << "head-only-motion",
            QCoreApplication::translate("main", "Display only head motion in the equirectangular video"));
    parser.addOption(headFileOption);

    // parse arguments
    parser.process(app);

    MainWindow *pMainWindow;
    SetupValues setup;


    setup.gazeType = GazeType::EYE_PLUS_HEAD;
    if (parser.isSet("field-of-view"))
        setup.gazeType = GazeType::FOV;
    if (parser.isSet("head-only-motion"))
        setup.gazeType = GazeType::HEAD;

    setup.arffFile = parser.value(arffFileOption);
    setup.saveFile = parser.value(saveFileOption);
    setup.videoFile = parser.value(videoFileOption);
    setup.primaryLabel = parser.value(primaryLabelOption);
    setup.primaryLabelValues = parser.value(primaryLabelValueOption);
    setup.secondaryLabel = parser.value(secondaryLabelOption);
    setup.secondaryLabelValues = parser.value(secondaryLabelValueOption);
    setup.windowDur = parser.value(speedFilterOption).toDouble();

    pMainWindow = new MainWindow(setup);

    if (parser.isSet(fullScreenOption))
        pMainWindow->showFullScreen();
    else
        pMainWindow->show();

    return app.exec();
}

