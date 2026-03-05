#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include "include.hpp"

enum Animations //ajouter des animations pour avoir 10 int (0-9)
{
    NONE,
    WAVING,
    WALKING, //vald - dans la rue
    JUMPING,
    T_POSE, // denzel curry - ultimate
    NARUTO_RUN, // Naruto - Run
    GANGNAM_STYLE, // PSY - Gangnam Style
    EAGLE_FLIGHT, // AIGLES QUI HURLENT FORT MURICAAAAAAAA
    MJ_PENCHING, // Michael Jackson - Billie Jean
    HARDBASS_ROBLOX, // titre
};

class Animator
{
    private:
        int   _state;
        float _time;
        int _pid;

    public:
        Animator();
        void setState(Animations state);
        void setPID(int pid);
        int getPID() const;
        void update(float deltaTime);
        void draw(Shader& shader, body& myBody);
};

#endif