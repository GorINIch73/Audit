#ifndef MOUSEWHEELWIDGETADJUSTMENTGUARD_H
#define MOUSEWHEELWIDGETADJUSTMENTGUARD_H

#include <QObject>
#include <QWidget>


// блокировщик события прокрутки мыши на виджете

class MouseWheelWidgetAdjustmentGuard : public QObject
{
    Q_OBJECT
public:
    explicit MouseWheelWidgetAdjustmentGuard(QObject *parent = nullptr);

signals:

protected:
    bool eventFilter(QObject* o, QEvent* e) override;

};

#endif // MOUSEWHEELWIDGETADJUSTMENTGUARD_H
