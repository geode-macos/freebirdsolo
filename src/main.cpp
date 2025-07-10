#include "Geode/binding/FMODAudioEngine.hpp"
#include "Geode/cocos/actions/CCActionInterval.h"
#include "Geode/cocos/cocoa/CCArray.h"
#include "Geode/cocos/cocoa/CCGeometry.h"
#include "Geode/cocos/platform/CCPlatformMacros.h"
#include "Geode/cocos/sprite_nodes/CCAnimation.h"
#include "Geode/cocos/sprite_nodes/CCSprite.h"
#include "Geode/cocos/sprite_nodes/CCSpriteFrame.h"
#include "Geode/cocos/textures/CCTexture2D.h"
#include "fmod.hpp"
#include "fmod_common.h"
#include <Geode/Geode.hpp>
#include <Geode/cocos/cocoa/CCObject.h>
#include <Geode/modify/SupportLayer.hpp>

using namespace geode::prelude;
using namespace cocos2d;

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
    if (!sprite || !sprite->getTexture()) {
      CCLOG("[ERROR]: Null sprite or texture");

      return nullptr;
    }


    if (frame_width <= 0 || frame_height <= 0 || frames_per_row <= 0 || total_frames <= 0 || delay_per_frame <= 0.0f) {
      CCLOG("[ERROR]: Invalid parameters for frame dimensions or timing.");
      return nullptr;
    }

    CCTexture2D* texture = sprite->getTexture();

    if (!texture) {
      CCLOG("[ERROR]: Sprite has no texture");
      return nullptr;
    }

    const CCSize& texture_size = texture->getContentSize();
    int frames_per_column = (int)(texture_size.height / frame_height);
    int max_possible_frames = frames_per_row * frames_per_column;

    if (total_frames > max_possible_frames) {
      CCLOG("[ERROR]: `total_frames` exceeds maximum frames in texture");
      return nullptr;
    }

    CCArray* frames = CCArray::createWithCapacity(total_frames);


    for (int i = 0; i < total_frames; ++i) {
      int row = i / frames_per_row;
      int col = i % frames_per_row;

      float x = static_cast<float>(col * frame_width);
      float y = static_cast<float>(row * frame_height);

      if (x + frame_width > texture_size.width || y + frame_height > texture_size.height) {
        CCLOG("[WARNING]: Frame %d is out of texture bounds. Skipping.", i);
        continue;
      }

      CCRect frameRect = CCRectMake(x, y, (float)frame_width, (float)frame_height);
      CCSpriteFrame* frame = CCSpriteFrame::createWithTexture(texture, frameRect);

      if (frame) {
        frames->addObject(frame);
      } else {
        CCLOG("[WARNING]: Failed to create sprite frame at index %d.", i);
      }
    }

    if (frames->count() == 0) {
      CCLOG("[ERROR]: No valid frames found for animation");
      return nullptr;
    }

    CCAnimation* animation = CCAnimation::createWithSpriteFrames(frames, delay_per_frame);

    if (!animation) {
      CCLOG("[ERROR]: Failed to create animation.");
      return nullptr;
    }

    return CCAnimate::create(animation);
  }

  void onRequestAccess(CCObject *sender) {
    if (! this->m_fields->m_playing_video) {
      auto screen_width = CCDirector::sharedDirector()->getWinSize().width;
      auto screen_height = CCDirector::sharedDirector()->getWinSize().height;

      CCSprite *free_bird = CCSprite::create("freebirdsolo.png"_spr, CCRectMake(0, 0, 185, 120));
      free_bird->setPosition(CCPointMake(screen_width / 2, screen_height / 2));
      free_bird->setTextureRect(CCRectMake(0, 0, 185, 120));
      this->addChild(free_bird, 100000);
      CCAnimate *animate = animate_spritesheet(free_bird, 185, 120, 20, 362, 0.1f);
      auto animator = CCRepeatForever::create(animate);
      free_bird->runAction(animator);
    }

    if (! this->m_fields->m_playing_audio) {
      play_solo();
    }

    SupportLayer::onRequestAccess(sender);
  }
};
