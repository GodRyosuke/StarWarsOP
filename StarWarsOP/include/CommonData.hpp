#pragma once

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "json.hpp"
#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "FMOD/common.hpp"
#include "Text.hpp"
#include "Shader.hpp"

namespace nl = nlohmann;

class CommonData {
public:
    CommonData() {}

    SDL_Window* mWindow;
    SDL_GLContext mContext;

    int mWindowWidth;
    int mWindowHeight;

    Shader* mTextShader;
    Shader* m3DTextShader;

    Text* mText;

    FMOD::Studio::System* mAudioSystem;


private:

};