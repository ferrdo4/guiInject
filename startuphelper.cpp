#include "startuphelper.h"

#include <QtConcurrentRun>
#include <QThread>

void StartupHelper::runInitFunc()
{
    m_initFunc();
    emit startupComplete();
}

void StartupHelper::doWait()
{
    while (QCoreApplication::startingUp())
    {
        QThread::msleep(100);
    }
    QMetaObject::invokeMethod(this, "runInitFunc", Qt::QueuedConnection);
}

void StartupHelper::watchForStartup()
{
    QtConcurrent::run(this, &StartupHelper::doWait);
}

