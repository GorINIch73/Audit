#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionBank_triggered();

    void on_actionOpenBase_triggered();

    void on_actionCloseBase_triggered();

    void on_actionImportPP_triggered();

private:
    Ui::MainWindow *ui;

    QString databaseName;
    QSqlDatabase database;

    bool OpenBase();
};
#endif // MAINWINDOW_H
