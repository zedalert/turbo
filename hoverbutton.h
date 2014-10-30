#ifndef HOVERBUTTON_H
#define HOVERBUTTON_H

#include <QPushButton>
#include <QEvent>

class HoverButton : public QPushButton
{
    Q_OBJECT

public:
    HoverButton(QWidget * parent = 0) : QPushButton(parent) {}
    ~HoverButton() {}

protected:
    void enterEvent(QEvent *e)
    {
        Q_EMIT hovered();
        QWidget::enterEvent(e);
    }
    void leaveEvent(QEvent *e)
    {
        Q_EMIT outered();
        QWidget::leaveEvent(e);
    }

signals:
    void hovered();
    void outered();
};

#endif // HOVERBUTTON_H
