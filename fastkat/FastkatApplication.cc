// fastkat - Copyright (c) Omiod, port by Gustav

#include "fastkat/FastkatApplication.h"

FastkatApplication::FastkatApplication(void) {}

FastkatApplication::~FastkatApplication(void) {}

void FastkatApplication::createScene(void) {
  // Set the scene's ambient light
  mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));

  // Create an Entity
  Ogre::Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");

  // Create a SceneNode and attach the Entity to it
  Ogre::SceneNode* headNode =
      mSceneMgr->getRootSceneNode()->createChildSceneNode("HeadNode");
  headNode->attachObject(ogreHead);

  // Create a Light and set its position
  Ogre::Light* light = mSceneMgr->createLight("MainLight");
  light->setPosition(20.0f, 80.0f, 50.0f);
}
