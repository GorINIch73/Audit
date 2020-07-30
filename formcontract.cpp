#include "formcontract.h"
#include "ui_formcontract.h"

FormContract::FormContract(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormContract)
{
    ui->setupUi(this);
}

FormContract::~FormContract()
{
    delete ui;
}
