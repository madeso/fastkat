// fastkat - Copyright (c) Omiod, port by Gustav

#ifndef FASTKAT_TUTORIALAPPLICATION_H_
#define FASTKAT_TUTORIALAPPLICATION_H_

#include "fastkat/BaseApplication.h"

class TutorialApplication : public BaseApplication {
 public:
  TutorialApplication(void);
  virtual ~TutorialApplication(void);

 protected:
  virtual void createScene(void);
};

#endif  // FASTKAT_TUTORIALAPPLICATION_H_
