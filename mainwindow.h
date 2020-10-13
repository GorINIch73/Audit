#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>

#define ORGANIZATION_NAME "gorinich-co"
#define ORGANIZATION_DOMAIN ""
#define APPLICATION_NAME "audit"
#define SETTINGS_BASE_FILE1 "File1"
#define SETTINGS_BASE_FILE2 "File2"
#define SETTINGS_BASE_FILE3 "File3"

#define FILE_EXT ".dbad"

const QString BUILDV =  QStringLiteral(__DATE__ " " __TIME__);


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


signals:

    void signalFromQuery(QString sq); // сигнал для запроса

private slots:
    void on_actionBank_triggered();

    void on_actionOpenBase_triggered();

    void on_actionCloseBase_triggered();

    void on_actionImportPP_triggered();

    void on_actionOptions_triggered();

    void on_actionContracts_triggered();

    void on_actionArticles_triggered();

    void on_actionCounterparties_triggered();

    void on_actionSaveAs_triggered();

    void on_actionNewBase_triggered();

    void on_actionExit_triggered();

    void on_actionRep_bank_triggered();

    void on_actionAbout_triggered();

    void on_actionRepContracts_triggered();

    void on_actionRepContracsShort_triggered();

    void on_actionRepContractsIsNote_triggered();

    void on_actionQuery_triggered();

    void slot_goQuery(QString sq); // запуск запроса

    void slot_OpenBase(QString fb); // открвть базу по имени


    void on_actionFile01_triggered();

    void on_actionFile02_triggered();

    void on_actionFile03_triggered();

private:
    Ui::MainWindow *ui;

    QString databaseName;
    QSqlDatabase database;

    bool OpenBase();
    void SetHistory();

};
#endif // MAINWINDOW_H
