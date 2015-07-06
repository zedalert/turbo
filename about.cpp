#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    ui(new Ui::About)
{
    ui->setupUi(this);

    ui->txt->setText(
        "Версия: 2.8 от 06.07.2015<br/>"
        "Автор идеи: <a href=\"http://vk.com/askubakov\" style=\"color: #82c800;\">Александр Скубаков</a><br/>"
        "Обсуждение проекта: <a href=\"http://forum.onlime.ru/index.php?showtopic=12307\" style=\"color: #82c800;\">форум провайдера</a><br/>"
        "Исходный код (GNU GPL): <a href=\"https://github.com/zedalert/turbo/\" style=\"color: #82c800;\">GitHub</a><br/>"
        "Горячие клавиши:<br/>"
        "- F1 - показать это окно;<br/>"
        "- F5 - обновить данные;<br/>"
        "- Esc - свернуть."
        "<p align=\"justify\">Это программное обеспечение создано без участия ОАО \"Ростелеком\" и "
        "ОАО \"Национальные кабельные сети\". Все права на товарные знаки принадлежат их правообладателям.</p>"
    );
}

About::~About()
{
    delete ui;
}
