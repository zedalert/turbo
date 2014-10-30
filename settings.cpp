#include "settings.h"
#include "ui_settings.h"

Settings::Settings(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    ui(new Ui::Settings)
{
    ui->setupUi(this);

    connect(ui->ok,     SIGNAL(clicked()),            this, SLOT(okClicked()));
    connect(ui->cancel, SIGNAL(clicked()),            this, SLOT(reject()));
    connect(ui->user,   SIGNAL(textChanged(QString)), this, SLOT(userChanged(QString)));
    connect(ui->pass,   SIGNAL(textChanged(QString)), this, SLOT(passChanged(QString)));
    connect(ui->user,   SIGNAL(returnPressed()),      this, SLOT(okClicked()));
    connect(ui->pass,   SIGNAL(returnPressed()),      this, SLOT(okClicked()));
}

void Settings::set(QByteArray u, QByteArray p, bool bonusFirst, bool dontAsk, bool focus)
{
    if (focus)
        ui->pass->setFocus();
    else
        ui->user->setFocus();

    ui->user->setText(QString(u));
    ui->pass->setText(QString(p));
    ui->bonus->setChecked(bonusFirst);
    ui->warn->setChecked(dontAsk);

    userOk = u.length() > 4;
    passOk = p.length() > 5;
    ui->ok->setEnabled(passOk && userOk);
}

void Settings::okClicked()
{
    bonusFirst = ui->bonus->isChecked();
    dontAsk = ui->warn->isChecked();
    user = ui->user->text().toUtf8();
    pass = ui->pass->text().toUtf8();
    this->accept();
}

void Settings::userChanged(QString text)
{
    userOk = text.length() > 4;
    ui->ok->setEnabled(passOk && userOk);
}

void Settings::passChanged(QString text)
{
    passOk = text.length() > 5;
    ui->ok->setEnabled(passOk && userOk);
}

Settings::~Settings()
{
    delete ui;
}
