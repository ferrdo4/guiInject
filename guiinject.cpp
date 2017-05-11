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
    putenv(QString("LD_PRELOAD=").toLatin1().data());
    StartupHelper* initHelper = new StartupHelper(guiInject);
    QObject::connect(initHelper, SIGNAL(startupComplete()), initHelper, SLOT(deleteLater()));
    initHelper->watchForStartup();
}

void guiInject()
{
    const char* TOK = "GI_TOKEN";
    QString token = QString(getenv(TOK));
    qDebug() << "lib inject with token " << token;
    putenv(QString("GI_TOKEN=").toLatin1().data());
    new GuiInject(QCoreApplication::instance(), token);
}

GuiInject::GuiInject(QObject* parent, const QString& token)
    : QObject(parent)
    ,_token(token)
    ,_server(nullptr)
    ,_objMap(QHash<QString, QObject*>())
    ,_objMapIdx(QHash<QObject*, QString>())
    ,_pick(new DirectPick(parent))
{
    _server = new MaiaXmlRpcServer(8888, this);

    // dynamic RF api
    _server->addMethod("get_keyword_names", this, "getKeywordNames");
    _server->addMethod("run_keyword", this, "runKeyword");
    _server->addMethod("get_keyword_arguments", this, "getKeywordArguments");
    _server->addMethod("get_keyword_documentation", this, "getKeywordDoc");
    _server->addMethod("stop_remote_server", this, "stopRemoteServer");

    _pick->start();

    createObjMap();
    _pick->setMap(&_objMapIdx);
}

QVariantList GuiInject::getKeywordNames()
{
    QVariantList list;
    list.append(CMD_PING);
    list.append(CMD_READ_ALL);
    list.append(CMD_CLICK);
    list.append(CMD_KEYPRESS);
    list.append(CMD_READ_PROP);
    list.append(CMD_SET_PROP);
    list.append(CMD_SET_COMBO_IDX);
    list.append(CMD_FIND_PATH);
    list.append(CMD_PICK_START);
    list.append(CMD_PICK_STOP);
    list.append(CMD_REFRESH);

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
    else if (name == CMD_KEYPRESS)
    {
        keyPress(args[0].toString());
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
    else if ( name == CMD_FIND_PATH )
    {
        result["return"] = findPaths(args[0].toList());
    }
    else if ( name == CMD_PICK_START )
    {
        _pick->start();
        result["return"] = "true";
    }
    else if ( name == CMD_PICK_STOP )
    {
        _pick->stop();
        result["return"] = "true";
    }
    else if ( name == CMD_REFRESH )
    {
        createObjMap();
        result["return"] = "true";
    }

    result["status"] = "PASS";
    return result;
}

QVariantList GuiInject::getKeywordArguments(QString name)
{
    QVariantList list;
    QList<QString> oneStringParam {CMD_PING, CMD_CLICK, CMD_KEYPRESS};
    QList<QString> oneListStringParam {CMD_FIND_PATH};
    QList<QString> twoStringParam {CMD_READ_PROP, CMD_SET_COMBO_IDX};
    QList<QString> threeStringParam {CMD_SET_PROP};

    if (oneStringParam.indexOf( name )!= -1)
    {
        list.append("QString");
    }
    else if (oneListStringParam.indexOf(name) != -1)
    {
        list.append("QVariantList<QString>");
    }
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
    if(name == CMD_KEYPRESS)
        return QString("sends text string to focused object");
    if(name == CMD_READ_ALL)
        return QString("retrun list of all objects");
    if(name == CMD_READ_PROP)
        return QString("retrun property of selected object");
    if(name == CMD_SET_PROP)
        return QString("set property of selected object");
    if(name == CMD_SET_COMBO_IDX)
        return QString("set combobox by index");
    if(name == CMD_FIND_PATH)
        return QString("find path based on string list");

    return QString();
}

bool GuiInject::stopRemoteServer()
{
    return true;
}

void GuiInject::readObjectTree(QHash<QString, QObject*>& map, QObject* obj, QString path)
{
    QString name = obj->objectName();
    path = QString("%1_%2").arg(path).arg(name);

    for(auto& child : obj->children())
    {
        readObjectTree(map, child, path);
    }

    if (QWidget* pb = (QWidget*)obj)
    {
        Q_UNUSED(pb);
        if (path.contains(_token))
        {
            map[path] = obj;
            _objMapIdx[obj] = path;
        }
    }
}

void GuiInject::createObjMap()
{
    auto widgets = QApplication::topLevelWidgets();

    QString path = QString();
    
    _objMap.clear();
    _objMapIdx.clear();

    for(auto& object : widgets)
    {
        readObjectTree(_objMap, object, path);
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
    for(auto& key : _objMap.keys())
    {
        list.append(key);
    }
    return list;
}

QVariantList GuiInject::findPaths(QVariantList marks)
{
    QVariantList list;

    for (auto& key : _objMap.keys())
    {
        //qDebug() << key;
        bool escape = false;
        for (auto& mark : marks)
        {
            qDebug() << mark;
            if (!key.contains(mark.toString()))
                escape = true;
        }
        if (escape)
            continue;
        list.append(key);
    }

    return list;
}

void GuiInject::click(QString objName)
{
    QObject *obj = _objMap[objName];
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

void GuiInject::keyPress(QString key)
{
    QObject *obj = QApplication::focusWidget();
    if (!obj)
    {
        qDebug() << QString("No focused object");
        return;
    }

    if (QWidget* pb = (QWidget*)obj)
    {
        pb->setFocus();

        QKeyEvent keyPress( QKeyEvent::KeyPress, key.at(0).toLatin1(), Qt::NoModifier, key, false, 0 );
        QApplication::sendEvent( pb, &keyPress );

        QKeyEvent keyRelease( QKeyEvent::KeyRelease, key.at(0).toLatin1(), Qt::NoModifier, key, false, 0 );
        QApplication::sendEvent( pb, &keyRelease );
    }
}


QString GuiInject::readProperty(QString objName, QString property)
{
    QString ret;
    QObject *obj = _objMap[objName];
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
    QObject *obj = _objMap[objName];
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
    QObject *obj = _objMap[objName];
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
