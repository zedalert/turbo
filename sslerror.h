#ifndef SSLERROR_H
#define SSLERROR_H

#include <QDialog>
#include <QtNetwork>

namespace Ui {
class SSLError;
}

class SSLError : public QDialog
{
    Q_OBJECT
    
public:
    explicit SSLError(QWidget *parent = 0);
    ~SSLError();
    
private:
    Ui::SSLError    *ui;

public:
    void set(QList<QSslError> errors);
};

#endif // SSLERROR_H
