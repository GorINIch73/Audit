#include "formoptions.h"
#include "ui_formoptions.h"

FormOptions::FormOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormOptions)
{
    ui->setupUi(this);
}

FormOptions::~FormOptions()
{
    delete ui;
}
