#pragma once

#include <QtCore/QObject>
#include <QHash>


class DirectPick : public QObject
{
    Q_OBJECT

public:
    DirectPick(QObject* parent);
    virtual bool eventFilter(QObject* obj, QEvent* event);
    void setMap(QHash<QObject*, QString> *map);

private:
    QHash<QObject*, QString>* _map;

signals:

public slots:
    void start();
    void stop();
};
