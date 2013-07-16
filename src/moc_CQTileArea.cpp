/****************************************************************************
** Meta object code from reading C++ file 'CQTileArea.h'
**
** Created: Tue Jul 16 07:21:47 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/CQTileArea.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CQTileArea.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CQTileArea[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       4,   64, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x05,
      48,   11,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      76,   11,   11,   11, 0x0a,
      94,   11,   11,   11, 0x0a,
     111,   11,   11,   11, 0x0a,
     125,   11,   11,   11, 0x0a,
     137,   11,   11,   11, 0x0a,
     148,   11,   11,   11, 0x0a,
     158,   11,   11,   11, 0x0a,
     170,   11,   11,   11, 0x0a,

 // properties: name, type, flags
     187,  183, 0x02095103,
     205,  200, 0x01095103,
     224,  217, 0x43095103,
     241,  217, 0x43095103,

       0        // eod
};

static const char qt_meta_stringdata_CQTileArea[] = {
    "CQTileArea\0\0currentWindowChanged(CQTileWindow*)\0"
    "windowClosed(CQTileWindow*)\0"
    "maximizeWindows()\0restoreWindows()\0"
    "tileWindows()\0printSlot()\0fillSlot()\0"
    "dupSlot()\0placeSlot()\0adjustSlot()\0"
    "int\0splitterSize\0bool\0animateDrag\0"
    "QColor\0titleActiveColor\0titleInactiveColor\0"
};

void CQTileArea::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CQTileArea *_t = static_cast<CQTileArea *>(_o);
        switch (_id) {
        case 0: _t->currentWindowChanged((*reinterpret_cast< CQTileWindow*(*)>(_a[1]))); break;
        case 1: _t->windowClosed((*reinterpret_cast< CQTileWindow*(*)>(_a[1]))); break;
        case 2: _t->maximizeWindows(); break;
        case 3: _t->restoreWindows(); break;
        case 4: _t->tileWindows(); break;
        case 5: _t->printSlot(); break;
        case 6: _t->fillSlot(); break;
        case 7: _t->dupSlot(); break;
        case 8: _t->placeSlot(); break;
        case 9: _t->adjustSlot(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData CQTileArea::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CQTileArea::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_CQTileArea,
      qt_meta_data_CQTileArea, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CQTileArea::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CQTileArea::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CQTileArea::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CQTileArea))
        return static_cast<void*>(const_cast< CQTileArea*>(this));
    return QWidget::qt_metacast(_clname);
}

int CQTileArea::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = splitterSize(); break;
        case 1: *reinterpret_cast< bool*>(_v) = animateDrag(); break;
        case 2: *reinterpret_cast< QColor*>(_v) = titleActiveColor(); break;
        case 3: *reinterpret_cast< QColor*>(_v) = titleInactiveColor(); break;
        }
        _id -= 4;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setSplitterSize(*reinterpret_cast< int*>(_v)); break;
        case 1: setAnimateDrag(*reinterpret_cast< bool*>(_v)); break;
        case 2: setTitleActiveColor(*reinterpret_cast< QColor*>(_v)); break;
        case 3: setTitleInactiveColor(*reinterpret_cast< QColor*>(_v)); break;
        }
        _id -= 4;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 4;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void CQTileArea::currentWindowChanged(CQTileWindow * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CQTileArea::windowClosed(CQTileWindow * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_CQTileWindowArea[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x0a,
      31,   17,   17,   17, 0x0a,
      44,   17,   17,   17, 0x0a,
      55,   17,   17,   17, 0x0a,
      70,   17,   17,   17, 0x0a,
      84,   17,   17,   17, 0x0a,
      96,   17,   17,   17, 0x08,
     123,  116,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_CQTileWindowArea[] = {
    "CQTileWindowArea\0\0detachSlot()\0"
    "attachSlot()\0tileSlot()\0maximizeSlot()\0"
    "restoreSlot()\0closeSlot()\0attachPreviewSlot()\0"
    "tabNum\0tabChangedSlot(int)\0"
};

void CQTileWindowArea::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CQTileWindowArea *_t = static_cast<CQTileWindowArea *>(_o);
        switch (_id) {
        case 0: _t->detachSlot(); break;
        case 1: _t->attachSlot(); break;
        case 2: _t->tileSlot(); break;
        case 3: _t->maximizeSlot(); break;
        case 4: _t->restoreSlot(); break;
        case 5: _t->closeSlot(); break;
        case 6: _t->attachPreviewSlot(); break;
        case 7: _t->tabChangedSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData CQTileWindowArea::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CQTileWindowArea::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_CQTileWindowArea,
      qt_meta_data_CQTileWindowArea, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CQTileWindowArea::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CQTileWindowArea::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CQTileWindowArea::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CQTileWindowArea))
        return static_cast<void*>(const_cast< CQTileWindowArea*>(this));
    return QFrame::qt_metacast(_clname);
}

int CQTileWindowArea::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}
static const uint qt_meta_data_CQTileWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      22,   14,   13,   13, 0x08,
      58,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_CQTileWindow[] = {
    "CQTileWindow\0\0old,now\0"
    "focusChangedSlot(QWidget*,QWidget*)\0"
    "widgetDestroyed()\0"
};

void CQTileWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CQTileWindow *_t = static_cast<CQTileWindow *>(_o);
        switch (_id) {
        case 0: _t->focusChangedSlot((*reinterpret_cast< QWidget*(*)>(_a[1])),(*reinterpret_cast< QWidget*(*)>(_a[2]))); break;
        case 1: _t->widgetDestroyed(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData CQTileWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CQTileWindow::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_CQTileWindow,
      qt_meta_data_CQTileWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CQTileWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CQTileWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CQTileWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CQTileWindow))
        return static_cast<void*>(const_cast< CQTileWindow*>(this));
    return QWidget::qt_metacast(_clname);
}

int CQTileWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
static const uint qt_meta_data_CQTileWindowTabBar[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_CQTileWindowTabBar[] = {
    "CQTileWindowTabBar\0"
};

void CQTileWindowTabBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData CQTileWindowTabBar::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CQTileWindowTabBar::staticMetaObject = {
    { &QTabBar::staticMetaObject, qt_meta_stringdata_CQTileWindowTabBar,
      qt_meta_data_CQTileWindowTabBar, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CQTileWindowTabBar::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CQTileWindowTabBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CQTileWindowTabBar::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CQTileWindowTabBar))
        return static_cast<void*>(const_cast< CQTileWindowTabBar*>(this));
    return QTabBar::qt_metacast(_clname);
}

int CQTileWindowTabBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTabBar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_CQTileWindowTitle[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x08,
      32,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_CQTileWindowTitle[] = {
    "CQTileWindowTitle\0\0detachSlot()\0"
    "maximizeSlot()\0"
};

void CQTileWindowTitle::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CQTileWindowTitle *_t = static_cast<CQTileWindowTitle *>(_o);
        switch (_id) {
        case 0: _t->detachSlot(); break;
        case 1: _t->maximizeSlot(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData CQTileWindowTitle::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CQTileWindowTitle::staticMetaObject = {
    { &CQTitleBar::staticMetaObject, qt_meta_stringdata_CQTileWindowTitle,
      qt_meta_data_CQTileWindowTitle, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CQTileWindowTitle::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CQTileWindowTitle::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CQTileWindowTitle::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CQTileWindowTitle))
        return static_cast<void*>(const_cast< CQTileWindowTitle*>(this));
    return CQTitleBar::qt_metacast(_clname);
}

int CQTileWindowTitle::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CQTitleBar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
static const uint qt_meta_data_CQTileStackedWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       2,   34, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      27,   21,   20,   20, 0x05,
      47,   21,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
      66,   21,   20,   20, 0x0a,
      89,   87,   20,   20, 0x0a,

 // properties: name, type, flags
     120,  116, 0x02495103,
     133,  116, 0x02095001,

 // properties: notify_signal_id
       0,
       0,

       0        // eod
};

static const char qt_meta_stringdata_CQTileStackedWidget[] = {
    "CQTileStackedWidget\0\0index\0"
    "currentChanged(int)\0widgetRemoved(int)\0"
    "setCurrentIndex(int)\0w\0"
    "setCurrentWidget(QWidget*)\0int\0"
    "currentIndex\0count\0"
};

void CQTileStackedWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CQTileStackedWidget *_t = static_cast<CQTileStackedWidget *>(_o);
        switch (_id) {
        case 0: _t->currentChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->widgetRemoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->setCurrentIndex((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->setCurrentWidget((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData CQTileStackedWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CQTileStackedWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_CQTileStackedWidget,
      qt_meta_data_CQTileStackedWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CQTileStackedWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CQTileStackedWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CQTileStackedWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CQTileStackedWidget))
        return static_cast<void*>(const_cast< CQTileStackedWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int CQTileStackedWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
#ifndef QT_NO_PROPERTIES
      else if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = currentIndex(); break;
        case 1: *reinterpret_cast< int*>(_v) = count(); break;
        }
        _id -= 2;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setCurrentIndex(*reinterpret_cast< int*>(_v)); break;
        }
        _id -= 2;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 2;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void CQTileStackedWidget::currentChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CQTileStackedWidget::widgetRemoved(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
