#include "fastkat/core.h"
#include <cassert>

#include "fastkat/ogremain.h"
#include "OGRE/OgreException.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"  // NOLINT this is how we include windows
#endif

void ShowMessageBox(const std::string& title, const std::string& message)
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
  MessageBox(NULL, message.c_str(),
    title.c_str(),
    MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
  std::cerr << title << ": " << message.c_str()
    << std::endl;
#endif
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
int main(int argc, char* argv[])
#endif
{
  try
  {
    OgreMain();
  }catch(Ogre::Exception &e)
  {
    ShowMessageBox("Ogre::Exception!!!!", e.what());
  }catch(std::exception &e)
  {
    ShowMessageBox("std::exception", e.what());
  }
  return 0;
}

