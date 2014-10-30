#ifndef TURBO_H
#define TURBO_H

#include <QMainWindow>
#include <QtNetwork>
#include <QFile>                    // файл настроек
#include <QMenu>                    // меню для трея
#include <QSystemTrayIcon>          // системный трей
#include <QDesktopServices>         // открытие ссылок
#include <QShortcut>                // горячие клавиши
#include <QWindowStateChangeEvent>  // сворачивание в трей
#include <QLocalServer>             // единичность экземпляра программы

#include <settings.h>       // окно настроек
#include <ask.h>            // окно подтверждения активации
#include <about.h>          // окно о программе
#include <sslerror.h>       // окно SSL ошибок
#include <hoverbutton.h>    // турбокнопки с событиями onMouse*

namespace Ui {
class Turbo;
}

class Turbo : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Turbo(QWidget *parent = 0);
    ~Turbo();
    
private:
    Ui::Turbo         *ui;

    // диалоги
    Settings                *settings;  // настройки
    Ask                     *ask;       // подтверждение активации
    About                   *about;     // о программе
    SSLError                *sslerror;  // SSL ошибки

    // массивы
    QList<HoverButton *>    uiTurboButtons;     // все турбокнопки в главном окне
    QList<QAction *>        trayTurboButtons;   // все турбокнопки в трее
    QList<QByteArray>       bon, mon;           // значения переменных для запроса активации
    QList<int>              price, hours;       // сопоставление цен и времени услуг
    QStringList             contracts;          // договоры на одном логине

    QSystemTrayIcon         *tray;
    QNetworkAccessManager   *manager;
    QDateTime               turboEndDateTime;
    QTimer                  *updateTimer, *turboEndTimer;   // автообновление данных, обратный отсчёт действия услуги
    QByteArray              user, pass, site, wtf;
    QString                 span;
    bool                    bonusFirst, dontAsk, loggedIn, turboOn, turboBuyed, firstRun, ignoreSslErr;
    int                     loginRetry, turboId, daysLeft, currentContract, fontSize;
    float                   money, bonus;

    // сворачивание/разворачивание программы в/из трей/-я
    void changeEvent(QEvent *event);

    // закрытие программы
    void closeEvent(QCloseEvent *)
    {
        qApp->quit();
    }

    // отправка запроса на активацию услуги
    void turboAsk(int i, bool asked = false);

    // загрузка списка доступных услуг до активации
    void turboActivate();

    // активация/деактивация GUI турбокнопок согласно балансу
    void turboButtons();

    // чтение файла настроек
    bool settingsLoad();

    // запись файла настроек
    void settingsSave();

    // парсинг JSON данных абонента
    void parseCabinetData(QString json);

    // деавторизация в личном кабинете (для смены логина/пароля)
    void cabinetLogout(bool internal = false);

    // парсинг истории услуги
    void parseTurboHistory(QString html);

    // парсинг списка контрактов
    void parseContractsData(QString html);

    // показ ошибок пользователю
    void errorHolder(QString errorText, bool showSettings = false);

    // показ балуна за 3 дня до окончания средств
    void daysWarn();

    // склонение окончания слов согласно числовому коэффициенту
    QString wordEnd(int num, QString one, QString more = "", QString none = "");

private slots:
    // отработка полученных в ответ HTML страниц на предмет данных и ошибок
    void requestReply(QNetworkReply *reply);

    // отображение SSL ошибок
    void receivedSSLError(QNetworkReply *reply, QList<QSslError> errors);

    // игнорирование SSL ошибок
    void ignoreSSLError();

    // показ диалогового окна настроек
    void settingsShow(bool focus = false);

    // закрытие диалогового окна настроек
    void settingsClosed();

    // показ диалогового окна "О программе"
    void aboutShow();

    // открытие личного кабинета без авторизации в браузере
    void openCabinetInBrowser();

    // авторизация в личном кабинете
    void cabinetLogin();

    // загрузка данных абонента
    void getCabinetData();

    // загрузка истории действия услуги
    void getTurboHistory();

    // выбор договора внутри контракта
    void selectContract(int contract = 0);

    // таймер срока действия услуги
    void turboTime();

    // отработка нажатия по турбокнопке
    void turboClicked()
    {
        turboAsk(QString(sender()->objectName()[6]).toInt());
    }

    // подтверждение активации услуги
    void turboAccepted()
    {
        turboAsk(turboId, true);
    }

    // подтверждение активации услуги без будущих запросов на подтверждение
    void turboAcceptedAlways()
    {
        dontAsk = true;
        settingsSave();
        ask->accept();
    }

    // курсор наведен на одну из турбокнопок
    void turboHovered();

    // курсор вне турбокнопки
    void turboOutered();

    // отработка двойного клика (только в Windows) по значку в трее
    void trayActivated(QSystemTrayIcon::ActivationReason reason)
    {
        if (reason == QSystemTrayIcon::DoubleClick)
            showNormal();
    }

    // проверка главного окна на свёрнутость
    void checkWindowState();

    // найден другой экземпляр программы
    void anotherInstance()
    {
        if (isMinimized())
            showNormal();

        activateWindow();
    }
};

#endif // TURBO_H
