// fastkat - Copyright (c) Omiod, port by Gustav

#ifndef FASTKAT_FASTKATAPPLICATION_H_
#define FASTKAT_FASTKATAPPLICATION_H_

#include "fastkat/BaseApplication.h"

class FastkatApplication : public BaseApplication {
 public:
  FastkatApplication(void);
  virtual ~FastkatApplication(void);

 protected:
  virtual void createScene(void);
};

#endif  // FASTKAT_FASTKATAPPLICATION_H_
