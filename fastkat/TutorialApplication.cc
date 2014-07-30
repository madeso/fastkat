#include "fastkat/TutorialApplication.h"

TutorialApplication::TutorialApplication(void) {}

TutorialApplication::~TutorialApplication(void) {}

void TutorialApplication::createScene(void) {
  // create your scene here :)
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"  // NOLINT this is how we include windows
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
int main(int argc, char* argv[])
#endif
{
  // Create application object
  TutorialApplication app;

  try {
    app.go();
  }
  catch (const Ogre::Exception& e) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    MessageBox(NULL, e.getFullDescription().c_str(),
               "An exception has occured!",
               MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
    std::cerr << "An exception has occured: " << e.getFullDescription().c_str()
              << std::endl;
#endif
  }

  return 0;
}

#ifdef __cplusplus
}
#endif
