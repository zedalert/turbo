#include "ask.h"
#include "ui_ask.h"

Ask::Ask(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    ui(new Ui::Ask)
{
    ui->setupUi(this);

    connect(ui->yes,     SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->no,      SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->dontask, SIGNAL(clicked()), this, SIGNAL(acceptedAlways()));
}

void Ask::set(int hour, int price, bool bonus)
{
    ui->txt->setText(
        QString("Активировать услугу «Турбокнопка» на <b>%1</b> час%2?<br/>С вашего счёта будет снято <b>%3</b> %4.")
            .arg(hour)
            .arg(wordEnd(hour, "а", "ов"))
            .arg(price)
            .arg(bonus ? "бонусных баллов" : "рублей")
        );
}

QString Ask::wordEnd(int num, QString one, QString more, QString none)
{
    if ((num > 4) & (num < 21) || num % 10 > 4 || num % 10 == 0)
        return more;
    if (num % 10 == 1)
        return none;
    return one;
}

Ask::~Ask()
{
    delete ui;
}
