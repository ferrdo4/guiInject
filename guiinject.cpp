#include "guiinject.h"

#include <QFile>
#include <QApplication>
#include <QWidget>
#include <QMouseEvent>
#include <QComboBox>
#include "startuphelper.h"

#define LIB_INIT_FUNC __attribute__((constructor))
LIB_INIT_FUNC void guiInjectInit()
{
    qDebug() << "lib init";
    StartupHelper* initHelper = new StartupHelper(guiInject);
    QObject::connect(initHelper, SIGNAL(startupComplete()), initHelper, SLOT(deleteLater()));
    initHelper->watchForStartup();
}

void guiInject()
{
    qDebug() << "lib inject";
    new GuiInject(QCoreApplication::instance());
}

GuiInject::GuiInject(QObject* parent)
    : QObject(parent)
{
    m_server = new MaiaXmlRpcServer(8888, this);

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
    list.append(CMD_PING);
    list.append(CMD_READ_ALL);
    list.append(CMD_CLICK);
    list.append(CMD_READ_PROP);
    list.append(CMD_SET_PROP);
    list.append(CMD_SET_COMBO_IDX);
    list.append(CMD_READ_ALL_FILTER);

    return list;
}

QVariantMap GuiInject::runKeyword(QString name, QVariantList args)
{
    qDebug() << "executing" << name;
    QVariantMap result;
    result["status"] = "FAIL";

    if (!checkArguments(name, args.count(), result))
    {
        return result;
    }

    if (name == CMD_PING)
    {
        result["return"] = ping(args[0].toString());
    }
    else if (name == CMD_READ_ALL)
    {
        result["return"] = readAllObjects();
    }
    else if (name == CMD_CLICK)
    {
        click(args[0].toString());
        result["return"] = "";
    }
    else if ( name == CMD_READ_PROP )
    {
        result["return"] = readProperty(args[0].toString(), args[1].toString());
    }
    else if ( name == CMD_SET_PROP )
    {
        if ( setProperty(args[0].toString(), args[1].toString(), args[2].toString()) )
            result["return"] = "true";
        else
            return result;
    }
    else if ( name == CMD_SET_COMBO_IDX )
    {
        result["return"] = setComboIdx(args[0].toString(), args[1].toInt());
    }
    else if (  name == CMD_READ_ALL_FILTER )
    {
        result["return"] = readAllFilteredObjects(args[0].toString(), args[1].toString());
    }

    result["status"] = "PASS";
    return result;
}

QVariantList GuiInject::getKeywordArguments(QString name)
{
    QVariantList list;
    QList<QString> oneStringParam {CMD_PING, CMD_CLICK};
    QList<QString> twoStringParam {CMD_READ_PROP, CMD_SET_COMBO_IDX, CMD_READ_ALL_FILTER};
    QList<QString> threeStringParam {CMD_SET_PROP};

    if (oneStringParam.indexOf( name )!= -1)
        list.append("QString");
    else if (twoStringParam.indexOf(name) != -1)
    {
        list.append("QString");
        list.append("QString");
    }
    else if (threeStringParam.indexOf(name) != -1)
    {
        list.append("QString");
        list.append("QString");
        list.append("QString");
    }
    return list;
}

QString GuiInject::getKeywordDoc(QString name)
{
    if(name == CMD_PING)
        return QString("retruns pong + string");
    if(name == CMD_CLICK)
        return QString("click on object with name");
    if(name == CMD_READ_ALL)
        return QString("retrun list of all objects");
    if(name == CMD_READ_PROP)
        return QString("retrun property of selected object");
    if(name == CMD_SET_PROP)
        return QString("set property of selected object");
    if(name == CMD_SET_COMBO_IDX)
        return QString("set combobox by index");

    return QString();
}

bool GuiInject::stopRemoteServer()
{
    return true;
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

bool GuiInject::checkArguments(QString command, int count, QVariantMap &result)
{
    int countShouldBe = getKeywordArguments(command).count();
    if (count < countShouldBe)
    {
        result["error"] = QString("Invalit arguments number %1/%2").arg(count).arg(countShouldBe);
        return false;
    }
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

QVariantList GuiInject::readAllFilteredObjects(QString filter, QString value)
{
    QVariantList list;
    for(auto& key : m_objMap.keys())
    {
        QObject *obj = m_objMap[key];
        if (QWidget* pb = (QWidget*)obj)
        {
            QVariant var = pb->property(filter.toStdString().c_str());
            QString ret = var.toString();
            if (ret.isEmpty())
            {
                qDebug() << QString("Property %1 not found").arg(filter);
                continue;
            }
            if ( ret == value )
            {
                list.append(key);
            }
        }
    }
    return list;
}

void GuiInject::click(QString objName)
{
    QObject *obj = m_objMap[objName];
    if (!obj)
    {
        qDebug() << QString("Object %1 not found !").arg(objName);
    }

    // click on the desired object
    if (QWidget* pb = (QWidget*)obj)
    {
        int w = pb->width();
        int h = pb->height();
        QPoint pos(w/2, h/2);
        QApplication::postEvent(pb, new QMouseEvent(QEvent::MouseButtonPress, pos, Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier) );
        QApplication::postEvent(pb, new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier) );
    }
}

QString GuiInject::readProperty(QString objName, QString property)
{
    QString ret;
    QObject *obj = m_objMap[objName];
    if (!obj)
    {
        qDebug() << QString("Object %1 not found !").arg(objName);
    }

    if (QWidget* pb = (QWidget*)obj)
    {
        QVariant var = pb->property(property.toStdString().c_str());
        ret = var.toString();
        if (ret.isEmpty())
        {
            qDebug() << QString("Property %1 not found").arg(property);
        }
    }
    return ret;
}

bool GuiInject::setProperty(QString objName, QString property, QString value)
{
    bool ret = false;
    QObject *obj = m_objMap[objName];
    if (!obj)
    {
        qDebug() << QString("Object %1 not found !").arg(objName);
    }

    if (QWidget* pb = (QWidget*)obj)
    {
        ret = pb->setProperty(property.toStdString().c_str(), QVariant(value));
    }
    return ret;
}

bool GuiInject::setComboIdx(QString objName, int index)
{
    bool ret = false;
    QObject *obj = m_objMap[objName];
    if (!obj)
    {
        qDebug() << QString("Object %1 not found !").arg(objName);
    }
    if (QComboBox* pb = (QComboBox*)obj)
    {
        if (index <= pb->count())
        {
            pb->setCurrentIndex(index);
            ret = true;
        }
        else
        {
            ret = false;
        }
    }
    return ret;
}
