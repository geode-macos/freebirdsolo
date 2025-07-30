#include "Geode/binding/CCMenuItemSpriteExtra.hpp"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "fmod.hpp"
#include "fmod_common.h"
#include <Geode/Geode.hpp>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/SupportLayer.hpp>

using namespace geode::prelude;

struct FreeBirdJesusBtn : Modify<FreeBirdJesusBtn, MenuLayer> {
  void play_solo() {
    auto system = FMODAudioEngine::get();

    system->playMusic((Mod::get()->getResourcesDir() / "freebirdsolo.ogg").string(), false, 0.0f, 1);
  }

  CCFiniteTimeAction *animate_spritesheet(CCSprite *sprite, int frame_width, int frame_height, int frames_per_row, int total_frames, float delay_per_frame, bool loop) {
    if (! sprite) {
      log::error("animate_spritesheet: sprite is null");
      return nullptr;
    }

    if (frame_width <= 0 || frame_height <= 0) {
      log::error("animate_spritesheet: invalid frame dimensions (%d x %d)", frame_width, frame_height);
      return nullptr;
    }

    if (frames_per_row <= 0 || total_frames <= 0) {
      log::error("animate_spritesheet: invalid frame count (frames_per_row: %d, total_frames: %d)", frames_per_row, total_frames);
      return nullptr;
    }

    if (delay_per_frame <= 0.0f) {
      log::error("animate_spritesheet: invalid delay_per_frame (%f)", delay_per_frame);
      return nullptr;
    }

    CCTexture2D *texture = sprite->getTexture();
    if (! texture) {
      log::error("animate_spritesheet: sprite has no texture");
      return nullptr;
    }

    const CCSize texture_size = texture->getContentSize();
    const int texture_width = static_cast<int>(texture_size.width);
    const int texture_height = static_cast<int>(texture_size.height);

    if (frame_width > texture_width || frame_height > texture_height) {
      log::error("animate_spritesheet: frame size (%d x %d) exceeds texture size (%d x %d)", frame_width, frame_height, texture_width, texture_height);
      return nullptr;
    }

    CCArray *animation_frames = CCArray::create();
    if (! animation_frames) {
      log::error("animate_spritesheet: failed to create animation frames array");
      return nullptr;
    }

    for (int frame_index = 0; frame_index < total_frames; ++frame_index) {
      const int column = frame_index % frames_per_row;
      const int row = frame_index / frames_per_row;
      const int frame_x = column * frame_width;
      const int frame_y = row * frame_height;

      if (frame_x + frame_width > texture_width || frame_y + frame_height > texture_height) {
        log::warn("animate_spritesheet: frame %d at position (%d, %d) exceeds texture bounds", frame_index, frame_x, frame_y);
        continue;
      }

      const CCRect frame_rect = CCRectMake(static_cast<float>(frame_x), static_cast<float>(frame_y), static_cast<float>(frame_width), static_cast<float>(frame_height));

      CCSpriteFrame *sprite_frame = CCSpriteFrame::createWithTexture(texture, frame_rect);

      if (! sprite_frame) {
        log::warn("animate_spritesheet: failed to create sprite frame %d", frame_index);
        continue;
      }

      animation_frames->addObject(sprite_frame);
    }

    if (animation_frames->count() == 0) {
      log::error("animate_spritesheet: no valid frames generated");
      return nullptr;
    }

    CCAnimation *animation = CCAnimation::createWithSpriteFrames(animation_frames, delay_per_frame);

    if (! animation) {
      log::error("animate_spritesheet: failed to create animation");
      return nullptr;
    }

    CCAnimate *animate_action = CCAnimate::create(animation);
    if (! animate_action) {
      log::error("animate_spritesheet: failed to create animate action");
      return nullptr;
    }

    if (loop) {
      return CCRepeatForever::create(animate_action);
    } else {
      return animate_action;
    }
  }

  void resize_sprite(CCSprite *sprite, float width_px, float height_px) {
    if (sprite == nullptr)
      return;

    CCSize original_size = sprite->getContentSize();

    float scalex = width_px / original_size.width;
    float scaley = height_px / original_size.height;

    sprite->setScaleX(scalex);
    sprite->setScaleY(scaley);
  }

  void onPressBtn(CCObject *sender) {
    auto frame_width = 184 / 2;
    auto frame_height = 60;
    auto screen_width = CCDirector::sharedDirector()->getWinSize().width;
    auto screen_height = CCDirector::sharedDirector()->getWinSize().height;

    #ifndef GEODE_IS_IOS
    CCSprite *free_bird = CCSprite::create("freebirdsolo.png"_spr, CCRectMake(0, 0, frame_width, frame_height));
    #else
    CCSprite *free_bird = CCSprite::create("freebirdsolo.png"_spr);
    #endif
    free_bird->setPosition(CCPointMake(screen_width / 2, screen_height / 2));
    free_bird->setTextureRect(CCRectMake(0, 0, frame_width, frame_height));
    this->addChild(free_bird, 100000);
    CCFiniteTimeAction *animate = animate_spritesheet(free_bird, frame_width, frame_height, 20, 362, 0.1025f, false);
    free_bird->runAction(animate);
    resize_sprite(free_bird, screen_width, screen_height);
    play_solo();
  }

  bool init() {
    if (! MenuLayer::init())
      return false;

    auto spr = CircleButtonSprite::createWithSprite("menu-btn-sprite.png"_spr);
    auto free_bird_button = CCMenuItemSpriteExtra::create(spr, this, menu_selector(FreeBirdJesusBtn::onPressBtn));

    auto menu = this->getChildByID("bottom-menu");
    menu->addChild(free_bird_button);
    free_bird_button->setID("free_bird_button"_spr);
    menu->updateLayout();

    return true;
  }
};
