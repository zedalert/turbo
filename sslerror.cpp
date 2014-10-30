#include "sslerror.h"
#include "ui_sslerror.h"

SSLError::SSLError(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    ui(new Ui::SSLError)
{
    ui->setupUi(this);

    connect(ui->ignore, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->cancel, SIGNAL(clicked()), this, SLOT(reject()));
}

SSLError::~SSLError()
{
    delete ui;
}

void SSLError::set(QList<QSslError> errors)
{
    ui->log->clear();
    int count = errors.count();
    for (int i = 0; i < count; i++)
        ui->log->appendPlainText(errors.at(i).errorString() + (i + 1 < count ? ";" : "."));
    // прокрутка лога ошибок в начало
    ui->log->moveCursor(QTextCursor::Start);
    ui->log->ensureCursorVisible();
}
