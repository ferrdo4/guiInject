#pragma once

#include <QString>
#include <QHash>
#include "guiinject_global.h"
#include "maiaXmlRpcServer.h"


void GUIINJECTSHARED_EXPORT guiInject();

class GUIINJECTSHARED_EXPORT GuiInject : public QObject
{
    Q_OBJECT

public:
    GuiInject(QObject *parent);

public slots:
// remote library methods
    QVariantList getKeywordNames();
    QVariantMap runKeyword(QString name, QVariantList args);
    QVariantList getKeywordArguments(QString name);
    QString getKeywordDoc(QString name);
    bool stopRemoteServer();

private:
    MaiaXmlRpcServer* m_server;
    QHash<QString, QObject*> m_objMap;

// server methods
    QString ping(QString str);
    QVariantList readAllObjects();
    void click(QString objName);

    void readObjectTree(QHash<QString, QObject*>& map, QObject *obj);
    void createObjMap();
};
