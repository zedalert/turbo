#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT
    
public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();

    void set(QByteArray u, QByteArray p, bool bonusFirst, bool dontAsk, bool focus = false);
    QByteArray  user, pass;
    bool        bonusFirst, dontAsk;

private:
    Ui::Settings    *ui;
    bool            userOk, passOk;

private slots:
    void userChanged(QString text);
    void passChanged(QString text);
    void okClicked();
};

#endif // SETTINGS_H
