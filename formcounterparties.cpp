#include "formcounterparties.h"
#include "ui_formcounterparties.h"

FormCounterparties::FormCounterparties(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormCounterparties)
{
    ui->setupUi(this);
}

FormCounterparties::~FormCounterparties()
{
    delete ui;
}
