#include "startuphelper.h"

#include <QtConcurrentRun>
#include <QThread>
#include <QFuture>
#include <QApplication>

class Utils : public QThread
{
public:
    static void msleep(int ms)
    {
        QThread::msleep(ms);
    }
};


void StartupHelper::runInitFunc()
{
    m_initFunc();
    emit startupComplete();
}

void StartupHelper::doWait()
{
    while (QCoreApplication::startingUp())
    {
        qDebug() << "waiting ...";
        Utils::msleep(100);
    }
    QMetaObject::invokeMethod(this, "runInitFunc", Qt::QueuedConnection);
}

void StartupHelper::watchForStartup()
{
    qDebug() << "watch for startup";
    QtConcurrent::run(this, &StartupHelper::doWait);
}

