#pragma once

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "json.hpp"
#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "FMOD/common.hpp"
#include "Text.hpp"

namespace nl = nlohmann;

class CommonData {
public:
    CommonData() {}



    SDL_Window* mWindow;
    SDL_GLContext mContext;

    int mWindowWidth;
    int mWindowHeight;

    Text* mText;
    nl::json mTextData;

    FMOD::Studio::System* mAudioSystem;

    enum {
        JAPANESE,
        ENGLISH,
        NUM_LANG_TYPE
    };

    int mLangType;


private:
    int player_num;

};