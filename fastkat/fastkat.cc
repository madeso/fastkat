// fastkat - Copyright (c) Omiod, port by Gustav

#include "fastkat/fastkat.h"

#include "OGRE/OgreString.h"
#include "OGRE/OgreSceneManager.h"
#include "OGRE/OgreSceneNode.h"
#include "OGRE/OgreEntity.h"
#include "OGRE/OgreLight.h"

#include "OIS/Ois.h"

Fastkat::Fastkat() {}

void Fastkat::load(Ogre::SceneNode* root_scene_node,
                   Ogre::SceneManager* scene) {
  Ogre::String mesh_name = "ogrehead.mesh";
  int number_of_entities = 5;
  for (int entity_id = 0; entity_id < number_of_entities; ++entity_id) {
    Ogre::Entity* entity = scene->createEntity(mesh_name);
    Ogre::SceneNode* node = root_scene_node->createChildSceneNode();
    node->attachObject(entity);

    float position_offset =
        static_cast<float>(1 + entity_id * 2 - number_of_entities);
    position_offset = position_offset * 30;
    node->translate(position_offset, 0, -200.0f);
  }

  Ogre::Light* light = scene->createLight("MainLight");
  light->setPosition(20.0f, 80.0f, 50.0f);
}

bool Fastkat::update(float delta_time, Ogre::SceneNode* camera_node,
                     OIS::Keyboard* keyboard, OIS::Mouse* mouse,
                     float window_width, float window_height) {
  bool keep_running = true;
  {
    float movement = 200.0f * delta_time;
    Ogre::Vector3 translate(Ogre::Vector3::ZERO);
    if (keyboard->isKeyDown(OIS::KC_W)) {
      translate.z -= movement;
    }
    if (keyboard->isKeyDown(OIS::KC_S)) {
      translate.z += movement;
    }
    if (keyboard->isKeyDown(OIS::KC_A)) {
      translate.x -= movement;
    }
    if (keyboard->isKeyDown(OIS::KC_D)) {
      translate.x += movement;
    }
    camera_node->translate(translate, Ogre::Node::TS_LOCAL);
    if (keyboard->isKeyDown(OIS::KC_ESCAPE)) {
      keep_running = false;
    }
  }

  {
    const OIS::MouseState& mouse_state = mouse->getMouseState();
    if (mouse_state.buttonDown(OIS::MB_Left)) {
      // do something here?
    }
    float mouse_dx = mouse_state.X.rel / window_width;
    float mouse_dy = mouse_state.Y.rel / window_height;
    float rotation_speed = -5.0f;
    Ogre::Radian lAngleX(mouse_dx * rotation_speed);
    Ogre::Radian lAngleY(mouse_dy * rotation_speed);

    camera_node->yaw(lAngleX, Ogre::Node::TS_WORLD);
    camera_node->pitch(lAngleY, Ogre::Node::TS_LOCAL);
  }

  return keep_running;
}
