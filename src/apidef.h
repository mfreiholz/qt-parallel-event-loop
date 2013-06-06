#ifndef _QTPARALLELEVENTLOOP_APIDEF_HEADER_
#define _QTPARALLELEVENTLOOP_APIDEF_HEADER_

#include "QtCore/qglobal.h"

#if defined(QT_PEL_EXPORT)
#  define QT_PEL_API Q_DECL_EXPORT
#else
#  define QT_PEL_API Q_DECL_IMPORT
#endif

#endif
