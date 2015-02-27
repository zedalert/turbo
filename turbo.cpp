#include "turbo.h"
#include "ui_turbo.h"

Turbo::Turbo(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Turbo)
{
    ui->setupUi(this);

    // платформозависимые размеры шрифтов
    #ifdef Q_WS_MAC
        fontSize = 11;
    #endif
    #ifdef Q_WS_X11
        fontSize = 9;
    #endif
    #ifdef Q_WS_WIN
        fontSize = 8;
    #endif

    connect(ui->cabinet, SIGNAL(clicked()), this, SLOT(openCabinetInBrowser()));
    connect(ui->update,  SIGNAL(clicked()), this, SLOT(getCabinetData()));

    // прячем по-началу ненужную форму выбора договора
    ui->contract_2->hide();

    connect(ui->settings, SIGNAL(clicked()), this, SLOT(settingsShow()));

    about   = new About(this);
    ask     = new Ask(this);
    settings = new Settings(this);
    sslerror = new SSLError(this);

    connect(ask, SIGNAL(accepted()), this, SLOT(turboAccepted()));
    connect(ask, SIGNAL(acceptedAlways()), this, SLOT(turboAcceptedAlways()));
    connect(settings, SIGNAL(accepted()), this, SLOT(settingsClosed()));
    connect(sslerror, SIGNAL(accepted()), this, SLOT(ignoreSSLError()));

    // больший размер текста для некоторых элементов
    span = QString("<span style=\"font-size: %1pt;\">").arg(fontSize * 1.6) + "%1</span>";

    qApp->setStyleSheet(
        QString(
            "QWidget { font-family: Tahoma, FreeSans; font-size: %1pt; }"
            "HoverButton { font-size: %2pt; }"
            "#support, #log, #basement, #infostr { font-size: %3pt; }"
        )
        .arg(fontSize)
        .arg(fontSize * 1.7)
        .arg(fontSize * 1.1)
    );

    QShortcut *aboutKey    = new QShortcut(QKeySequence("F1"), this);
    QShortcut *updateKey   = new QShortcut(QKeySequence("F5"), this);
    QShortcut *minimizeKey = new QShortcut(QKeySequence("Esc"), this);
    connect(aboutKey,    SIGNAL(activated()), this, SLOT(aboutShow()));
    connect(updateKey,   SIGNAL(activated()), this, SLOT(getCabinetData()));
    connect(minimizeKey, SIGNAL(activated()), this, SLOT(showMinimized()));

    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestReply(QNetworkReply*)));
    connect(manager, SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)), this, SLOT(receivedSSLError(QNetworkReply*, QList<QSslError>)));

    updateTimer   = new QTimer();
    turboEndTimer = new QTimer();
    connect(updateTimer,   SIGNAL(timeout()), this, SLOT(getCabinetData()));
    connect(turboEndTimer, SIGNAL(timeout()), this, SLOT(turboTime()));

    bon << "LOY_TURBO1" << "LOY_TURBO3" << "LOY_TURBO6" << "LOY_TURBO24";
    mon << "134"    << "135"    << "136"    << "138";
    price << 10     << 20       << 30       << 50;
    hours << 1      << 3        << 6        << 24;

    wtf = "";
    site = "https://my.onlime.ru/";
    loginRetry = money = bonus = turboId = currentContract = daysLeft = 0;
    bonusFirst = firstRun = true;
    loggedIn = dontAsk = turboOn = turboBuyed = ignoreSslErr = false;

    uiTurboButtons << ui->turbo_0 << ui->turbo_1 << ui->turbo_2 << ui->turbo_3;

    QMenu *trayMenu = new QMenu(this);
    trayMenu->addAction("Восстановить", this, SLOT(showNormal()));
    trayMenu->addSeparator();

    for (int i = 0; i < bon.count(); i++)
    {
        connect(uiTurboButtons.at(i), SIGNAL(clicked()), this, SLOT(turboClicked()));
        connect(uiTurboButtons.at(i), SIGNAL(hovered()), this, SLOT(turboHovered()));
        connect(uiTurboButtons.at(i), SIGNAL(outered()), this, SLOT(turboOutered()));

        QIcon trayButtonIcon = QIcon(QString(":/img/%1.png").arg(i));
        QAction *trayButton = new QAction(trayButtonIcon, QString("%1 час").arg(hours[i]) + wordEnd(hours[i], "а", "ов"), this);
        trayButton->setObjectName(QString("turbo_%1").arg(i));
        trayButton->setEnabled(false);
        connect(trayButton, SIGNAL(triggered()), this, SLOT(turboClicked()));
        trayMenu->addAction(trayButton);
        trayTurboButtons << trayButton;
    }

    trayMenu->addSeparator();
    trayMenu->addAction("Личный кабинет", this, SLOT(openCabinetInBrowser()));
    trayMenu->addAction("Настройки", this, SLOT(settingsShow()));
    trayMenu->addAction("Обновить", this, SLOT(getCabinetData()));
    trayMenu->addSeparator();
    trayMenu->addAction("О программе", this, SLOT(aboutShow()));
    trayMenu->addAction("Закрыть", qApp, SLOT(quit()));

    tray = new QSystemTrayIcon(this);
    tray->setIcon(QIcon(":/img/tray.png"));
    tray->setContextMenu(trayMenu);
    tray->setToolTip("Турбокнопка");
    connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));

    // проверка на свёрнутость должна быть вызвана вне конструктора UI
    QTimer::singleShot(0, this, SLOT(checkWindowState()));

    // ожидание других экземпляров программы
    QLocalServer *localServer = new QLocalServer(this);
    connect(localServer, SIGNAL(newConnection()), this, SLOT(anotherInstance()));
    localServer->listen("TurboButton");

    if (settingsLoad())
    {
        firstRun = false;
        cabinetLogin();
    }
    else
        // внешний вызов диалога настроек для корректного отображения в Mac OS X
        QTimer::singleShot(0, this, SLOT(settingsShow()));
}

void Turbo::openCabinetInBrowser()
{
    QDesktopServices::openUrl(QUrl(site + "session/login/?login_credentials[login]=" + user + "&login_credentials[password]=" + pass));
}

bool Turbo::settingsLoad()
{
    QFile ini("./turbo.ini");
    if (ini.open(QIODevice::ReadOnly))
    {
        QByteArray sett = ini.readAll();
        if (!sett.isEmpty())
        {
            QList<QByteArray> list = sett.split(':');
            if (list.count() == 4)
            {
                user = QByteArray::fromBase64(list[0]);
                pass = QByteArray::fromBase64(list[1]);
                bonusFirst = !!list[2].toInt();
                dontAsk = !!list[3].toInt();
                return true;
            }
        }
    }
    return false;
}

void Turbo::settingsSave()
{
    QFile ini("./turbo.ini");
    if (ini.open(QIODevice::WriteOnly))
    {
        QByteArray sett(user.toBase64() + ":" + pass.toBase64());
        sett.append(QString(":%1:%2").arg(bonusFirst).arg(dontAsk));
        ini.write(sett);
    }
}

void Turbo::settingsShow(bool focus)
{
    if (settings->isHidden() && ask->isHidden() && about->isHidden())
    {
        settings->set(user, pass, bonusFirst, dontAsk, focus);
        if (firstRun)
            connect(settings, SIGNAL(rejected()), qApp, SLOT(quit()));
        else
            disconnect(settings, SIGNAL(rejected()), qApp, SLOT(quit()));
        settings->show();
    }
    else
        settings->activateWindow();
}

void Turbo::settingsClosed()
{
    if (user != settings->user || pass != settings->pass || !loggedIn)
    {
        user = settings->user;
        pass = settings->pass;
        if (loggedIn)
            cabinetLogout();
        else
            cabinetLogin();
    }

    bonusFirst = settings->bonusFirst;
    dontAsk = settings->dontAsk;
    firstRun = false;

    settingsSave();
}

void Turbo::aboutShow()
{
    if (settings->isHidden() && ask->isHidden() && about->isHidden())
        about->show();
    else
        about->activateWindow();
}

void Turbo::cabinetLogin()
{
    ui->basement->setText("Авторизация..." + (loginRetry > 1 ? QString(" (%1)").arg(loginRetry + 1) : ""));

    // URL-кодируются только логин и пароль, но не строка запроса целиком
    QByteArray data = "login_credentials[login]=" + QUrl::toPercentEncoding(user) + "&login_credentials[password]=" + QUrl::toPercentEncoding(pass);

    QNetworkRequest request(QUrl(site + "session/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    manager->post(request, data);

    loginRetry++;
}

void Turbo::cabinetLogout(bool internal)
{
    if (!internal)
        contracts.clear();

    for (int i = 0; i < bon.count(); i++)
    {
        uiTurboButtons.at(i)->setEnabled(false);
        trayTurboButtons.at(i)->setEnabled(false);
    }

    disconnect(ui->contract_2, SIGNAL(currentIndexChanged(int)), this, SLOT(contractSelect(int)));
    ui->basement->setText("Выход...");
    manager->get(QNetworkRequest(QUrl(site + "session/logout")));

    loginRetry = 0;
}

void Turbo::parseContractsData(QString html)
{
    QRegExp rx;
    rx.setPattern("<td><strong>[0-9]{7}</strong></td>\\n<td><strong>[0-9]{7}</strong></td>");

    int pos = 0;
    while ((pos = rx.indexIn(html, pos)) != -1)
    {
        pos += rx.matchedLength();
        QStringList list = rx.cap(0).remove(QRegExp("<(/)?(td)?(strong)?>")).split("\n");

        if (!contracts.contains(list[0]))
            contracts << list[0];
    }

    selectContract();
}

void Turbo::selectContract(int contract)
{
    if (loggedIn)
    {
        ui->contract_2->setEnabled(false);
        currentContract = contract;
        cabinetLogout(true);
    }
    else
    {
        ui->basement->setText("Выбор договора...");

        QNetworkRequest request(QUrl(site + "index/choosecontract/"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        manager->post(request, QByteArray::number(currentContract).prepend("contract="));
    }
}

void Turbo::getCabinetData()
{
    ui->basement->setText("Загрузка данных абонента...");
    QNetworkRequest request(QUrl(site + "json/cabinet/"));
    request.setRawHeader("X-Wtf", wtf); // Иначе получим в ответ {"error":"WTF?"} :)
    manager->get(request);
}

void Turbo::parseCabinetData(QString json)
{
    if (!updateTimer->isActive())
        updateTimer->start(300000); // 5 минут

    QStringList list = json.remove(QRegExp("[{\"}]")).split(",");
    QMap<QString, QString> map;

    for (int i = 0; i < list.count(); i++)
    {
        QStringList str = QString(list[i]).split(":");
        map.insert(str[0], str[1]);
    }

    // округление бонусных баллов и добавление нуля перед точкой в балансе для соответствия личному кабинету
    float points = map["points"].toFloat();
    float balance = map["balance"].toFloat();

    ui->account->setText("Лицевой счет №: <b>" + map["account"] + "</b>");
    ui->balance->setText("Баланс: " + span.arg(QString::number(balance)) + " руб.");
    ui->lock->setText("Количество дней до блокировки: " + span.arg(map["lock"]));
    ui->points->setText("Бонусный счет: " + span.arg(QString::number(points)) + " баллов");
    ui->tier->setText("Статус: <b>" + map["tier"] + "</b>");

    if (contracts.count() == 0)
    {
        ui->contract_1->setText("");
        ui->contract_2->clear();
        ui->contract_2->hide();
        ui->contract->setText("Договор №: <b>" + map["contract"] + "</b>");
    }
    else
    {
        ui->contract->setText("");
        ui->contract_1->setText("Договор №: ");

        if (ui->contract_2->count() == 0)
            for (int i = 0; i < contracts.count(); i++)
                ui->contract_2->addItem(contracts.at(i));

        ui->contract_2->show();
        ui->contract_2->setEnabled(true);
        connect(ui->contract_2, SIGNAL(currentIndexChanged(int)), this, SLOT(selectContract(int)));
    }

    bonus = points;
    money = balance;
    daysLeft = map["lock"].toInt();

    if (daysLeft < 4)
        daysWarn();

    if (!turboOn)
        getTurboHistory();

    tray->setToolTip("Баланс: " + QString::number(money) + " руб.\nБонусы: " + QString::number(bonus) +
                     "\nОсталось: " + map["lock"] + wordEnd(daysLeft, " дня", " дней", " день"));
}

void Turbo::getTurboHistory()
{
    ui->basement->setText(turboBuyed ? "Ожидание подключения услуги..." : "Загрузка истории турбокнопки...");
    manager->get(QNetworkRequest(QUrl(site + "turbo/history/")));
}

void Turbo::parseTurboHistory(QString html)
{
    QRegExp rx;
    rx.setPattern("[0-9:.\\s]{16}</td><td>[0-9:.\\s]{16}</td><td>Подключено"); // до этого значится статус "В обработке"

    if (rx.indexIn(html) != -1)
    {
        QStringList list = rx.cap(0).split("</td><td>");
        QDateTime now = QDateTime::currentDateTime();
        turboEndDateTime = QDateTime::fromString(list[1], "dd.MM.yyyy HH:mm");

        if (turboEndDateTime > now && !turboEndTimer->isActive())
        {
            turboOn = true;
            turboEndTimer->start(1000);

            turboTime();

            if (turboBuyed)
                getCabinetData();
        }

    }

    if (!turboOn && turboBuyed)
        QTimer::singleShot(1000, this, SLOT(getTurboHistory()));

    if (!turboBuyed)
        ui->basement->setText("");

    turboButtons();
}

void Turbo::requestReply(QNetworkReply *reply)
{
    QString authError = "Ошибка авторизации: ";
    QString connError = "Ошибка подключения: ";
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError)
    {
        QString replyData = QString::fromUtf8(reply->readAll().data());
        int v = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if (v >= 200 && v < 300)
        {
            if (replyData.contains("message error"))
            {
                loggedIn = false;

                if (replyData.contains("блокирован"))
                    errorHolder(authError + "пользователь заблокирован");

                else if (replyData.contains("Пользователь"))
                    errorHolder(authError + "пользователь не найден", true);

                // всего 3 попытки входа, после этого пользователь блокируется
                else if (replyData.contains("неверный"))
                    errorHolder(authError + "неверный пароль" + (loginRetry > 1 ? "\nВнимание! Осталась одна попытка входа" : ""), true);

                else if (replyData.contains("Подождите"))
                {
                    errorHolder(authError + "превышена частота запросов\nПовторная авторизация через 1 минуту");
                    QTimer::singleShot(60000, this, SLOT(cabinetLogin()));
                }

                else
                    errorHolder(authError + "неопознанная ошибка");
            }

            else if (replyData.contains("-100%"))
                turboActivate();

            else if (replyData.contains("успешно"))
                getTurboHistory();

            // иногда JSON данные не сразу отдаются
            else if (replyData.contains("wait"))
                QTimer::singleShot(1000, this, SLOT(getCabinetData()));

            else if (replyData.contains("tier"))
                parseCabinetData(replyData);

            else if (replyData.contains("Дата начала"))
                parseTurboHistory(replyData);

            else if (replyData.contains("Уважаемые"))
                errorHolder(connError + "личный кабинет временно не работает");

            else if (replyData.contains("Доступные договоры"))
                parseContractsData(replyData);

            else if (replyData.contains("Мои услуги"))
            {
                loginRetry = 0;
                loggedIn = true;

                QRegExp rx;
                rx.setPattern("[0-9a-f]{32}");

                if (rx.indexIn(replyData) != -1)
                {
                    wtf = rx.cap(0).toAscii();
                    getCabinetData();
                }
                else
                    errorHolder(connError + "wtf");
            }

            else
            {
                if (loggedIn)
                {
                    updateTimer->stop();
                    turboEndTimer->stop();
                    loggedIn = turboBuyed = turboOn = false;
                }

                if (loginRetry < 3)
                    QTimer::singleShot(5000, this, SLOT(cabinetLogin()));
                else
                    errorHolder(connError + "неопознанная ошибка");
            }
        }

        // перенаправление после POST запросов
        else if (v >= 300 && v < 400)
        {
            QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            manager->get(QNetworkRequest(reply->url().resolved(redirectUrl)));
        }
    }
    else
        errorHolder("Сетевая ошибка: " + reply->errorString());
}

void Turbo::receivedSSLError(QNetworkReply *reply, QList<QSslError> errors)
{
    if (ignoreSslErr)
        reply->ignoreSslErrors();
    else
    {
        sslerror->set(errors);
        sslerror->show();
    }
}

void Turbo::ignoreSSLError()
{
    ignoreSslErr = true;
    cabinetLogin();
}

void Turbo::turboTime()
{
    QDateTime now = QDateTime::currentDateTime();
    int secondsLeft = now.secsTo(turboEndDateTime);
    QString time = QTime().addSecs(secondsLeft).toString("HH:mm");
    tray->setToolTip("Турбокнопка включена\nОсталось времени: " + time);
    ui->basement->setText("Услуга активна, осталось времени: " +
                          span.arg(time.replace(":", secondsLeft % 2 ? ":" : "<span style=\"color: #94d400;\">:</span>")));

    if (!turboEndTimer->isActive() && tray->isVisible())
        tray->showMessage("Услуга активирована", QString("Cрок действия услуги: %1 час%2")
                          .arg(hours[turboId]).arg(wordEnd(hours[turboId], "а", "ов")));

    if (turboEndDateTime <= now)
    {
        turboOn = turboBuyed = false;
        turboEndTimer->stop();
        tray->setToolTip("Турбокнопка");
        ui->basement->setText("");

        if (tray->isVisible())
            tray->showMessage("Услуга деактивирована", "Действие турбокнопки закончилось");

        getTurboHistory();
    }
}

void Turbo::turboActivate()
{
    if (turboBuyed)
    {
        bool link = bonusFirst && bonus >= price[turboId];
        ui->basement->setText("Активация услуги...");

        QNetworkRequest request(QUrl(site + (link ? "bonus/activate/" : "turbo")));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        manager->post(request, QByteArray(link ? "ids=" + bon[turboId] : "turbo_id=" + mon[turboId]));
    }
    else
    {
        turboBuyed = true;
        turboButtons();

        // необходимо для успешной активации услуги
        ui->basement->setText("Загрузка списка доступных услуг...");
        manager->get(QNetworkRequest(QUrl(site + "bonus")));
    }
}

void Turbo::turboAsk(int i, bool asked)
{
    int hour = hours[i];
    bool link = bonusFirst && bonus >= price[i];

    if (dontAsk || asked)
        turboActivate();
    else
    {
        turboId = i;
        ask->set(hour, price[i], link);

        if (ask->isHidden() && settings->isHidden() && about->isHidden())
            ask->show();
        else
            ask->activateWindow();
    }
}

void Turbo::turboButtons()
{
    for (int i = 0; i < bon.count(); i++)
    {
        bool enabled = (bonus >= price[i] || money >= price[i]) && !turboOn && !turboBuyed && daysLeft > 1;

        uiTurboButtons.at(i)->setEnabled(enabled);
        trayTurboButtons.at(i)->setEnabled(enabled);
    }
}

void Turbo::turboHovered()
{
    int i = QString(sender()->objectName()[6]).toInt();
    int hour = hours.at(i);
    int gb = 95 * 3600 * hour / 8000; // да, объём измеряется в СИ, а не в бинарной системе, как в Windows

    if (uiTurboButtons.at(i)->isEnabled())
        ui->basement->setText(
            QString("За это время можно скачать примерно %1 ГБ данных.\nСтоимость услуги - %2 %3.")
                .arg(gb)
                .arg(price[i])
                .arg(bonusFirst && bonus >= price[i] ? "бонусных баллов" : "рублей")
        );
}

void Turbo::turboOutered()
{
    bool t = true;

    for (int i = 0; i < bon.count(); i++)
        t = t & uiTurboButtons.at(i)->isEnabled();

    if (t)
        ui->basement->setText("");
}

void Turbo::errorHolder(QString errorText, bool showSettings)
{
    if (showSettings)
        settingsShow(errorText.contains("пароль"));

    ui->basement->setText(errorText);

    if (tray->isVisible())
    {
        QStringList text = errorText.split(": ");
        tray->showMessage(text[0], text[1], QSystemTrayIcon::Critical);
    }
}

void Turbo::daysWarn()
{
    if (tray->isVisible())
        tray->showMessage("На счёте заканчиваются средства", QString("Через %1 %2 интернет и телевидение будут отключены.")
                          .arg(daysLeft).arg(wordEnd(daysLeft, "дня", "дней", "день")), QSystemTrayIcon::Warning);
}

QString Turbo::wordEnd(int num, QString one, QString more, QString none)
{
    if ((num > 4) & (num < 21) || num % 10 > 4 || num % 10 == 0)
        return more;
    if (num % 10 == 1)
        return none;
    return one;
}

void Turbo::checkWindowState()
{
    if (isMinimized())
    {
        tray->show();
        // скрытие главного окна должно быть вызвано извне
        QTimer::singleShot(0, this, SLOT(hide()));
    }
}

void Turbo::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent *e = (QWindowStateChangeEvent *)event;

        if ((e->oldState() != Qt::WindowMinimized) && isMinimized())
        {
            checkWindowState();
            return;
        }
        else
        {
            // только в такой последовательности
            show();
            tray->hide();
            activateWindow();
            return;
        }

        event->ignore();
    }

    QMainWindow::changeEvent(event);
}

Turbo::~Turbo()
{
    delete ui;
}
