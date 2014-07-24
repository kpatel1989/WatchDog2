#include "locacc.h"

QString LOC_LANG_FOLDER = "lang";
QString LOC_EN_FOLDER = "en";
QString LOC_DATA_FOLDER = "data";
QString LOC_FILE_NAME = "loc-acc.json";

LOCACC::LOCACC(QString strPath)
{
    QStringList treeList;
    treeList << "locAccData";
    root = new QTreeWidgetItem(treeList);
    changeBasePath(strPath);
}

void LOCACC::changeBasePath(QString strPath)
{
    str_basePath  = strPath;
    readFile();
    emptyTreeWidget(root);
    getLocAccTree();
}

void LOCACC::emptyTreeWidget(QTreeWidgetItem *parent)
{
    QTreeWidgetItem *child;
    for(int i = 0 ; i < parent->childCount(); i++)
    {
        child = parent->child(i);
        emptyTreeWidget(child);
        parent->removeChild(child);
        delete child;
    }
}

bool LOCACC::addScreen(QStringList screenData)
{
    QJsonObject newJObjScreen;
    if(screenExists(screenData))
    {
        return false;
    }
    newJObjScreen["id"] = screenData.at(0);
    newJObjScreen["name"] = screenData.at(1);
    QJsonArray tempJArray;
    newJObjScreen["elements"] = tempJArray;
    QJsonArray locDataJArray = masterJObj["locAccData"].toArray();
    locDataJArray.append(newJObjScreen);
    masterJObj["locAccData"] = locDataJArray;
    QStringList newScreen;
    newScreen << screenData.at(0); ;
    QTreeWidgetItem *newScreenWidget = new QTreeWidgetItem(newScreen);
    root->addChild(newScreenWidget);
    writeFile();
    return true;
}

bool LOCACC::addElement(QStringList elementData, QTreeWidgetItem *parent)
{
    QJsonArray jArray = masterJObj["locAccData"].toArray();
    QString parentScreen = parent->text(0);
    QJsonObject tempObj ;
    for(int i = 0 ; i < jArray.count() ; i++ )
    {
        tempObj = jArray.at(i).toObject();
        if(tempObj["id"] == parentScreen)
        {
            QJsonArray eleJArray = tempObj["elements"].toArray();
            if(elementExists(elementData,eleJArray))
            {
                return false;
            }
            QJsonObject newEleObj = getElementJson(elementData);
            eleJArray.append(newEleObj);
            tempObj["elements"] = eleJArray;
            jArray.replace(i,tempObj);

            QStringList strList;
            strList << elementData;
            QTreeWidgetItem *newEleWidget = new QTreeWidgetItem(strList);
            parent->addChild(newEleWidget);
            break;
        }
    }
    masterJObj["locAccData"] = jArray;
    writeFile();
    return true;
}

bool LOCACC::addMessage(QStringList msgData, bool isAccTextSame, QTreeWidgetItem *parent)
{
    return false;
}

bool LOCACC::screenExists(QStringList screenData)
{
    QString screenId = screenData.at(0);
    QJsonArray locArray = masterJObj["locAccData"].toArray();
    QJsonObject tempObj;
    for(int i = 0 ; i < locArray.count() ; i++)
    {
        tempObj = locArray.at(i).toObject();
        if(tempObj["id"] == screenId)
        {
            return true;
        }
    }
    return false;
}

bool LOCACC::elementExists(QStringList elementData, QJsonArray parentScreenJArray)
{
    QString eleId = elementData.at(0);
    QJsonObject tempObj;
    for(int i = 0 ; i < parentScreenJArray.count(); i++)
    {
        tempObj = parentScreenJArray.at(i).toObject();
        if(tempObj["id"] == eleId)
        {
            return true;
        }
    }
    return false;
}

bool LOCACC::messageExists(QStringList messageData)
{

}


QTreeWidgetItem * LOCACC::getLocAccTree()
{
    QJsonArray locAccArray = masterJObj["locAccData"].toArray();
        QJsonObject screenJObj;
        for(int i = 0 ; i < locAccArray.count() ; i++)
        {
            screenJObj = locAccArray.at(i).toObject();
            root->addChild(getScreenTree(screenJObj));
        }
    return root;
}

QTreeWidgetItem* LOCACC::getScreenTree(QJsonObject screenJObj)
{
    QStringList treeList;
    treeList << screenJObj["id"].toString();
    QTreeWidgetItem *screenItemWidget = new QTreeWidgetItem(treeList);
    screenItemWidget->addChildren(getElementsTree(screenJObj));
    return screenItemWidget;
}

QList<QTreeWidgetItem *> LOCACC::getElementsTree(QJsonObject screenJObj)
{
    QStringList treeList;
    QJsonObject eleJObject;
    QList<QTreeWidgetItem *> eleTreeItemlist;
    QJsonArray eleJArray = screenJObj["elements"].toArray();
    for(int i = 0 ; i < eleJArray.count(); i++)
    {
        treeList.clear();
        eleJObject = eleJArray.at(i).toObject();
        treeList << eleJObject["id"].toString();
        QTreeWidgetItem *ele = new QTreeWidgetItem(treeList);
        qDebug() << eleJArray.at(i).toString();
        eleTreeItemlist.append(ele);
    }
    return eleTreeItemlist;
}

QJsonObject LOCACC::getElementJson(QStringList elementData)
{
    QJsonArray tempMsgArray;
    QJsonObject eleObj ;
    eleObj["id"] = elementData.at(0);
    eleObj["accId"] =  elementData.at(1);
    eleObj["type"] = elementData.at(2);
    eleObj["role"] =  elementData.at(3);
    eleObj["tabIndex"] =  elementData.at(4);
    eleObj["messages"] = tempMsgArray;
    return eleObj;
}

QString LOCACC :: getLocAccFilePath()
{
    return str_basePath + "/" + LOC_LANG_FOLDER
            + "/" + LOC_EN_FOLDER +  + "/" + LOC_DATA_FOLDER  + "/" + LOC_FILE_NAME;
}

void LOCACC::readFile()
{
    locAccFile.setFileName(getLocAccFilePath());
    locAccFile.open(QIODevice::ReadOnly | QIODevice::Text);
    if(!locAccFile.exists())
    {
        qDebug() << " LOC ACC READING FAILED :" << getLocAccFilePath();
        return;
    }
    QByteArray rawData = locAccFile.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(rawData));
    masterJObj = doc.object();
    qDebug() << "READ : " << masterJObj;
}

void LOCACC :: writeFile()
{
    QString filePath = getLocAccFilePath();
    locAccFile.setFileName(filePath);
    locAccFile.remove();
    locAccFile.open(QIODevice::ReadWrite | QIODevice::Text);
    if(!locAccFile.exists())
    {
        qDebug() << " LOC ACC DO NOT EXIST . NEW WILL BE CREATED :" << getLocAccFilePath();
    }
    qDebug() << "WRITE : \n" << masterJObj;
    QJsonDocument doc(masterJObj);
    locAccFile.write(doc.toJson());
    locAccFile.close();
}
