// fastkat - Copyright (c) Omiod, port by Gustav

#include <cstdint>
#include <memory>
#include <exception>

#include "OGRE/OgreRoot.h"
#include "OGRE/OgreRenderSystem.h"
#include "OGRE/OgreRenderWindow.h"
#include "OGRE/OgreWindowEventUtilities.h"
#include "OGRE/OgreManualObject.h"
#include "OGRE/OgreEntity.h"

#include "fastkat/core.h"
#include "fastkat/fastkat.h"

#include "OIS/Ois.h"

// use the gui instead?
std::auto_ptr<Ogre::Root> InitOgre(Ogre::String window_title,
                                   Ogre::RenderWindow** window) {
  std::auto_ptr<Ogre::Root> root;

  Ogre::String config_file_name = "";
  Ogre::String plugins_file_name = "";
  Ogre::String log_file_name = "Ogre.log";

  root.reset(
      new Ogre::Root(config_file_name, plugins_file_name, log_file_name));
  {
    typedef std::vector<Ogre::String> Strings;

    Strings plugin_names;
    plugin_names.push_back("RenderSystem_GL");
    // lPluginNames.push_back("RenderSystem_Direct3D9");
    plugin_names.push_back("Plugin_ParticleFX");
    plugin_names.push_back("Plugin_CgProgramManager");
    // lPluginNames.push_back("Plugin_PCZSceneManager");
    // lPluginNames.push_back("Plugin_OctreeZone");
    plugin_names.push_back("Plugin_OctreeSceneManager");
    // lPluginNames.push_back("Plugin_BSPSceneManager");

    {
      for (Strings::iterator plugin_iterator = plugin_names.begin();
           plugin_iterator != plugin_names.end(); plugin_iterator++) {
        Ogre::String& plugin_name = (*plugin_iterator);
        bool is_in_debug_mode = OGRE_DEBUG_MODE;
        if (is_in_debug_mode) {
          plugin_name.append("_d");
        }
        root->loadPlugin(plugin_name);
      }
    }
  }

  {
    const Ogre::RenderSystemList& render_system_list =
        root->getAvailableRenderers();
    if (render_system_list.size() == 0) {
      throw std::exception("Sorry, no render system was found.");
    }

    Ogre::RenderSystem* render_system = render_system_list.at(0);
    root->setRenderSystem(render_system);
  }

  {
    bool create_window_automatically = false;
    Ogre::String window_title = "";
    Ogre::String custom_capacities = "";
    root->initialise(create_window_automatically, window_title,
                     custom_capacities);
  }

  unsigned int window_width = 800;
  unsigned int window_height = 600;
  bool is_fullscreen = false;
  Ogre::NameValuePairList params;
  params["FSAA"] = "0";
  params["vsync"] = "true";
  *window = root->createRenderWindow(window_title, window_width, window_height,
                                     is_fullscreen, &params);

  return root;
}

void OgreMain() {
  Ogre::RenderWindow* window;
  std::auto_ptr<Ogre::Root> root = InitOgre("fastkat", &window);

  OIS::InputManager* input_manager = NULL;
  OIS::Mouse* mouse = NULL;
  OIS::Keyboard* keyboard = NULL;
  {
    size_t window_handle = 0;
    window->getCustomAttribute("WINDOW", &window_handle);

    std::string window_handle_as_string = "";
    {
      std::ostringstream windowHndStr;
      windowHndStr << window_handle;
      window_handle_as_string = windowHndStr.str();
    }

    OIS::ParamList special_parameters;
    special_parameters.insert(
        std::make_pair(std::string("WINDOW"), window_handle_as_string));

    input_manager = OIS::InputManager::createInputSystem(special_parameters);

    bool buffered_keys = false;
    bool buffered_mouse = false;

    keyboard = static_cast<OIS::Keyboard*>(
        input_manager->createInputObject(OIS::OISKeyboard, buffered_keys));

    mouse = static_cast<OIS::Mouse*>(
        input_manager->createInputObject(OIS::OISMouse, buffered_mouse));

    const OIS::MouseState& mouse_state = mouse->getMouseState();
    mouse_state.width = window->getWidth();
    mouse_state.height = window->getHeight();
  }

  Ogre::SceneManager* scene =
      root->createSceneManager(Ogre::ST_GENERIC, "MyFirstSceneManager");

  Ogre::SceneNode* root_scene_node = scene->getRootSceneNode();
  Ogre::Camera* camera = scene->createCamera("MyFirstCamera");
  Ogre::SceneNode* camera_node =
      root_scene_node->createChildSceneNode("MyFirstCameraNode");
  camera_node->attachObject(camera);

  float viewport_width = 1.0f;
  float viewport_height = 1.0f;
  float viewport_left = (1.0f - viewport_width) * 0.5f;
  float viewport_top = (1.0f - viewport_height) * 0.5f;
  int main_viewport_zorder = 100;
  Ogre::Viewport* viewport =
      window->addViewport(camera, main_viewport_zorder, viewport_left,
                          viewport_top, viewport_width, viewport_height);
  viewport->setAutoUpdated(true);
  viewport->setBackgroundColour(Ogre::ColourValue(0, 0, 0));

  float ratio = static_cast<float>(viewport->getActualWidth()) /
                static_cast<float>(viewport->getActualHeight());
  camera->setAspectRatio(ratio);
  camera->setNearClipDistance(1.5f);
  camera->setFarClipDistance(3000.0f);
  window->setActive(true);
  window->setAutoUpdated(false);

  Fastkat fastkat;

  Ogre::String resource_group_name = "resourcegroup";
  {
    Ogre::ResourceGroupManager& resource_group_manager =
        Ogre::ResourceGroupManager::getSingleton();
    resource_group_manager.createResourceGroup(resource_group_name);

    Ogre::String directory_to_load = "../media/models";
    bool is_recursive = false;
    resource_group_manager.addResourceLocation(
        directory_to_load, "FileSystem", resource_group_name, is_recursive);
    resource_group_manager.initialiseResourceGroup(resource_group_name);
    resource_group_manager.loadResourceGroup(resource_group_name);

    fastkat.load(root_scene_node, scene);
  }

  root->clearEventTimes();
  Ogre::Timer* timer = root->getTimer();
  timer->reset();

  int64_t last_time_ms = timer->getMilliseconds();
  int64_t delta_time_ms = 0;
  float delta_time = 0.0f;

  while (!window->isClosed()) {
    int64_t current_time_ms = timer->getMilliseconds();
    delta_time_ms = current_time_ms - last_time_ms;
    if (delta_time_ms == 0) {
      continue;
    }
    last_time_ms = current_time_ms;
    delta_time = 0.001f * delta_time_ms;

    keyboard->capture();
    mouse->capture();

    if (fastkat.update(delta_time, camera_node, keyboard, mouse,
                       static_cast<float>(window->getWidth()),
                       static_cast<float>(window->getHeight()),
                       camera) == false) {
      break;
    }

    window->update(false);
    window->swapBuffers();

    root->renderOneFrame();

    Ogre::WindowEventUtilities::messagePump();
  }

  input_manager->destroyInputObject(keyboard);
  keyboard = NULL;
  input_manager->destroyInputObject(mouse);
  mouse = NULL;
  OIS::InputManager::destroyInputSystem(input_manager);
  input_manager = NULL;

  window->removeAllViewports();
  scene->destroyAllCameras();
  scene->destroyAllManualObjects();
  scene->destroyAllEntities();
  root_scene_node->removeAndDestroyAllChildren();

  Ogre::ResourceGroupManager& resource_group_manager =
      Ogre::ResourceGroupManager::getSingleton();
  resource_group_manager.destroyResourceGroup(resource_group_name);

  return;
}
