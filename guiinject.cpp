#include "guiinject.h"

#include <QFile>
#include <QApplication>
#include <QWidget>
#include <QMouseEvent>
#include "startuphelper.h"

#define LIB_INIT_FUNC __attribute__((constructor))
LIB_INIT_FUNC void guiInjectInit()
{
    StartupHelper* initHelper = new StartupHelper(guiInject);
    QObject::connect(initHelper, SIGNAL(startupComplete()), initHelper, SLOT(deleteLater()));
    initHelper->watchForStartup();
}

void guiInject()
{
    new GuiInject(QCoreApplication::instance());
}

GuiInject::GuiInject(QObject* parent)
    : QObject(parent)
{
    m_server = new MaiaXmlRpcServer(8080, this);
    // dynamic RF api
    m_server->addMethod("get_keyword_names", this, "getKeywordNames");
    m_server->addMethod("run_keyword", this, "runKeyword");
    m_server->addMethod("get_keyword_arguments", this, "getKeywordArguments");
    m_server->addMethod("get_keyword_documentation", this, "getKeywordDoc");
    m_server->addMethod("stop_remote_server", this, "stopRemoteServer");

    createObjMap();
}

QVariantList GuiInject::getKeywordNames()
{
    QVariantList list;
    list.append("ping");
    list.append("read all objects");
    list.append("click");
    return list;
}

QVariantMap GuiInject::runKeyword(QString name, QVariantList args)
{
    qDebug() << "executing" << name;
    QVariantMap result;
    result["status"] = "FAIL";

    if (name == "ping")
    {
        unsigned int count = args.count();
        if (count != 1)
        {
            result["error"] = " invalid arguments number";
            return result;
        }
        result["return"] = ping(args[0].toString());
    }
    else if (name == "read all objects")
    {
        result["return"] = readAllObjects();
    }
    else if (name == "click")
    {
        unsigned int count = args.count();
        if (count != 1)
        {
            result["error"] = " invalid arguments number";
            return result;
        }
        click(args[0].toString());
        result["return"] = "";
    }

    result["status"] = "PASS";
    return result;
}

QVariantList GuiInject::getKeywordArguments(QString name)
{
    QVariantList list;
    if (name == "ping" || name == "click")
        list.append("QString");

    return list;
}

QString GuiInject::getKeywordDoc(QString name)
{
    if(name == "ping")
        return QString("retruns pong + string");
    if(name == "click")
        return QString("click on object with name");

    return QString();
}

bool GuiInject::stopRemoteServer()
{
    return true;
}

QString GuiInject::ping(QString str)
{
    QString ret = QString("pong %1").arg(str);
    return ret;
}

QVariantList GuiInject::readAllObjects()
{
    QVariantList list;
    for(auto& key : m_objMap.keys())
    {
        list.append(key);
    }
    return list;
}

void GuiInject::click(QString objName)
{
    QObject *obj = m_objMap[objName];
    if (obj)
    {
        if (QWidget* pb = (QWidget*)obj)
        {
            QPoint pos(0, 0);
            QApplication::postEvent(pb, new QMouseEvent(QEvent::MouseButtonPress, pos, Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier) );
            QApplication::postEvent(pb, new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier) );
        }
    }
}

void GuiInject::readObjectTree(QHash<QString, QObject*> &map, QObject* obj)
{
    QString name = obj->objectName();
    map[name] = obj;
    for(auto& child : obj->children())
    {
        readObjectTree(map, child);
    }
}

void GuiInject::createObjMap()
{
    auto widgets = QApplication::topLevelWidgets();

    for(auto& object : widgets)
    {
        readObjectTree(m_objMap, object);
    }
}
