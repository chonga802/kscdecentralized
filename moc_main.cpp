/****************************************************************************
** Meta object code from reading C++ file 'main.hh'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "main.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'main.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_InitDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_InitDialog[] = {
    "InitDialog\0\0loadID()\0"
};

void InitDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        InitDialog *_t = static_cast<InitDialog *>(_o);
        switch (_id) {
        case 0: _t->loadID(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData InitDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject InitDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_InitDialog,
      qt_meta_data_InitDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &InitDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *InitDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *InitDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_InitDialog))
        return static_cast<void*>(const_cast< InitDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int InitDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_ChatDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      20,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x0a,
      31,   11,   11,   11, 0x0a,
      49,   11,   11,   11, 0x0a,
      67,   11,   11,   11, 0x0a,
      84,   11,   11,   11, 0x0a,
      98,   11,   11,   11, 0x0a,
     119,   11,   11,   11, 0x0a,
     131,   11,   11,   11, 0x0a,
     147,   11,   11,   11, 0x0a,
     159,   11,   11,   11, 0x0a,
     191,   11,   11,   11, 0x0a,
     207,   11,   11,   11, 0x0a,
     250,  240,   11,   11, 0x0a,
     286,   11,   11,   11, 0x0a,
     308,   11,   11,   11, 0x0a,
     327,  323,   11,   11, 0x0a,
     353,   11,   11,   11, 0x0a,
     371,  369,   11,   11, 0x0a,
     402,   11,   11,   11, 0x0a,
     435,  323,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ChatDialog[] = {
    "ChatDialog\0\0gotReturnPressed()\0"
    "incomingMessage()\0timedOutWaiting()\0"
    "preventEntropy()\0processPeer()\0"
    "checkInfo(QHostInfo)\0sendRoute()\0"
    "sendBroadcast()\0shareFile()\0"
    "addFilesForSharing(QStringList)\0"
    "resendRequest()\0privateMessage(QListWidgetItem*)\0"
    "msg,buddy\0sendPrivateMessage(QString,QString)\0"
    "createSearchRequest()\0resendSearch()\0"
    "msg\0beginTorrent(QVariantMap)\0"
    "startDownload()\0,\0startDownload(QString,QString)\0"
    "requestSeeders(QListWidgetItem*)\0"
    "replySeeders(QVariantMap)\0"
};

void ChatDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ChatDialog *_t = static_cast<ChatDialog *>(_o);
        switch (_id) {
        case 0: _t->gotReturnPressed(); break;
        case 1: _t->incomingMessage(); break;
        case 2: _t->timedOutWaiting(); break;
        case 3: _t->preventEntropy(); break;
        case 4: _t->processPeer(); break;
        case 5: _t->checkInfo((*reinterpret_cast< QHostInfo(*)>(_a[1]))); break;
        case 6: _t->sendRoute(); break;
        case 7: _t->sendBroadcast(); break;
        case 8: _t->shareFile(); break;
        case 9: _t->addFilesForSharing((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 10: _t->resendRequest(); break;
        case 11: _t->privateMessage((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 12: _t->sendPrivateMessage((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 13: _t->createSearchRequest(); break;
        case 14: _t->resendSearch(); break;
        case 15: _t->beginTorrent((*reinterpret_cast< QVariantMap(*)>(_a[1]))); break;
        case 16: _t->startDownload(); break;
        case 17: _t->startDownload((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 18: _t->requestSeeders((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 19: _t->replySeeders((*reinterpret_cast< QVariantMap(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ChatDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ChatDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_ChatDialog,
      qt_meta_data_ChatDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ChatDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ChatDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ChatDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ChatDialog))
        return static_cast<void*>(const_cast< ChatDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int ChatDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
