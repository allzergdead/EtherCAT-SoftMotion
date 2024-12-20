#pragma once

#include <QObject>

#include <QHash>
#include <QAction>
#include <QDir>
//#include "../taskmanager/taskmanger.h"
// QT JSON
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

// QT SQL
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMutex>
/*
*	变量的最基础描述类，包含了变量的初始化等功能，所有框架构建的基础
*	支持以下类型：
*	untype
*	bool
*	char
*	byte
*	short
*	unsigned short
*	int 
*	unsigned int 
*	long
*	unsigned long
*	float
*	double
*	qaction
*	cimgstream
*	cqstring
*	
* 
*	参数本身可以有方向，主要方向为
*	unset
*	input
*	output
*	param
*/

/*后续会新增一个CQString用于多线程安全，现在没有，暂时用QString继续编写代码*/
#define CQString QString
#define CImgStream QJsonObject

class cBaseVar;
/*参数方向类型*/
enum cDIR
{
    C_UNSET,        /*参数方向类型*/  
    C_INPUT,        /*输入类型*/
    C_OUTPUT,       /*输出类型*/
    C_PARAM         /*参数类型，无法被输入也无法被绑定*/
};

/*参数值类型*/
enum cTYPE
{
    C_UNTYPE,
    C_BOOL,
    C_CHAR,
    C_BYTE,
    C_SHORT,
    C_USHORT,
    C_INT,
    C_UINT,
    C_LONG,
    C_ULONG,
    C_FLOAT,
    C_DOUBLE,
    C_QACTION,
    C_IMGSTREAM,
    C_CQSTRING
};
enum inputType 
{
    C_BTN,
    C_KEYBOARD_NO_DOT_SIGNED,
    C_KEYBOARD_NO_DOT_UNSIGNED,
    C_KEYBOARD_DOT,
    C_USER_SET,
    C_OPEN_FILE_DIR,
    C_SAVE_FILE_DIR,
    C_STRING_INPUT
};

struct cBaseFunc
{
    bool (*isValue)(void *mem , cBaseVar *var) = nullptr;
    QString (*toString)(void *mem, cBaseVar* var) = nullptr;
    int (*byString)(void* mem , QString str, cBaseVar* var) = nullptr;
    int (*update)(void* src, void* dest, cBaseVar* var) = nullptr;
    int (*connect)(cBaseVar* src, cBaseVar* dest) = nullptr;
    int (*disconnect)(cBaseVar* src, cBaseVar* dest) = nullptr;
    inputType inputType = C_USER_SET;
    int (*userSetInput)(cBaseVar* var) = nullptr;
};
enum infoType
{
    C_NORMAL_TYPE,
    C_SUCCESS_TYPE,
    C_ALARM_TYPE,
    C_ERROR_TYPE,
    C_ACTION_TYPE
};
struct logInfo
{
    QString title = "";
    QString info = "";
    QString actionName = "";
    infoType type = C_NORMAL_TYPE;
};

class cBaseVar  : public QObject
{
	Q_OBJECT
    
public:
    const std::type_info& type;
    template<typename T>
    cBaseVar(QObject* parent, T* mem)
        : QObject(parent),
        type(typeid(*mem)) // 存储传入对象的类型信息
    {
    }
	~cBaseVar();
    /*线程池*/
    static void* taskPool;
    static int addTask(void* lpStartAddress, void* lpParameter);
    /*原始数据地址*/
    void *realMem = nullptr;

    /*映射数据地址*/
    void* mem = nullptr;

    /*数据类型*/
    //cTYPE type = C_UNTYPE;
   

    /*数据方向*/
    cDIR dir = C_UNSET;

    /*数据方向*/
    CQString info = "";

    /*变量全称*/
    CQString fullName = "";

    /*变量类型*/
    CQString typeName = "";

    /*参数等级*/
    int level = 0;

    /*绑定变量*/
    CQString bindName = "";

    /*绑定标志*/
    bool bind = 0;
    ///*线程池类*/
    //static TaskManger taskPool;

    /*绑定标志*/
    bool retain = 0;

    /*线程池初始化标记*/
    bool taskPoolInitDone = false;

    /*线程池初始化*/
    int initTaskPool();

    /*数据结构判定类*/
    template<typename T>
    bool is(void) 
    {
        if (type == typeid(T)) {
            return true;
        }
        return false;
    }

    QString typeStr = "";
    //bool is(cTYPE type0);

    /*预显示进制*/
    int base = 10;

    /*预显示小数点精度*/
    int accr = 3;

    /*输入的最大值限制*/
    double max = 999999999;

    /*输入的最小值限制*/
    double min = -999999999;

    /*是否为空*/
    bool isNull(void);

    /*是否为 “值” 类型的数据*/
    bool isVALUE(void);

    /*链接变量*/
    int connectVar(QString var0);

    /*变量取消链接*/
    int disConnectVar();


    /*获取这个变量的输入类型*/
    inputType getInputType();

    /*设置输入类型*/
    static int setInputType(QString varname ,inputType type0);

    /*获取这个变量的输入类型*/
    inputType inputTypeUser = inputType::C_USER_SET;
    /*用于 "值" 类型数据如何显示的描述   小数点后显示几位*/
    //int precision = 3;

    /*用于 "值" 类型数据如何显示的描述   进制模式*/
    //int binary = 10;

    /*通用Json描述*/
    QJsonObject jsObj;

    /*将任意数据转换为QString输出*/
    QString toString(void);

    /*将任意数据由QString赋值*/
    int byString(QString str);

    /*检查是否可以访问*/
    bool checkLevel();

    /*参数树HASH描述*/
    static QMutex hashMtx;
    QHash<QString, cBaseVar *> cHash;

    /*类型行为树Hash表*/
    static QHash<QString, cBaseFunc> cTypeFunc;

    /*注册类型行为*/
    template<typename T>
    static int regTypeFunc(cBaseFunc func)
    {
        QString typeStr0 = typeid(T).name();
        cBaseVar::cTypeFunc[typeStr0] = func;
        return 0;
    }



    /*返回类型值*/
    template<typename T>
    T to(bool* ok)
    {
        T value;
        if (typeid(T) == type) {
            value = *(T*)mem;
            *ok = true;
            return value;
        } else {
            *ok = false;
            return value;
        }
    }
    /*提前存储类型行为*/
    cBaseFunc *func = nullptr;

    /**
     * 插入新节点并返回指针。进行查重，如果路径已存在或插入失败，返回空指针。
     * 如果中间节点不存在，会自动创建这些节点。
     *
     * @param path 要插入的节点的路径，用"."分割。
     * @param var 要插入的节点的数据。
     * @return 插入的节点的指针，如果节点已存在或插入失败，则为nullptr。
     */
    static cBaseVar* insertNode(const QString& path, cBaseVar *var); 
    /**
     * 寻找指定路径的节点并返回节点指针。
     * 使用 "." 作为分隔符来分割路径的各个部分。
     *
     * @param path 要查找的节点的路径，用 "." 分割。
     * @return 如果找到了指定路径的节点，则返回指向该节点的指针；
     *         如果路径中的任何部分不存在，则返回 nullptr。
     */
    static cBaseVar* findNode(const QString& path);
    static int rootValue;

    static bool bInited;

 
    //动态日志
    /*装填日志*/
    static QMutex logMtx;
    static QVector<logInfo> logVec;
 
    static int insertLog(infoType type, QString title, QString info);
    static int insertLog(infoType type,QString title,QString info,QString actionName);
    // 数据库操作
    /*数据库*/
    static QSqlDatabase db;
    static QMutex dbMtx;
    /*打开数据库*/
    static int openDB(QString path);
    /*创建数据库*/
    static int newDB(QString path);
    /*软件启动初始化数据库*/
    static int connectDB();
    /*单个数据存入数据库*/
    int varToDB();
    static void *exVarToDB(void *var0);
    /*整体存入数据库*/
    static int allVarToDB();
    /*单个从数据库读取*/
    int dbToVar();
    static void* exDBToVar(void* var0);
    /*整体存入数据库*/
    static int allDBToVar();
 

    /*列表搜索*/
    static QVector<QString> collectNames(const QString& searchStr);
    static void collectNamesRecursive(const QHash<QString, cBaseVar*>& hash, const QString& searchStr, QVector<QString>& names);

    /*通用connect函数*/

    static int connect0(cBaseVar* src, cBaseVar* dest);
    /*通用disconnect函数*/

    static int disconnect0(cBaseVar* src, cBaseVar* dest);

    /*bool变量注册函数*/
    static bool isValueBool(void* mem, cBaseVar* var);
    static QString toStringBool(void* mem, cBaseVar* var);
    static int byStringBool(void* mem, QString str, cBaseVar* var);
    static int updateBool(void* src, void* dest, cBaseVar* var);

     
    /*char变量注册函数*/
    static bool isValueChar(void* mem, cBaseVar* var);
    static QString toStringChar(void* mem, cBaseVar* var);
    static int byStringChar(void* mem, QString str, cBaseVar* var);
    static int updateChar(void* src, void* dest, cBaseVar* var);


    /*short变量注册函数*/
    static bool isValueShort(void* mem, cBaseVar* var);
    static QString toStringShort(void* mem, cBaseVar* var);
    static int byStringShort(void* mem, QString str, cBaseVar* var);
    static int updateShort(void* src, void* dest, cBaseVar* var);
    /*ushort变量注册函数*/
    static bool isValueUShort(void* mem, cBaseVar* var);
    static QString toStringUShort(void* mem, cBaseVar* var);
    static int byStringUShort(void* mem, QString str, cBaseVar* var);
    static int updateUShort(void* src, void* dest, cBaseVar* var);
    /*int变量注册函数*/
    static bool isValueInt(void* mem, cBaseVar* var);
    static QString toStringInt(void* mem, cBaseVar* var);
    static int byStringInt(void* mem, QString str, cBaseVar* var);
    static int updateInt(void* src, void* dest, cBaseVar* var);
    /*uint变量注册函数*/
    static bool isValueUInt(void* mem, cBaseVar* var);
    static QString toStringUInt(void* mem, cBaseVar* var);
    static int byStringUInt(void* mem, QString str, cBaseVar* var);
    static int updateUInt(void* src, void* dest, cBaseVar* var);
    /*long变量注册函数*/
    static bool isValueLong(void* mem, cBaseVar* var);
    static QString toStringLong(void* mem, cBaseVar* var);
    static int byStringLong(void* mem, QString str, cBaseVar* var);
    static int updateLong(void* src, void* dest, cBaseVar* var);
    /*ulong变量注册函数*/
    static bool isValueULong(void* mem, cBaseVar* var);
    static QString toStringULong(void* mem, cBaseVar* var);
    static int byStringULong(void* mem, QString str, cBaseVar* var);
    static int updateULong(void* src, void* dest, cBaseVar* var);
    /*float变量注册函数*/
    static bool isValueFloat(void* mem, cBaseVar* var);
    static QString toStringFloat(void* mem, cBaseVar* var);
    static int byStringFloat(void* mem, QString str, cBaseVar* var);
    static int updateFloat(void* src, void* dest, cBaseVar* var);
    /*double变量注册函数*/
    static bool isValueDouble(void* mem, cBaseVar* var);
    static QString toStringDouble(void* mem, cBaseVar* var);
    static int byStringDouble(void* mem, QString str, cBaseVar* var);
    static int updateDouble(void* src, void* dest, cBaseVar* var);
    /*cqstruing变量注册函数*/
    static bool isValueCQString(void* mem, cBaseVar* var);
    static QString toStringCQString(void* mem, cBaseVar* var);
    static int byStringCQString(void* mem, QString str, cBaseVar* var);
    static int updateCQString(void* src, void* dest, cBaseVar* var);
    /*qaction变量注册函数*/
    static bool isValueQAction(void* mem, cBaseVar* var);
    static QString toStringQAction(void* mem, cBaseVar* var);
    static int byStringQAction(void* mem, QString str, cBaseVar* var);
    static int updateQAction(void* src, void* dest, cBaseVar* var);
    static int connectQAction(cBaseVar* src, cBaseVar* dest);
    static int disconnectQAction(cBaseVar* src, cBaseVar* dest);
signals:
    void varChanged();
};
extern cBaseVar rootVar;
/**
 * 各种数据类型的注册函数，自动往hash表里面添加代码
 *
 * @mem 注册进来的函数的指针
 */
template<typename T>
cBaseVar* newCVar(T* mem, QString name, cDIR dir, int level, QString info)
{
    //注册基础数据类型
    if (!cBaseVar::bInited) {
        cBaseFunc tempFunc;
        //bool变量注册函数
        tempFunc.byString = cBaseVar::byStringBool;
        tempFunc.isValue = cBaseVar::isValueBool;
        tempFunc.toString = cBaseVar::toStringBool;
        tempFunc.update = cBaseVar::updateBool;
        tempFunc.inputType = inputType::C_BTN;
        cBaseVar::regTypeFunc<bool>(tempFunc);

        //char变量注册函数
        tempFunc.byString = cBaseVar::byStringChar;
        tempFunc.isValue = cBaseVar::isValueChar;
        tempFunc.toString = cBaseVar::toStringChar;
        tempFunc.update = cBaseVar::updateChar;
        tempFunc.inputType = inputType::C_KEYBOARD_NO_DOT_UNSIGNED;
        cBaseVar::regTypeFunc<char>(tempFunc);

        //short变量注册函数
        tempFunc.byString = cBaseVar::byStringShort;
        tempFunc.isValue = cBaseVar::isValueShort;
        tempFunc.toString = cBaseVar::toStringShort;
        tempFunc.update = cBaseVar::updateShort;
        tempFunc.inputType = inputType::C_KEYBOARD_NO_DOT_SIGNED;
        cBaseVar::regTypeFunc<short>(tempFunc);

        //ushort变量注册函数
        tempFunc.byString = cBaseVar::byStringUShort;
        tempFunc.isValue = cBaseVar::isValueUShort;
        tempFunc.toString = cBaseVar::toStringUShort;
        tempFunc.update = cBaseVar::updateUShort;
        tempFunc.inputType = inputType::C_KEYBOARD_NO_DOT_UNSIGNED;
        cBaseVar::regTypeFunc<ushort>(tempFunc);

        //int变量注册函数
        tempFunc.byString = cBaseVar::byStringInt;
        tempFunc.isValue = cBaseVar::isValueInt;
        tempFunc.toString = cBaseVar::toStringInt;
        tempFunc.update = cBaseVar::updateInt;
        tempFunc.inputType = inputType::C_KEYBOARD_NO_DOT_SIGNED;
        cBaseVar::regTypeFunc<int>(tempFunc);


        //uint变量注册函数
        tempFunc.byString = cBaseVar::byStringUInt;
        tempFunc.isValue = cBaseVar::isValueUInt;
        tempFunc.toString = cBaseVar::toStringUInt;
        tempFunc.update = cBaseVar::updateUInt;
        tempFunc.inputType = inputType::C_KEYBOARD_NO_DOT_UNSIGNED;
        cBaseVar::regTypeFunc<uint>(tempFunc);

        //long变量注册函数
        tempFunc.byString = cBaseVar::byStringLong;
        tempFunc.isValue = cBaseVar::isValueLong;
        tempFunc.toString = cBaseVar::toStringLong;
        tempFunc.update = cBaseVar::updateLong;
        tempFunc.inputType = inputType::C_KEYBOARD_NO_DOT_SIGNED;
        cBaseVar::regTypeFunc<long>(tempFunc);

        //ulong变量注册函数
        tempFunc.byString = cBaseVar::byStringULong;
        tempFunc.isValue = cBaseVar::isValueULong;
        tempFunc.toString = cBaseVar::toStringULong;
        tempFunc.update = cBaseVar::updateULong;
        tempFunc.inputType = inputType::C_KEYBOARD_NO_DOT_UNSIGNED;
        cBaseVar::regTypeFunc<ulong>(tempFunc);

        //float变量注册函数
        tempFunc.byString = cBaseVar::byStringFloat;
        tempFunc.isValue = cBaseVar::isValueFloat;
        tempFunc.toString = cBaseVar::toStringFloat;
        tempFunc.update = cBaseVar::updateFloat;
        tempFunc.inputType = inputType::C_KEYBOARD_DOT;
        cBaseVar::regTypeFunc<float>(tempFunc);

        //double变量注册函数
        tempFunc.byString = cBaseVar::byStringDouble;
        tempFunc.isValue = cBaseVar::isValueDouble;
        tempFunc.toString = cBaseVar::toStringDouble;
        tempFunc.update = cBaseVar::updateDouble;
        tempFunc.inputType = inputType::C_KEYBOARD_DOT;
        cBaseVar::regTypeFunc<double>(tempFunc);

        //cqstring变量注册函数
        tempFunc.byString = cBaseVar::byStringCQString;
        tempFunc.isValue = cBaseVar::isValueCQString;
        tempFunc.toString = cBaseVar::toStringCQString;
        tempFunc.update = cBaseVar::updateCQString;
        tempFunc.inputType = inputType::C_STRING_INPUT;
        cBaseVar::regTypeFunc<CQString>(tempFunc);

        //qaction变量注册函数
        cBaseFunc qactionFunc;
        qactionFunc.byString = cBaseVar::byStringQAction;
        qactionFunc.isValue = cBaseVar::isValueQAction;
        qactionFunc.toString = cBaseVar::toStringQAction;
        qactionFunc.update = cBaseVar::updateQAction;
        qactionFunc.connect = cBaseVar::connectQAction;
        qactionFunc.disconnect = cBaseVar::disconnectQAction;
        tempFunc.inputType = inputType::C_USER_SET;
        cBaseVar::regTypeFunc<QAction>(qactionFunc);

        cBaseVar::bInited = true;

    }
    //cBaseVar var(nullptr, mem);
    QString typeStr0 = typeid(T).name();
    if (cBaseVar::cTypeFunc.find(typeStr0) == cBaseVar::cTypeFunc.end()) {
        return nullptr;
    }

    cBaseVar* var = new cBaseVar(nullptr, mem);
    cBaseVar* curVar = cBaseVar::insertNode(name, var);
    if (curVar == nullptr) {
        return nullptr;
    }
    curVar->fullName = name;
    curVar->dir = dir;
    curVar->level = level;
    curVar->info = info;
    curVar->realMem = (void*)mem;
    curVar->mem = (void*)mem;
    curVar->func = &cBaseVar::cTypeFunc[typeStr0];
    curVar->typeName = curVar->type.name();
    curVar->inputTypeUser = curVar->func->inputType;
    return curVar;
}





//











































