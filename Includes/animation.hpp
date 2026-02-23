#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include "include.hpp"

enum Animations //ajouter des animations pour avoir 10 int (0-9)
{
    NONE,
    WAVING,
    WALKING,
};

class Animator
{
    private:
        int   _state;
        float _time;

    public:
        Animator();
        void setState(Animations state);
        void update(float deltaTime);
        void draw(Shader& shader, body& myBody);
};

#endif