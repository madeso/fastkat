// Euphoria - Copyright (c) Gustav

#include <SDL.h>

#include <string>
#include <vector>
#include <cassert>
#include <memory>
#include <algorithm>
#include <iostream>

#ifdef WIN32
// unresolved external symbol _GetFileVersionInfoA@16 referenced in function
// _IME_GetId
#pragma comment(lib, "Version.lib")
// unresolved external symbol _ImmGetIMEFileNameA@12 referenced in function
// _IME_GetId
#pragma comment(lib, "Imm32.lib")
#endif

class Noncopyable {
public:
  Noncopyable() {}
private:
  Noncopyable(const Noncopyable& other);
  void operator=(const Noncopyable& other);
};

void Error(const std::string& title, const std::string& text) {
  const int result = SDL_ShowSimpleMessageBox(
      SDL_MESSAGEBOX_ERROR, title.c_str(), text.c_str(), NULL);
  if (result < 0) {
    std::cerr << "ERROR: " << title << "\n" << text << "\n";
  }
}

void ReportFail() {
  const std::string error = SDL_GetError();
  throw error;
}

void HandleStatus(int code) {
  if (code < 0) {
    ReportFail();
  }
}

class Sdl : Noncopyable {
 public:
  Sdl() {
    assert(this);
    const Uint32 flags = SDL_INIT_HAPTIC | SDL_INIT_JOYSTICK |
                                   SDL_INIT_VIDEO | SDL_INIT_EVENTS;
    HandleStatus(SDL_Init(flags));
  }

  ~Sdl() {
    assert(this);
    SDL_Quit();
  }
};

class VideoMode {
 public:
  explicit VideoMode(const SDL_DisplayMode& mode)
      : width_(mode.w),
        height_(mode.h),
        refreshRate_(mode.refresh_rate),
        pixel_format_(SDL_GetPixelFormatName(mode.format)) {
    assert(this);
  }

  int width() const {
    assert(this);
    return width_;
  }

  int height() const {
    assert(this);
    return height_;
  }

  int refreshRate() const {
    assert(this);
    return refreshRate_;
  }

  const std::string& pixel_format() const {
    assert(this);
    return pixel_format_;
  }

 private:
  int width_;
  int height_;
  int refreshRate_;
  std::string pixel_format_;
};

class DisplayInfo {
 public:
  explicit DisplayInfo(int id) {
    assert(this);
    SDL_Rect bounds;
    HandleStatus(SDL_GetDisplayBounds(id, &bounds));
    width_ = bounds.w;
    height_ = bounds.h;
    x_ = bounds.x;
    y_ = bounds.y;

    const char* name = SDL_GetDisplayName(id);
    if (name != 0) {
      name_ = name;
    }

    const int nummodes = SDL_GetNumDisplayModes(id);
    if (nummodes <= 0) {
      ReportFail();
      return;
    }
    for (int modeid = 0; modeid < nummodes; ++modeid) {
      SDL_DisplayMode mode;
      HandleStatus(SDL_GetDisplayMode(id, modeid, &mode));
      modes_.push_back(VideoMode(mode));
    }
  }

  int width() const {
    assert(this);
    return width_;
  }

  int height() const {
    assert(this);
    return height_;
  }

  int x() const {
    assert(this);
    return x_;
  }

  int y() const {
    assert(this);
    return y_;
  }

  const std::string& name() {
    assert(this);
    return name_;
  }

  const std::vector<VideoMode>& modes() const {
    assert(this);
    return modes_;
  }

 private:
  int width_;
  int height_;
  int x_;
  int y_;
  std::string name_;
  std::vector<VideoMode> modes_;
};

class VideoDisplays {
 public:
  VideoDisplays() {
    assert(this);
    const int numberofdisplays = SDL_GetNumVideoDisplays();
    if (numberofdisplays <= 0) {
      ReportFail();
      return;
    }
    for (int displayid = 0; displayid < numberofdisplays; ++displayid) {
      displays_.push_back(DisplayInfo(displayid));
    }
  }

  const std::vector<DisplayInfo>& displays() const {
    assert(this);
    return displays_;
  }

 private:
  std::vector<DisplayInfo> displays_;
};

class Window : Noncopyable {
 public:
  Window(const std::string& title, int x, int y, int width, int height,
         bool main)
      : window_(SDL_CreateWindow(
            title.c_str(), x, y, width, height,
            main ? (SDL_WINDOW_OPENGL | SDL_WINDOW_INPUT_FOCUS |
                    SDL_WINDOW_MOUSE_FOCUS)
                 : SDL_WINDOW_BORDERLESS)),
        width_(width),
        height_(height) {
    assert(this);
    if (window_ == NULL) {
      ReportFail();
    }
  }

  ~Window() {
    assert(this);
    if (window_) {
      SDL_DestroyWindow(window_);
      window_ = 0;
    }
  }

  int width() const { return width_; }

  int height() const { return height_; }

  SDL_Window* window() {
    assert(this);
    return window_;
  }

  void SetupDisplayModeToDesktop() {
    assert(this);
    assert(window_);
    int result = SDL_SetWindowDisplayMode(window_, NULL);
    if (result < 0) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                   "Failed to set display mode: %d", SDL_GetWindowID(window_));
    }
  }

  bool SetFullscreen(bool truefullscreen) {
    if (truefullscreen) {
      const int fullscreen =
          SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN);
      if (fullscreen < 0) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "Failed to set true full screen: %d - %s",
                    SDL_GetWindowID(window_), SDL_GetError());
        const int fullscreendesktop =
            SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
        if (fullscreendesktop < 0) {
          SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                      "Failed to set fake full screen: %d - %s",
                      SDL_GetWindowID(window_), SDL_GetError());
        }
        return fullscreendesktop >= 0;
      } else {
        return true;
      }
    } else {
      const int fullscreendesktop =
          SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
      if (fullscreendesktop < 0) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "Setting window fake fullscreen: %d - %s",
                    SDL_GetWindowID(window_), SDL_GetError());
      }
      return fullscreendesktop >= 0;
    }
  }

 private:
  SDL_Window* window_;
  int width_;
  int height_;
};

class BlackRenderer : Noncopyable {
 public:
  explicit BlackRenderer(Window* window)
      : renderer_(SDL_CreateRenderer(
            window->window(), -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)) {
    assert(this);
    if (renderer_ == NULL) {
      ReportFail();
    }
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
  }

  void Render() {
    assert(this);
    SDL_RenderClear(renderer_);
    SDL_RenderPresent(renderer_);
  }

  ~BlackRenderer() {
    assert(this);
    SDL_DestroyRenderer(renderer_);
  }

 private:
  SDL_Renderer* renderer_;
};

class Timer {
 public:
  Timer() : start_(0) { Reset(); }

  void Reset() { start_ = SDL_GetPerformanceCounter(); }

  template <typename F>
  F GetElapsedSeconds() const {
    const Uint64 now = SDL_GetPerformanceCounter();
    return static_cast<F>(now - start_) / SDL_GetPerformanceFrequency();
  }

 private:
  Uint64 start_;
};

typedef std::pair<int, int> Posi;
Posi GetHatValues(Uint8 hat) {
  switch (hat) {
    case SDL_HAT_LEFTUP:
      return Posi(-1, 1);
    case SDL_HAT_UP:
      return Posi(0, 1);
    case SDL_HAT_RIGHTUP:
      return Posi(1, 1);
    case SDL_HAT_LEFT:
      return Posi(-1, 0);
    case SDL_HAT_CENTERED:
      return Posi(0, 0);
    case SDL_HAT_RIGHT:
      return Posi(1, 0);
    case SDL_HAT_LEFTDOWN:
      return Posi(-1, -1);
    case SDL_HAT_DOWN:
      return Posi(0, -1);
    case SDL_HAT_RIGHTDOWN:
      return Posi(1, -1);
    default:
      assert(0 && "Invalid hat value");
      return Posi(0, 0);
  }
}

class Joystick {
 public:
  explicit Joystick(int id)
      : id_(id), joystick_(SDL_JoystickOpen(id)), numHats_(0) {
    if (joystick_ == NULL) {
      throw "Failed to open joystick";
    }

    numHats_ = SDL_JoystickNumHats(joystick_);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Opened joystick %s on %d: #axes: %d /"
                " #buttons: %d / #balls: %d / #hats: %d",
                SDL_JoystickNameForIndex(id_), id_,
                SDL_JoystickNumAxes(joystick_),
                SDL_JoystickNumButtons(joystick_),
                SDL_JoystickNumBalls(joystick_), numHats_);
  }

  ~Joystick() {
    if (IsAttached()) {
      SDL_JoystickClose(joystick_);
      joystick_ = NULL;
    }
  }

  bool IsAttached() const {
    return SDL_JoystickGetAttached(joystick_) == SDL_TRUE;
  }

 private:
  int id_;
  int numHats_;
  SDL_Joystick* joystick_;
};

class Context : Noncopyable {
 public:
  explicit Context(Window* window)
      : window_(window->window()),
        context_(SDL_GL_CreateContext(window->window())) {
    assert(this);
    assert(window);
    assert(window_);
    if (context_ == NULL) {
      ReportFail();
    }
  }

  ~Context() {
    assert(this);
    SDL_GL_DeleteContext(context_);
  }

  void Swap() { SDL_GL_SwapWindow(window_); }

  void MakeCurrent() {
    const int ret = SDL_GL_MakeCurrent(window_, context_);
    if (ret < 0) {
      SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                  "Failed changing the context: %d - %s",
                  SDL_GetWindowID(window_), SDL_GetError());
    }
  }

 private:
  SDL_Window* window_;
  SDL_GLContext context_;
};

enum class OculusVrDetection {
  Auto, Oculusvr, None
};

int FindOculusDisplay(const VideoDisplays& displays) {
  for (size_t i = 0; i < displays.displays().size(); ++i) {
    DisplayInfo di = displays.displays()[i];
    if (di.name() == "Rift DK1" || di.name() == "Rift DK" ||
        (di.width() == 1280 && di.height() == 720)) {
      return i;
    }
  }
  return -1;
}

int DeterminePrimaryDisplayId(const OculusVrDetection oculus_vr_detection, int displayid,
                              const VideoDisplays& displays,
                              bool* foundoculus) {
  assert(foundoculus);
  if (oculus_vr_detection == OculusVrDetection::Auto) {
    const int vrid = FindOculusDisplay(displays);
    if (vrid != -1) {
      *foundoculus = true;
      return vrid;
    } else {
      *foundoculus = false;
      return displayid;
    }
  } else {
    *foundoculus = oculus_vr_detection == OculusVrDetection::Oculusvr;
    return displayid;
  }
}

class Settings {
public:
  bool blackoutOther() { return false; }
  bool fullscreen() { return false; }
  int width() { return 800; };
  int height() { return 600; }
  OculusVrDetection vrdetection() { return OculusVrDetection::Oculusvr; }
  int displayId() { return 1; }
};

class Game {
public:
  Game(bool renderVr) : run(true) {}
  bool keep_running() { return run; }
  bool lock_mouse() { return false; }
  void Quit() {
    run = false;
  }
  void Render() {}
  void Update(float dt) {}

  void OnKeyboardKey(SDL_Keysym key, bool down) {}
private:
  bool run;
};

std::string GrabExceptionInformation() {
  try {
    throw;
  }
  catch (const std::string& str) {
    return str;
  }
  catch(const char* const str) {
    return str;
  }
  catch (...) {
    return "Unknown error";
  }
  return "No error?";
}

void logic() {
  SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
  Settings settings;

  Sdl sdl;
  SDL_DisableScreenSaver();
  VideoDisplays displays;

  std::shared_ptr<Window> primaryscreen;
  std::vector<std::shared_ptr<Window>> windows;
  std::vector<std::shared_ptr<BlackRenderer>> blacks;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                      SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
  // The core profile causes http://www.opengl.org/wiki/GLAPI/glGenVertexArrays
  // to crash, weird...

  bool renderoculus = false;
  

  int displayid = DeterminePrimaryDisplayId(settings.vrdetection(), settings.displayId(), displays, &renderoculus);

  for (size_t i = 0; i < displays.displays().size(); ++i) {
    const bool isprimaryscrreen = i == displayid;
    const bool createscreen = isprimaryscrreen || settings.blackoutOther();

    if (createscreen) {
      DisplayInfo di = displays.displays()[i];

      const int width = isprimaryscrreen == false || settings.width() == 0
                            ? di.width()
                            : settings.width();
      const int height = isprimaryscrreen == false || settings.height() == 0
                             ? di.height()
                             : settings.height();
      if (width > di.width()) {
        throw "Target width is too large";
      }
      if (height > di.height()) {
        throw "height is too large";
      }

      const int x = di.x() + (di.width() - width) / 2;
      const int y = di.y() + (di.height() - height) / 2;
      std::shared_ptr<Window> screen(
          new Window(isprimaryscrreen ? "Game" : "Black", x, y, width, height,
                     isprimaryscrreen));
      windows.push_back(screen);

      screen->SetupDisplayModeToDesktop();

      if (isprimaryscrreen) {
        primaryscreen = screen;
      } else {
        std::shared_ptr<BlackRenderer> br(new BlackRenderer(screen.get()));
        blacks.push_back(br);
      }
    }
  }

  if (renderoculus) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Renders with the oculus vr");
  } else {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Renders normally without vr support");
  }

  if (primaryscreen.get() == NULL) {
    throw "Unable to find primary screen";
  }

  Context context(primaryscreen.get());
  if (settings.fullscreen()) {
    /* Setting the black windows fullscreen messes with the primary fullscreen
    for(auto screen:windows) {
        if( screen != primaryscreen ){
            screen->SetFullscreen(false);
        }
    }*/
    primaryscreen->SetFullscreen(true);
  }

  std::vector<std::shared_ptr<Joystick>> joysticks;

  const int numberofjoysticks = SDL_NumJoysticks();
  for (int i = 0; i < numberofjoysticks; ++i) {
    const char* const name = SDL_JoystickNameForIndex(i);
    if (name == NULL) {
      ReportFail();
    }
    std::shared_ptr<Joystick> js(new Joystick(i));
    joysticks.push_back(js);
  }

  Timer timer;

  context.MakeCurrent();
  Game game(renderoculus);

  bool inside = false;

  while (game.keep_running()) {
    const float delta = timer.GetElapsedSeconds<float>();
    timer.Reset();
    game.Update(delta);

    for (auto black : blacks) {
      black->Render();
    }

    context.MakeCurrent();
    game.Render();
    context.Swap();

    SDL_Event event;

    const bool lock = inside && game.lock_mouse();
    SDL_SetRelativeMouseMode(lock ? SDL_TRUE : SDL_FALSE);

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        game.Quit();
      } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        const bool down = event.type == SDL_KEYDOWN;
        game.OnKeyboardKey(event.key.keysym, down);
      }
    }
  }
}

int main(int argc, char* argv[]) {
  try {
    logic();
    return 0;
  }
  catch (...) {
    const std::string message = GrabExceptionInformation();
    Error("Error!", message);
    return 1;
  }
}
