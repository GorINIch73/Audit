#include "mainwindow.h"

#include <QApplication>


int main(int argc, char *argv[])
{

    QLocale::setDefault(QLocale(QLocale::Russian, QLocale::RussianFederation));

    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(APPLICATION_NAME);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
