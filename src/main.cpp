#include "fmod.hpp"
#include "fmod_common.h"
#include <Geode/Geode.hpp>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/modify/SupportLayer.hpp>

using namespace geode::prelude;

struct TestPopup : Modify<TestPopup, SupportLayer> {
  struct Fields {
    bool m_playing_audio = false;
    bool m_playing_video = false;
  };

  void play_solo() {
    auto system = FMODAudioEngine::get()->m_system;
    FMOD::Channel *channel;
    FMOD::Sound *sound;

    system->createSound((Mod::get()->getResourcesDir() / "freebirdsolo.ogg").string().c_str(), FMOD_DEFAULT, nullptr, &sound);
    system->playSound(sound, nullptr, false, &channel);
    channel->setVolume(100.0f);

    this->m_fields->m_playing_audio = true;
  }

  CCAnimate *animate_spritesheet(CCSprite *sprite, int frame_width, int frame_height, int frames_per_row, int total_frames, float delay_per_frame) {
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

    return animate_action;
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

  void onRequestAccess(CCObject *sender) {
    auto frame_width = 184 / 2;
    auto frame_height = 60;

    if (! this->m_fields->m_playing_video) {
      auto screen_width = CCDirector::sharedDirector()->getWinSize().width;
      auto screen_height = CCDirector::sharedDirector()->getWinSize().height;

      CCSprite *free_bird = CCSprite::create("freebirdsolo.png"_spr, CCRectMake(0, 0, frame_width, frame_height));
      free_bird->setPosition(CCPointMake(screen_width / 2, screen_height / 2));
      free_bird->setTextureRect(CCRectMake(0, 0, frame_width, frame_height));
      this->addChild(free_bird, 100000);
      CCAnimate *animate = animate_spritesheet(free_bird, frame_width, frame_height, 20, 362, 0.105f);
      auto animator = CCRepeatForever::create(animate);
      free_bird->runAction(animator);
      resize_sprite(free_bird, screen_width, screen_height);
    }

    if (! this->m_fields->m_playing_audio) {
      play_solo();
    }
  }
};
