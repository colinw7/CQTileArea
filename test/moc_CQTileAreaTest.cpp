/****************************************************************************
** Meta object code from reading C++ file 'CQTileAreaTest.h'
**
** Created: Tue Jul 16 07:21:55 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "CQTileAreaTest.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CQTileAreaTest.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CQTileAreaTest[] = {

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
      16,   15,   15,   15, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_CQTileAreaTest[] = {
    "CQTileAreaTest\0\0addWindow()\0"
};

void CQTileAreaTest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CQTileAreaTest *_t = static_cast<CQTileAreaTest *>(_o);
        switch (_id) {
        case 0: _t->addWindow(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData CQTileAreaTest::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CQTileAreaTest::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_CQTileAreaTest,
      qt_meta_data_CQTileAreaTest, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CQTileAreaTest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CQTileAreaTest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CQTileAreaTest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CQTileAreaTest))
        return static_cast<void*>(const_cast< CQTileAreaTest*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int CQTileAreaTest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
