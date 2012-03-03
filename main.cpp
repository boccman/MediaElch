#include <QtGui/QApplication>
#include <QTextCodec>
#include <QTranslator>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator editTranslator;
    QString filename;
    filename = QString("XBMM_%1"). arg(QLocale::system().name());
    editTranslator.load(":/i18n/" + filename);
    a.installTranslator(&editTranslator);

    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QCoreApplication::setOrganizationName("kVibes");
    QCoreApplication::setApplicationName("XBMM");
    QCoreApplication::setApplicationVersion("0.1");

    MainWindow w;
    w.show();
    
    return a.exec();
}