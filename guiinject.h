#pragma once

#include <QString>
#include <QHash>
#include "guiinject_global.h"
#include "maiaXmlRpcServer.h"

#define CMD_PING QString("ping")
#define CMD_READ_ALL QString("read all objects")
#define CMD_CLICK QString("click")
#define CMD_READ_PROP QString("read property")
#define CMD_SET_PROP QString("set property")
#define CMD_SET_COMBO_IDX QString("set combobox index")
#define CMD_READ_ALL_FILTER QString("read filter objects")

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
    QVariantList readAllFilteredObjects(QString filter, QString value);
    void click(QString objName);
    QString readProperty(QString objName, QString property);
    bool setProperty(QString objName, QString property, QString value);
    bool setComboIdx(QString objName, int index);

// private methods
    void readObjectTree(QHash<QString, QObject*>& map, QObject *obj, QString path);
    void createObjMap();
    bool checkArguments(QString command, int count, QVariantMap& result);
};
