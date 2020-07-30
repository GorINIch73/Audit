#include "formarticles.h"
#include "ui_formarticles.h"

FormArticles::FormArticles(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormArticles)
{
    ui->setupUi(this);
}

FormArticles::~FormArticles()
{
    delete ui;
}
