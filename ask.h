#ifndef ASK_H
#define ASK_H

#include <QDialog>

namespace Ui {
class Ask;
}

class Ask : public QDialog
{
    Q_OBJECT
    
public:
    explicit Ask(QWidget *parent = 0);
    ~Ask();

    void set(int hour, int price, bool bonus);

signals:
    void acceptedAlways();
    
private:
    Ui::Ask *ui;
    QString wordEnd(int num, QString one, QString more = "", QString none = "");
};

#endif // ASK_H
