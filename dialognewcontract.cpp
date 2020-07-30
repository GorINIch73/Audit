#include "dialognewcontract.h"
#include "ui_dialognewcontract.h"

DialogNewContract::DialogNewContract(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewContract)
{
    ui->setupUi(this);
}

DialogNewContract::~DialogNewContract()
{
    delete ui;
}
