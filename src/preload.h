#pragma once

#include <memory>

#include "log.h" // defines AFTER_HOURS_REPLACE_LOGGING before any afterhours header
#include <afterhours/src/library.h>
#include <afterhours/src/singleton.h>

SINGLETON_FWD(Preload)
struct Preload {
  SINGLETON(Preload)

  Preload();
  ~Preload();

  Preload(const Preload &) = delete;
  void operator=(const Preload &) = delete;

  Preload &init(const char *title);
  Preload &make_singleton();
};
