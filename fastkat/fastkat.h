// fastkat - Copyright (c) Omiod, port by Gustav

#ifndef FASTKAT_FASTKAT_H_
#define FASTKAT_FASTKAT_H_

#include <string>

namespace Ogre {
class SceneNode;
class SceneManager;
class Camera;
}

namespace OIS {
class Keyboard;
class Mouse;
};

class Fastkat {
 public:
  Fastkat();

  void load(Ogre::SceneNode* root_scene_node, Ogre::SceneManager* scene,
            Ogre::Camera* camera);
  bool update(float delta_time, Ogre::SceneNode* camera_node,
              OIS::Keyboard* keyboard, OIS::Mouse* mouse, float window_width,
              float window_height, Ogre::Camera* camera);
};

#endif  // FASTKAT_FASTKAT_H_
