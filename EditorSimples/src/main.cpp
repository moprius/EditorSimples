#include "include/mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    QApplication::setApplicationName(PROGRAM_NAME);
    QApplication::setApplicationVersion(PROGRAM_VERSION);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("target", QApplication::translate("target", "Arquivo para abrir"));
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    QString target;
    if (args.length() > 0) {
        target = args.at(0);
    }

    MainWindow w;
    w.show();
    if (!target.isNull()) {
        w.do_open(target);
    }

    return app.exec();
}
