#include "cBaseVar.h"
#include "include/taskmanager/taskmanger.h"
int cBaseVar::rootValue;
cBaseVar rootVar(nullptr, &cBaseVar::rootValue);
QHash<QString, cBaseFunc> cBaseVar::cTypeFunc;
bool cBaseVar::bInited = false;

QMutex cBaseVar::logMtx;
QVector<logInfo> cBaseVar::logVec;

QSqlDatabase cBaseVar::db;
QMutex cBaseVar::dbMtx;

QMutex cBaseVar::hashMtx;
void* cBaseVar::taskPool = nullptr;

cBaseVar::~cBaseVar()
{}



bool cBaseVar::isNull(void)
{
	if (mem == nullptr) {
		return true;
	}
	return false;
}
bool cBaseVar::isVALUE(void)
{
    if (func == nullptr) {
        auto it = cTypeFunc.find(type.name());
        if (it != cTypeFunc.end()) {
            func = &it.value();
        }
    }
    if (func == nullptr) {
        return false;
    }
    if (func->toString == nullptr) {
        return false;
    }
	return func->isValue(mem , this);
}

/*将任意数据转换为QString输出*/
QString cBaseVar::toString(void)
{
    if (isNull()) {
        return "";
    }
    QString str = "";
    if (func == nullptr) {
        auto it = cTypeFunc.find(type.name());
        if (it != cTypeFunc.end()) {
            func = &it.value();
        }
    }
    if (func == nullptr) {
        return str;
    }
    if (func->toString == nullptr) {
        return str;
    }
    str = func->toString(mem , this);
    return str;
}

/*将任意数据由QString赋值*/
int cBaseVar::byString(QString str)
{
    if (isNull()) {
        return -1;
    }
    auto it = cTypeFunc.find(type.name());
    if (func == nullptr) {
        auto it = cTypeFunc.find(type.name());
        if (it != cTypeFunc.end()) {
            func = &it.value();
        }
    }
    if (func == nullptr) {
        return -1;
    }
    if (func->byString == nullptr) {
        return -1;
    }
    return func->byString(mem , str , this);
}

/*检查是否可以访问*/
bool cBaseVar::checkLevel()
{
    //if (dir != C_INPUT) {
    //    return false;
    //}
    return true;
}

/**
 * 插入新节点并返回指针。进行查重，如果路径已存在或插入失败，返回空指针。
 * 如果中间节点不存在，会自动创建这些节点。
 *
 * @param path 要插入的节点的路径，用"."分割。
 * @param var 要插入的节点的数据。
 * @return 插入的节点的指针，如果节点已存在或插入失败，则为nullptr。
 */
cBaseVar* cBaseVar::insertNode(const QString& path, cBaseVar* var) {
    QStringList pathParts = path.split('.');
    QString currentPath;
    QHash<QString, cBaseVar *>* currentHash = &rootVar.cHash;

    // 导航到应该插入新节点的父节点位置
    for (int i = 0; i < pathParts.size() - 1; ++i) {
        currentPath += (i > 0 ? "." : "") + pathParts[i];
        if (!currentHash->contains(pathParts[i])) {
            bool* var0ptr = nullptr;
            cBaseVar* var0 = new cBaseVar(nullptr, var0ptr);
            currentHash->insert(pathParts[i], var0);  // 如果节点不存在，则创建一个新的空节点
        }
        currentHash = &((*currentHash)[pathParts[i]]->cHash);
    }

    // 检查最后一个元素是否已存在
    if (currentHash->contains(pathParts.last())) {
        return nullptr;  // 路径已存在，返回空指针
    }

    // 插入最后一个元素
    currentHash->insert(pathParts.last(), var);
    return ((*currentHash)[pathParts.last()]);
}
/**
 * 寻找指定路径的节点并返回节点指针。
 * 使用 "." 作为分隔符来分割路径的各个部分。
 *
 * @param path 要查找的节点的路径，用 "." 分割。
 * @return 如果找到了指定路径的节点，则返回指向该节点的指针；
 *         如果路径中的任何部分不存在，则返回 nullptr。
 */

cBaseVar* cBaseVar::findNode(const QString& path) 
{
    QStringList pathParts = path.split('.');
    QHash<QString, cBaseVar*>* currentHash = &rootVar.cHash;

    for (int i = 0; i < pathParts.size(); i++) {
        if (!currentHash->contains(pathParts[i])) {
            return nullptr;  // 如果路径部分不存在，则返回 nullptr
        }
        // 检查是否为最后一部分，如果是，返回该部分的指针
        if (i == pathParts.size() - 1) {
            return (*currentHash)[pathParts[i]];
        }
        currentHash = &((*currentHash)[pathParts[i]]->cHash);  // 移动到下一层级
    }

    return nullptr;  // 理论上不应该达到这里，但保留以避免潜在的编译器警告
}




//template<typename T>
//static int cBaseVar::regTypeFunc(cBaseFunc func)
//{
//    QString typeStr0 = typeid(T).name();
//    cBaseVar::cTypeFunc[typeStr0] = func;
//    return 0;
//}






int cBaseVar::insertLog(infoType type, QString title, QString info)
{
    return insertLog( type,  title,  info,  u8"");
}
int cBaseVar::insertLog(infoType type, QString title, QString info, QString actionName)
{
    logInfo log0;
    log0.type = type;
    log0.title = title;
    log0.info = info;
    log0.actionName = actionName;
    logMtx.lock();
    logVec.append(log0);
    if (logVec.size() > 1000) {
        logVec.remove(0);
    }
    logMtx.unlock();
    return 0;
}


QVector<QString> cBaseVar::collectNames(const QString& searchStr)
{
    QVector<QString> names;
    const QHash<QString, cBaseVar*>& hash = rootVar.cHash;
    hashMtx.lock();
    collectNamesRecursive(hash, searchStr, names);
    hashMtx.unlock();
    return names;
}
void cBaseVar::collectNamesRecursive(const QHash<QString, cBaseVar*>& hash, const QString& searchStr, QVector<QString>& names)
{
    for (auto it = hash.begin(); it != hash.end(); ++it) {
        if (searchStr.isEmpty() || it.value()->fullName.contains(searchStr, Qt::CaseInsensitive)) {
            if (it.value()->realMem != nullptr) {
                names.push_back(it.value()->fullName);
            }
            
        }
        collectNamesRecursive(it.value()->cHash, searchStr, names); // 递归调用以处理嵌套结构
    }
}
/*通用connect函数*/

int cBaseVar::connect0(cBaseVar* src, cBaseVar* dest)
{

    if (dest->bind == true) {
        return -1;
    }
    if (dest->func->connect != nullptr) {
        dest->func->connect(src, dest);
    }
    dest->bindName = src->fullName;
    dest->bind = true;
    dest->mem = src->mem;
    return 0;
}
/*通用disconnect函数*/

int cBaseVar::disconnect0(cBaseVar* src, cBaseVar* dest)
{
    if (dest->func->disconnect != nullptr) {
        dest->func->disconnect(src, dest);
    }
   
    dest->bindName = "";
    dest->bind = false;
    dest->mem = dest->realMem;
    return 0;
}
int cBaseVar::addTask(void* lpStartAddress, void* lpParameter)
{
    if (taskPool == nullptr) {
        TaskManger *taskPool0 = new TaskManger();
        taskPool0->init(10);
        taskPool = taskPool0;
    }
    TaskManger* taskPool1 = (TaskManger*)taskPool;
    taskPool1->addTask(lpStartAddress, lpParameter);
    return 0;
}
/*打开数据库*/
int cBaseVar::openDB(QString path)
{
    int ref = 0;
    dbMtx.lock();
    if (db.isOpen()) {
        db.close();
    }
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    }
    else {
        db = QSqlDatabase::addDatabase("QSQLITE");
    }

    db.setDatabaseName(path);
    //打开数据库
    if (!db.open()) {
        //db_lastError = u8"打开数据库 " + path + u8" 失败";
        //insertLog(0, u8"数据库", db_lastError);
        ref = -1;
        goto error_out;
    }
    allDBToVar();
    dbMtx.unlock();
    //insertLog(0, u8"数据库", u8"打开数据库 " + path + u8" 成功");
    //return all_db_to_vars();
    
    return 0 ;
error_out:
    dbMtx.unlock();
    return ref;
}
/*创建数据库*/
int cBaseVar::newDB(QString path)
{
    int ref = 0;
    dbMtx.lock();

    if (db.isOpen()) {
        db.close();
    }
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    }
    else {
        db = QSqlDatabase::addDatabase("QSQLITE");
    }

    db.setDatabaseName(path);
    db.open();
    QString error = db.lastError().text();


    
    QStringList drivers = QSqlDatabase::drivers();
    foreach(QString driver, drivers) {
        QString driver0 = driver;
    }
       


    QSqlQuery query(db);

    const QString sql_check = "SELECT COUNT(*) FROM sqlite_master where type ='table' and name ='CBaseVars'";
    const QString sql = R"(
                      CREATE TABLE IF NOT EXISTS  CBaseVars(
                      id   INTEGER   PRIMARY KEY AUTOINCREMENT NOT NULL,
                      name TEXT UNIQUE NOT NULL,
                      type TEXT,
                      dir INTEGER,
                      var TEXT,
                      level INTEGER,
                      max REAL,
                      min REAL,
                      info TEXT,
                      retain INTEGER,
                      accur INTEGER,
                      base INTEGER,
                      bind INTEGER,
                      bindName TEXT,
                      jsObjStr TEXT
                      );)";

    //打开数据库
    if (!db.isOpen()) {
        //db_lastError = u8"打开数据库 " + path + u8" 失败";
        //insertLog(0, u8"数据库", db_lastError);
        ref = -1;
        goto error_out;
    }

    if (!query.exec(sql)) {
        //db_lastError = path + u8" 执行创建表命令失败";
        //insertLog(0, u8"数据库", db_lastError);
        ref = -2;
        goto error_out;
    }
    query.finish();
    allVarToDB();
    dbMtx.unlock();
    //insertLog(0, u8"数据库", u8"创建数据库 " + path + u8" 成功");
    //return all_vars_to_db();
    return 0;
error_out:
    query.finish();
    dbMtx.unlock();
    return ref;
}


int cBaseVar::connectDB()
{
    QDir currentDir = QDir::currentPath();
    if (currentDir.exists("data")){
        
    }else {
        currentDir.mkdir("data");
    }

    QString path = QDir::currentPath() + "/" + "data" + "/" + "varData.db";
    QFile dbFile(path);
    if (dbFile.exists()) {
        openDB(path);
    }else {
        newDB(path);
    }

    return 0;
}
/*单个数据存入数据库*/
int cBaseVar::varToDB()
{
    //exVarToDB(this);
    addTask(exVarToDB,this);
    return 0;
}
void* cBaseVar::exVarToDB(void* var0)
{
    int ref = 0;
    cBaseVar* var = (cBaseVar*)var0;  
    if (!db.isOpen()) {
        //db_lastError = u8"数据库未打开";
        //insertLog(0, u8"数据库", db_lastError);
        return nullptr;
    }
    dbMtx.lock();
    QSqlQuery query(db);
    bool execRef;
    QString execstr;
    QString fullName = var->fullName;

    query.exec(QString(R"(SELECT * FROM CBaseVars WHERE name='%1';)")
        .arg(fullName));
    if (!query.next()) {
        //QSqlQuery query;
        query.prepare(R"(
            INSERT INTO CBaseVars (name, type, dir, var, level, max, min, info, retain, accur, base, bind, bindName, jsObjStr) 
            VALUES (:name, :type, :dir, :var, :level, :max, :min, :info, :retain, :accur, :base, :bind, :bindName, :jsObjStr) 
        )");
        query.bindValue(":name", var->fullName);
        query.bindValue(":type", var->typeName);
        query.bindValue(":dir", var->dir);
        query.bindValue(":var", var->toString());
        query.bindValue(":level", var->level);
        query.bindValue(":max", var->max);
        query.bindValue(":min", var->min);
        query.bindValue(":info", var->info);
        query.bindValue(":retain", var->retain);
        query.bindValue(":accur", var->accr);
        query.bindValue(":base", var->base);
        query.bindValue(":bind", var->bind);
        query.bindValue(":bindName", var->bindName);
        QJsonDocument jsonDoc(var->jsObj);
        QString jsObjStr = jsonDoc.toJson();
        query.bindValue(":jsObjStr", jsObjStr);
        if (!query.exec()) {
            //qDebug() << "Error: Failed to insert data!" << query.lastError().text();
            dbMtx.unlock();
            return nullptr;
        }
        dbMtx.unlock();
        return nullptr;
    }else {
        query.prepare(R"(
            UPDATE CBaseVars SET name = :name, type = :type, dir = :dir, var = :var, level = :level, max = :max, min = :min, info = :info, retain = :retain, accur = :accur, base = :base, bind = :bind, bindName = :bindName, jsObjStr = :jsObjStr 
            WHERE name = :name
        )");
        query.bindValue(":name", var->fullName);
        query.bindValue(":type", var->typeName);
        query.bindValue(":dir", var->dir);
        query.bindValue(":var", var->toString());
        query.bindValue(":level", var->level);
        query.bindValue(":max", var->max);
        query.bindValue(":min", var->min);
        query.bindValue(":info", var->info);
        query.bindValue(":retain", var->retain);
        query.bindValue(":accur", var->accr);
        query.bindValue(":base", var->base);
        query.bindValue(":bind", var->bind);
        query.bindValue(":bindName", var->bindName);
        QJsonDocument jsonDoc(var->jsObj);
        QString jsObjStr = jsonDoc.toJson();
        query.bindValue(":jsObjStr", jsObjStr);
        if (!query.exec()) {
            //qDebug() << "Error: Failed to insert data!" << query.lastError().text();
            dbMtx.unlock();
            return nullptr;

        }
        dbMtx.unlock();
        return nullptr;

    }
    query.finish();
    dbMtx.unlock();
    return nullptr;
error_out:
    query.finish();
    dbMtx.unlock();
    return nullptr;

}
/*整体存入数据库*/
int cBaseVar::allVarToDB()
{
    QVector<QString> names = collectNames("");
    for (int i = 0; i < names.size(); i++) {
        cBaseVar *var = findNode(names[i]);
        if (var != nullptr) {
            var->varToDB();
        }
    }
    return 0;
}
/*单个从数据库读取*/
int cBaseVar::dbToVar()
{
    addTask(exDBToVar, this);
    return 0;
}
QJsonObject stringToJsonObject(const QString& jsonString) {
    // 将QString转换为QByteArray
    QByteArray byteArray = jsonString.toUtf8();

    // 解析QByteArray为QJsonDocument
    QJsonDocument jsonDoc = QJsonDocument::fromJson(byteArray);

    // 检查是否是一个有效的JSON文档，并且是一个对象
    if (!jsonDoc.isNull() && jsonDoc.isObject()) {
        return jsonDoc.object(); // 返回QJsonObject
    }
    else {
        //qWarning() << "Failed to parse the JSON string or the JSON is not an object.";
        return QJsonObject(); // 返回一个空的QJsonObject
    }
}
void* cBaseVar::exDBToVar(void* var0)
{
    int ref = 0;
    cBaseVar* var = (cBaseVar*)var0;
    if (!db.isOpen()) {
        //db_lastError = u8"数据库未打开";
        //insertLog(0, u8"数据库", db_lastError);
        return nullptr;
    }
    dbMtx.lock();
    QSqlQuery query(db);
    query.prepare("SELECT name, type, dir, var, level, max, min, info, retain, accur, base, bind, bindName, jsObjStr FROM CBaseVars WHERE name = :name");
    query.bindValue(":name", var->fullName);

    if (!query.exec()) {
        //qDebug() << "Error: Failed to execute query!" << query.lastError().text();
        dbMtx.unlock();
        return nullptr;
    }

    // 检查是否有结果
    if (query.next()) {
        var->retain = query.value("retain").toString().toInt();
        //if (var->retain) {
            var->byString(query.value("var").toString());
        //}
        var->level = query.value("level").toString().toInt();
        var->max = query.value("max").toString().toDouble();
        var->min = query.value("min").toString().toDouble();
        var->info = query.value("info").toString();
        var->accr = query.value("accur").toString().toInt();
        var->base = query.value("base").toString().toInt();
        //var->bind = query.value("bind").toString().toInt();
        //var->bindName = query.value("bindName").toString();
        QString jsStr = query.value("jsObjStr").toString();

        var->jsObj = stringToJsonObject(jsStr);    

        if (var->dir == C_INPUT) {
            QString bindName = query.value("bindName").toString();
            cBaseVar* varSrc = findNode(bindName);
            if (varSrc != nullptr) {
                connect0(varSrc , var);
            }
        }
    }else {

    }
    dbMtx.unlock();
    return nullptr;
}
/*整体存入数据库*/
int cBaseVar::allDBToVar()
{
    QVector<QString> names = collectNames("");
    for (int i = 0; i < names.size(); i++) {
        cBaseVar* var = findNode(names[i]);
        if (var != nullptr) {
            var->dbToVar();
        }
    }
    return 0;
}


//bool变量注册函数
bool cBaseVar::isValueBool(void* mem, cBaseVar* var)
{
    return true;
}
/*获取这个变量的输入类型*/
inputType cBaseVar::cBaseVar::getInputType()
{
    return inputTypeUser;
}
/*设置输入类型*/
int cBaseVar::setInputType(QString varname, inputType type0)
{
   cBaseVar *var = cBaseVar::findNode(varname);
   if (var == nullptr) {
       return -1;
   }
   var->inputTypeUser = type0;
   return 0;

}
QString cBaseVar::toStringBool(void* mem, cBaseVar* var)
{
    bool* value = (bool*)mem;
    if (*value) {
        return "1";
    }else {
        return "0";
    }
}
int cBaseVar::byStringBool(void* mem, QString str, cBaseVar* var)
{
    bool* value = (bool*)mem;
    bool ok = false;
    int vari = str.toInt(&ok);
    if (!ok) {
        return -1;
    }
    if (vari != 0) {
        *value = true;
    }else {
        *value = false;
    }
    return 0;
}
int cBaseVar::updateBool(void* src, void* dest, cBaseVar* var)
{
    memcpy(dest, src, sizeof(bool));
    return 0;
}


//char变量注册函数
bool cBaseVar::isValueChar(void* mem, cBaseVar* var)
{
    return true;
}
QString cBaseVar::toStringChar(void* mem, cBaseVar* var)
{
    char* value = (char*)mem;
    return QString::number(*value, var->base);

}
int cBaseVar::byStringChar(void* mem, QString str, cBaseVar* var)
{
    char* value = (char*)mem;
    bool ok = false;
    int vari = str.toInt(&ok);
    if (!ok) {
        return -1;
    }
    if (vari < 0 || vari > 255) {
        return -1;
    }
    *value = vari;
    return 0;
}
int cBaseVar::updateChar(void* src, void* dest, cBaseVar* var)
{
    memcpy(dest, src, sizeof(char));
    return 0;
}
//short变量注册函数
bool cBaseVar::isValueShort(void* mem, cBaseVar* var)
{
    return true;
}
QString cBaseVar::toStringShort(void* mem, cBaseVar* var)
{
    short* value = (short*)mem;
    return QString::number(*value, var->base);
}
int cBaseVar::byStringShort(void* mem, QString str, cBaseVar* var)
{
    short* value = (short*)mem;
    bool ok = false;
    short vari = str.toShort(&ok);
    if (!ok) {
        return -1;
    }
    *value = vari;
    return 0;
}
int cBaseVar::updateShort(void* src, void* dest, cBaseVar* var)
{
    memcpy(dest, src, sizeof(short));
    return 0;
}


//ushort变量注册函数
bool cBaseVar::isValueUShort(void* mem, cBaseVar* var)
{
    return true;
}
QString cBaseVar::toStringUShort(void* mem, cBaseVar* var)
{
    ushort* value = (ushort*)mem;
    return QString::number(*value, var->base);
}
int cBaseVar::byStringUShort(void* mem, QString str, cBaseVar* var)
{
    ushort* value = (ushort*)mem;
    bool ok = false;
    ushort vari = str.toUShort(&ok);
    if (!ok) {
        return -1;
    }
    *value = vari;
    return 0;
}
int cBaseVar::updateUShort(void* src, void* dest, cBaseVar* var)
{
    memcpy(dest, src, sizeof(ushort));
    return 0;
}
//int变量注册函数
bool cBaseVar::isValueInt(void* mem, cBaseVar* var)
{
    return true;
}
QString cBaseVar::toStringInt(void* mem, cBaseVar* var)
{
    int* value = (int*)mem;
    return QString::number(*value, var->base);
}
int cBaseVar::byStringInt(void* mem, QString str, cBaseVar* var)
{
    int* value = (int*)mem;
    bool ok = false;
    int vari = str.toInt(&ok);
    if (!ok) {
        return -1;
    }
    *value = vari;
    return 0;
}
int cBaseVar::updateInt(void* src, void* dest, cBaseVar* var)
{
    memcpy(dest, src, sizeof(int));
    return 0;
}


//uint变量注册函数
bool cBaseVar::isValueUInt(void* mem, cBaseVar* var)
{
    return true;
}
QString cBaseVar::toStringUInt(void* mem, cBaseVar* var)
{
    uint* value = (uint*)mem;
    return QString::number(*value, var->base);
}
int cBaseVar::byStringUInt(void* mem, QString str, cBaseVar* var)
{
    uint* value = (uint*)mem;
    bool ok = false;
    int vari = str.toUInt(&ok);
    if (!ok) {
        return -1;
    }
    *value = vari;
    return 0;
}
int cBaseVar::updateUInt(void* src, void* dest, cBaseVar* var)
{
    memcpy(dest, src, sizeof(uint));
    return 0;
}

//long变量注册函数
bool cBaseVar::isValueLong(void* mem, cBaseVar* var)
{
    return true;
}
QString cBaseVar::toStringLong(void* mem, cBaseVar* var)
{
    long* value = (long*)mem;
    return QString::number(*value, var->base);
}
int cBaseVar::byStringLong(void* mem, QString str, cBaseVar* var)
{
    long* value = (long*)mem;
    bool ok = false;
    int vari = str.toLong(&ok);
    if (!ok) {
        return -1;
    }
    *value = vari;
    return 0;
}
int cBaseVar::updateLong(void* src, void* dest, cBaseVar* var)
{
    memcpy(dest, src, sizeof(long));
    return 0;
}

//ulong变量注册函数
bool cBaseVar::isValueULong(void* mem, cBaseVar* var)
{
    return true;
}
QString cBaseVar::toStringULong(void* mem, cBaseVar* var)
{
    ulong* value = (ulong*)mem;
    return QString::number(*value, var->base);
}
int cBaseVar::byStringULong(void* mem, QString str, cBaseVar* var)
{
    ulong* value = (ulong*)mem;
    bool ok = false;
    int vari = str.toULong(&ok);
    if (!ok) {
        return -1;
    }
    *value = vari;
    return 0;
}
int cBaseVar::updateULong(void* src, void* dest, cBaseVar* var)
{
    memcpy(dest, src, sizeof(ulong));
    return 0;
}

//float变量注册函数
bool cBaseVar::isValueFloat(void* mem, cBaseVar* var)
{
    return true;
}
QString cBaseVar::toStringFloat(void* mem, cBaseVar* var)
{
    float* value = (float*)mem;
    return QString::number(*value, 'f',var->accr);
}
int cBaseVar::byStringFloat(void* mem, QString str, cBaseVar* var)
{
    float* value = (float*)mem;
    bool ok = false;
    float vari = str.toFloat(&ok);
    if (!ok) {
        return -1;
    }
    *value = vari;
    return 0;
}
int cBaseVar::updateFloat(void* src, void* dest, cBaseVar* var)
{
    memcpy(dest, src, sizeof(float));
    return 0;
}

//double变量注册函数
bool cBaseVar::isValueDouble(void* mem, cBaseVar* var)
{
    return true;
}
QString cBaseVar::toStringDouble(void* mem, cBaseVar* var)
{
    double* value = (double*)mem;
    return QString::number(*value, 'f', var->accr);
}
int cBaseVar::byStringDouble(void* mem, QString str, cBaseVar* var)
{
    double* value = (double*)mem;
    bool ok = false;
    double vari = str.toDouble(&ok);
    if (!ok) {
        return -1;
    }
    *value = vari;
    return 0;
}
int cBaseVar::updateDouble(void* src, void* dest, cBaseVar* var)
{
    memcpy(dest, src, sizeof(double));
    return 0;
}
//cqstruing变量注册函数
bool cBaseVar::isValueCQString(void* mem, cBaseVar* var)
{
    return false;
}
QString cBaseVar::toStringCQString(void* mem, cBaseVar* var)
{
    QString str = *(CQString*)mem;
    return str;
}
int cBaseVar::byStringCQString(void* mem, QString str, cBaseVar* var)
{
    *(CQString*)mem = str;
    return 0;
}
int cBaseVar::updateCQString(void* src, void* dest, cBaseVar* var)
{
    *(CQString*)src = *(CQString*)dest;
    return 0;
}

//qaction变量注册函数
bool cBaseVar::isValueQAction(void* mem, cBaseVar* var)
{
    return false;
}
QString cBaseVar::toStringQAction(void* mem, cBaseVar* var)
{
    return var->fullName;
}
int cBaseVar::byStringQAction(void* mem, QString str, cBaseVar* var)
{
    return -1;
}
int cBaseVar::updateQAction(void* src, void* dest, cBaseVar* var)
{
    return 0;
}
int cBaseVar::connectQAction(cBaseVar* src, cBaseVar* dest)
{
    if (src == nullptr || dest == nullptr) {
        return -1;
    }
    QAction* sig = (QAction*)src->realMem;
    QAction* slot = (QAction*)dest->realMem;
    QObject::connect(sig, &QAction::triggered, slot, &QAction::trigger, Qt::QueuedConnection);
    return 0;
}
int cBaseVar::disconnectQAction(cBaseVar* src, cBaseVar* dest)
{
    if (src == nullptr || dest == nullptr) {
        return -1;
    }
    QAction* sig = (QAction*)src->realMem;
    QAction* slot = (QAction*)dest->realMem;
    bool ok = disconnect(sig, &QAction::triggered, slot, &QAction::trigger);
    if (!ok) {
        return -1;
    }
    return 0;
}



///*通用update函数*/
//int cBaseVar::update0(void* src, void* dest, cBaseVar* var)
//{
//
//}
///*通用connect函数*/
//int cBaseVar::connect0(cBaseVar* src, cBaseVar* dest)
//{
//
//}
///*通用disconnect函数*/
//int cBaseVar::disconnect0(cBaseVar* src, cBaseVar* dest)
//{
//
//}


