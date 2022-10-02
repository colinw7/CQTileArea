#ifndef CQTileAreaConstants_H
#define CQTileAreaConstants_H

namespace CQTileAreaConstants {
  static const bool            debug_grid      = false;
  static const int             min_size        = 16;
  static const int             highlight_size  = 16;
  static const int             attach_timeout  = 10;
  static const QColor          bar_active_fg   = QColor(140, 140, 140);
  static const QColor          bar_inactive_fg = QColor(120, 120, 120);
  static const Qt::WindowFlags normalFlags     = Qt::Widget;
  static const Qt::WindowFlags floatingFlags   = Qt::Tool | Qt::FramelessWindowHint |
                                                 Qt::X11BypassWindowManagerHint;
  static const Qt::WindowFlags detachedFlags   = Qt::Tool | Qt::FramelessWindowHint;
}

#endif
