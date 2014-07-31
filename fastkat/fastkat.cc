// fastkat - Copyright (c) Omiod, port by Gustav

#include "fastkat/fastkat.h"

#include <vector>
#include <algorithm>

#include "OGRE/OgreString.h"
#include "OGRE/OgreSceneManager.h"
#include "OGRE/OgreSceneNode.h"
#include "OGRE/OgreEntity.h"
#include "OGRE/OgreLight.h"

#include "OIS/Ois.h"

Fastkat::Fastkat() {}

const float SCALE = 2.0f;

class Item {
 public:
  Ogre::Vector3 rotation;
  Ogre::Vector3 position;

  Ogre::Entity* entity;
  Ogre::SceneNode* node;

  void update() {
    // node->setOrientation(Ogre::Quaternion::)
    Ogre::Vector3 p = position;
    p.z *= SCALE;
    node->setPosition(p);
  }
};

int status = 0;
float speed = 10;
float track = 0;
int next_frame = 0;
int phase = 0;
const float fogdepth = 3500;
float fov = 80;
float p[8];
float health = 100;
Item ship;
std::vector<Item> objs;
float dtm = 0;
float maxspeed = 0;
int score = 0;
float mytm = 0;
float clight = 0;
float speedlimit = 100;

const float MathPI = 3.14f;

Ogre::Light* light1;
Ogre::Light* light2;

void SetHsv(Ogre::Light* l, float h, float s, float v) {
  Ogre::ColourValue cv;
  cv.setHSB(h, s, v);
  l->setDiffuseColour(cv);
  l->setSpecularColour(cv);
}

float Random() { return Ogre::Math::RangeRandom(0.0f, 1.0f); }
float MathCos(float d) { return Ogre::Math::Cos(d); }
float MathSin(float d) { return Ogre::Math::Sin(d); }
float MathAbs(float d) { return Ogre::Math::Abs(d); }
float MathMin(float a, float b) { return std::min(a, b); }
float MathMax(float a, float b) { return std::max(a, b); }
int MathFloor(float d) { return static_cast<int>(Ogre::Math::Floor(d)); }

void game_init(Ogre::SceneNode* root_scene_node, Ogre::SceneManager* scene) {
  scene->setFog(Ogre::FOG_LINEAR, Ogre::ColourValue::Black, 0.0, 1,
                fogdepth * SCALE);
  for (int i = 0; i < 200; i++) {
    Item obs;
    obs.position.z = -i * (fogdepth / 200);
    obs.position.x = Random() * 5000 - 2500;
    obs.position.y = Random() * 3000 - 1500;
    obs.rotation.x = Random() * MathPI;
    obs.rotation.y = Random() * MathPI;

    Ogre::String mesh_name = "ogrehead.mesh";
    obs.entity = scene->createEntity(mesh_name);
    obs.node = root_scene_node->createChildSceneNode();
    obs.node->attachObject(obs.entity);
    obs.update();

    objs.push_back(obs);
  }
}

void gameReset() {
  health = 100;
  speed = 0;
  score = 0;
  status = 1;
  ship.position.x = 0;
  ship.position.y = 0;
  // view = 1;
  score = 0;
  maxspeed = 52;
  // speedlimit = 100;
  // zcamera2 = 0;

  for (std::size_t i = 0, l = objs.size(); i < l; i++) {
    Item& obj = objs[i];
    obj.position.x = Random() * 5000 - 2500;
    obj.position.y = -300;
  }
}

void please_start_game() {
  if (status == 0) gameReset();
}

void introReset(bool gamecompleted) {
  speed = 0;
  // view = 2;
  status = 0;
  /*
  hiscore = localStorage.getItem("fk2hiscore");
  if ( hiscore == 0 || hiscore == undefined || hiscore == null ) hiscore = 0;

  if ( gamecompleted && hiscore < score ) {
    hiscore = score;
    localStorage.setItem("fk2hiscore", hiscore);
  }

  html("score","hi-score "+hiscore);
  hide("hud");
  show("like");
  show("panel1");
  show("feedback");
  show("neon");
  html("player","");
  html("musicplayer","");*/
}

void doit() {
  for (std::size_t i = 0, l = objs.size(); i < l; i++) {
    Item& object = objs[i];

    object.rotation.x += 0.01f;
    object.rotation.y += 0.005f;

    object.position.z += speed;
    if (object.position.z > 100) {
      object.position.z -= fogdepth;
      next_frame++;

      switch (phase) {
        case 1:  // asteroids field
          if (Random() < 0.97) {
            object.position.x = Random() * 3000 - 1500;
            object.position.y = Random() * 3000 - 1500;
          } else {
            object.position.x = ship.position.x;
            object.position.y = ship.position.y;
          }
          break;
        case 2:
          object.position.x = MathCos(next_frame / p[0]) * p[1] +
                              MathCos(next_frame / p[2]) * p[3];
          object.position.y = MathSin(next_frame / p[4]) * p[5] +
                              MathSin(next_frame / p[6]) * p[7];
          break;
        case 3:
          object.position.x = MathCos(next_frame / p[0]) * p[1] +
                              MathCos(next_frame / p[2]) * p[3];
          object.position.y = MathSin(next_frame / p[4]) * p[5] +
                              MathSin(next_frame / p[6]) * p[7];
          break;
        case 4: {
          float r = MathCos(next_frame / p[0]) * 2000;
          object.position.x = MathCos(next_frame / p[1]) * r;
          object.position.y = MathSin(next_frame / p[1]) * r;
          break;
        }
        case 5:
          if (Random() < 0.95) {
            object.position.x = ship.position.x;
            object.position.y = ship.position.y;
          } else {
            object.position.x = Random() * 3000 - 1500;
            object.position.y = Random() * 3000 - 1500;
          }
          break;
      }
    }

    // Collision check
    if (MathAbs(ship.position.x - object.position.x) < 100 &&
        MathAbs(ship.position.y - object.position.y) < 50 &&
        MathAbs(ship.position.z - object.position.z) < 50) {
      if (speed > 0) {
        health -= speed;
      }
      speed = -3;
    }
  }

  if (health < 0 && speed > 0) introReset(true);

  speed = speed + (dtm / 300);
  if (speed > maxspeed) {
    speed = maxspeed;
    maxspeed = MathMin(maxspeed + (dtm / 1500), 100);
  }
}

void nextphase() {
  track -= speed;
  if (track < 0) {
    track = 5000 + Ogre::Math::RangeRandom(0, 5000);

    phase = MathFloor(Random() * 5) + 1;

    switch (phase) {
      case 1:
        break;

      case 2:  // twirl 1
        p[0] = Random() * 3 + 0.01;
        p[1] = 300 + Random() * 900;
        p[4] = p[0];
        p[5] = 300 + Random() * 900;

        p[2] = 8 + Random() * 77;  // x secondary
        p[3] = Random() * 500;     // x secondary
        p[6] = 8 + Random() * 77;  // y secondary
        p[7] = Random() * 400;     // y secondary

        break;

      case 3:  // snake
        p[0] = Random() * 30 + 7;
        p[1] = 300 + Random() * 900;
        p[4] = p[0];
        p[5] = 300 + Random() * 700;

        p[2] = 8 + Random() * 77;      // x secondary
        p[3] = 200 + Random() * 1000;  // x secondary
        p[6] = 8 + Random() * 77;      // y secondary
        p[7] = 200 + Random() * 1000;  // y secondary

        break;

      case 4:  // plane
        p[0] = Random() * 3 + 0.01;
        p[1] = (Random() * 500 + 40) * (Random() > 0.5 ? 1 : -1);
        break;
    }
  }
}

void render_game() {
  if (speed > 0) {
    clight = speed / speedlimit;
    // bdy.style.backgroundColor = '#000';
  } else {
    clight = 0;
    // tmp = -Math.floor( Math.random()*speed*100);
    // bdy.style.backgroundColor = 'rgb('+tmp+','+(tmp/2)+',0)';
  }
  SetHsv(light2, clight, 0.3, 1);
  doit();
  nextphase();
  fov = fov - (fov - (65 + speed / 2)) / 4;
}

float frac(float f) { return f - static_cast<int64_t>(f); }

void render_intro() {
  clight = frac(mytm / 100000);
  SetHsv(light2, clight, 0.4, 1);

  /*
  zcamera2 = zcamera = -220;
  xratio = 1;
  yratio = 1;

  camera.position = { x:shipX*xratio , y:shipY*yratio , z:zcamera };

  group2.position.z += speed;
  group2.rotation.y = - new Date().getTime() * 0.0004;
  if ( group2.position.z > 0 ) {
    group2.position.z = -fogdepth;
    group2color.color.setHSV( Math.random(),1,1 );
  }*/

  for (std::size_t i = 0, l = objs.size(); i < l; i++) {
    Item& object = objs[i];

    object.rotation.x += 0.01;
    object.rotation.y += 0.005;

    object.position.z += speed;
    if (object.position.z > 100) {
      object.position.z -= fogdepth;
      object.position.x = Random() * 3000 - 1500;
      object.position.y = Random() * 3000 - 1500;
    }
  }
  speed = 0.3;

  fov = 110;
}

void animate(float dt) {
  dtm = dt * 1000;
  mytm += dtm;
  while (mytm > 10000) mytm -= 10000;

  if (status == 0) {
    render_intro();
  } else {
    render_game();
  }

  for (std::size_t i = 0, l = objs.size(); i < l; i++) {
    Item& object = objs[i];
    object.update();
  }
}

//------------

void Fastkat::load(Ogre::SceneNode* root_scene_node, Ogre::SceneManager* scene,
                   Ogre::Camera* camera) {
  game_init(root_scene_node, scene);

  camera->setFarClipDistance(fogdepth * SCALE);

  light1 = scene->createLight("MainLight1");
  light1->setType(Ogre::Light::LT_DIRECTIONAL);
  light1->setDirection(Ogre::Vector3(2, -3, 1.5).normalisedCopy());
  light1->setDiffuseColour(
      Ogre::ColourValue(0xdd / 255.0f, 0xdd / 255.0f, 0xff / 255.0f));

  light2 = scene->createLight("MainLight2");
  light2->setType(Ogre::Light::LT_DIRECTIONAL);
  SetHsv(light2, Random(), 0.75, 1);
  light2->setDirection(Ogre::Vector3(-1.5, 2, 0).normalisedCopy());
}

bool Fastkat::update(float delta_time, Ogre::SceneNode* camera_node,
                     OIS::Keyboard* keyboard, OIS::Mouse* mouse,
                     float window_width, float window_height,
                     Ogre::Camera* camera) {
  animate(delta_time);
  bool keep_running = true;
  {
    /*
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
    */
    if (keyboard->isKeyDown(OIS::KC_ESCAPE)) {
      keep_running = false;
    }
  }

  {
    const OIS::MouseState& mouse_state = mouse->getMouseState();
    if (mouse_state.buttonDown(OIS::MB_Left)) {
      please_start_game();
    }
    float mx = (mouse_state.X.abs / window_width) * 2 - 1;
    float my = (mouse_state.Y.abs / window_height) * 2 - 1;

    mx = MathMax(MathMin(mx, 1), -1);
    my = MathMax(MathMin(my, 1), -1);

    float shipX = ship.position.x;
    float shipY = ship.position.y;

    // 700
    // 250
    shipX = shipX - (shipX - mx * 700) / 4;
    shipY = shipY - (shipY - (-my) * 700) / 4;
    ship.position.x = shipX;
    ship.position.y = shipY;
    ship.position.z = -200;

    float rotation_speed = -4.0f;

    float zcamera = -150 * SCALE;
    float xratio = 1;
    float yratio = 1;

    camera_node->setPosition(shipX * xratio, shipY * yratio, zcamera);
    /*camera_node->resetOrientation();
    camera_node->lookAt(Ogre::Vector3(shipX * 0.5, shipY * 0.25, -1000),
                        Ogre::Node::TS_PARENT);*/
    // Ogre::Radian angle_x(mouse_dx * rotation_speed);
    // Ogre::Radian angle_y(mouse_dy * rotation_speed);
    // camera_node->yaw(angle_x, Ogre::Node::TS_WORLD);
    // camera_node->pitch(angle_y, Ogre::Node::TS_LOCAL);
  }

  // camera->setFOVy(Ogre::Degree(fov));

  return keep_running;
}
