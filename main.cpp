#include <QtGui/QApplication>
#include <QLocalSocket>
#include "turbo.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8"));

    QLocalSocket socket;
    socket.connectToServer("TurboButton");
    if (socket.waitForConnected(100))
        return 0;
    else
    {
        Turbo w;
        w.show();
        return a.exec();
    }
}
