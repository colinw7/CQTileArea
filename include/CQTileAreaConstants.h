#ifndef CQTileAreaConstants_H
#define CQTileAreaConstants_H

namespace CQTileAreaConstants {
  static bool            debug_grid      = false;
  static int             min_size        = 16;
  static int             highlight_size  = 16;
  static int             attach_timeout  = 10;
  static QColor          bar_active_fg   = QColor(140,140,140);
  static QColor          bar_inactive_fg = QColor(120,120,120);
  static Qt::WindowFlags normalFlags     = Qt::Widget;
  static Qt::WindowFlags floatingFlags   = Qt::Tool | Qt::FramelessWindowHint |
                                           Qt::X11BypassWindowManagerHint;
  static Qt::WindowFlags detachedFlags   = Qt::Tool | Qt::FramelessWindowHint;
}

#endif
