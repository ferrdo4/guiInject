#include "directpick.h"

#include <QApplication>
#include <QWidget>
#include <QtCore/qglobal.h>
#include <qdebug.h>


DirectPick::DirectPick(QObject *parent)
    : QObject(parent)
{
}

bool DirectPick::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        if (QWidget* widget = static_cast<QWidget*>(obj))
            qDebug() << (*_map)[obj];
    }
    return false;
}

void DirectPick::setMap(QHash<QObject *, QString> *map)
{
    _map = map;
}

void DirectPick::start()
{
    qDebug() << "start pick";
    QCoreApplication::instance()->installEventFilter(this);
}

void DirectPick::stop()
{
    qDebug() << "stop pick";
    QCoreApplication::instance()->removeEventFilter(this);
}
