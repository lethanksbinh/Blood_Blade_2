#include "Player.h"
#include "Others.h"
#include "LTimer.h"
#include <math.h>
#include <sstream>
#include <SDL_ttf.h>

#define FORCE_SPEED 1/10
#define FORCE_CAPABILITY 100
#define FORCE_LOSS 2
#define M_PI 3.14159265358979323846

Player::Player(SDL_Renderer* gRenderer, LTexture& gRedTexture, const SDL_Rect& camera)
{
    renderer = gRenderer;

    mPos.init(LEVEL_WIDTH / 2, LEVEL_HEIGHT / 2);
    mCollider = { mPos.x, mPos.y, PLAYER_WIDTH, PLAYER_HEIGHT };
    mForce = mVelX = mVelY = 0;

    mHP = PLAYER_MAX_HP;
    gotHit = false;
    isAlive = true;
    isAppear = true;

    for (int i = 0; i < TOTAL_PARTICLES; ++i)
    {
        particles[i] = new Particle(mCollider, gRedTexture);
    }
}

Player::~Player()
{
    SDL_DestroyRenderer(renderer);
    renderer = NULL;

    for (int i = 0; i < TOTAL_PARTICLES; ++i)
    {
        delete particles[i];
    }
}

void Player::handleEvent(SDL_Event& e, const SDL_Rect& camera)
{
    swordAngle = SDL_atan2((initPos.y - lastPos.y), (initPos.x - lastPos.x)) * (180.0 / M_PI) + 90;
    
    if (e.type == SDL_MOUSEBUTTONDOWN && !isHold)
    {
        SDL_GetMouseState(&initPos.x, &initPos.y);
        //if (mPos.x - camera.x < initPos.x && initPos.x < mPos.x - camera.x + PLAYER_WIDTH && mPos.y - camera.y < initPos.y && initPos.y < mPos.y - camera.y + PLAYER_HEIGHT)
        isHold = true;
        mTime.start();
    }
    SDL_GetMouseState(&lastPos.x, &lastPos.y);
    if (e.type == SDL_MOUSEBUTTONUP && isHold) {
        updateVel((initPos.x - lastPos.x), (initPos.y - lastPos.y));
        isHold = false;
        mTime.stop();

        std::cerr << "Final remainVel " << mForce << std::endl;
    }
}

void Player::react(const SDL_Rect& enemyCollider)
{
    if (isAlive)
    {
        if (!isMoving() && checkCollision(mCollider, enemyCollider))
        {
            gotHit = true;
            mHP--;
            std::cerr << mVelX << " " << mVelY << std::endl;
        }

        if (mHP <= 0)
        {
            die();
        }
    }
}

void Player::updateVel(const int& x, const int& y)
{
    mVelX = PLAYER_VEL * (x * 1.0) / pytago(x, y);
    mVelY = PLAYER_VEL * (y * 1.0) / pytago(x, y);
}

void Player::updateForce()
{
    if (isHold)
    {
        mForce = (mTime.getTicks() * FORCE_SPEED) % (2 * FORCE_CAPABILITY);
        if (mForce > FORCE_CAPABILITY) mForce = 2 * FORCE_CAPABILITY - mForce;
    }
}

void Player::move()
{
    if (isAlive)
    {
        if (mVelX < 0) flip = SDL_FLIP_HORIZONTAL;
        else if (mVelX > 0) flip = SDL_FLIP_NONE;

        updateForce();

        if (mForce > 0)
        {
            mPos.x += mVelX;
            mCollider.x = mPos.x;

            mPos.y += mVelY;
            mCollider.y = mPos.y;

            if ((mPos.x < 0) || (mPos.x + PLAYER_WIDTH > LEVEL_WIDTH))
            {
                mVelX = -mVelX;
                mForce += FORCE_LOSS;
            }

            if ((mPos.y < 0) || (mPos.y + PLAYER_HEIGHT > LEVEL_HEIGHT))
            {
                mVelY = -mVelY;
                mForce += FORCE_LOSS;
            }

            mForce -= FORCE_LOSS;
        }
        else
        {
            mForce = mVelX = mVelY = 0;
        }
    }
}

void Player::renderParticles(LTexture& gRedTexture, const SDL_Rect& camera)
{
    for (int i = 0; i < mForce/5; ++i)
    {
        if (particles[i]->isDead())
        {
            delete particles[i];
            particles[i] = new Particle(mCollider, gRedTexture);
        }
    }

    for (int i = 0; i < mForce/5; ++i)
    {
        particles[i]->render(renderer, camera);
    }
}

void Player::die()
{
    isAlive = false;
    mCollider.y = -500;
    mForce = FORCE_CAPABILITY;
}

void Player::render(LTexture& gPlayerTexture, LTexture& gRedTexture, LTexture& gBlueSlash, LTexture& gRedSword, LTexture& gRedCircle, const SDL_Rect& camera)
{
    if (isAlive)
    {
        gPlayerTexture.render(renderer, mPos.x - camera.x, mPos.y - camera.y, NULL, 0.0, 0, flip);
        if (gotHit)
        {
            gBlueSlash.render(renderer, mPos.x - PLAYER_WIDTH / 2 - camera.x, mPos.y - PLAYER_HEIGHT / 2 - camera.y, NULL, 0.0, 0, flip);
            gotHit = false;
        }
        if (isHold)
        {
            //Show mouse position when holding
            gRedCircle.render(renderer, initPos.x - PLAYER_WIDTH/4, initPos.y - PLAYER_HEIGHT/4);
            gRedCircle.render(renderer, lastPos.x - PLAYER_WIDTH/4, lastPos.y - PLAYER_HEIGHT/4);
            //Show player direction
            gRedSword.render(renderer, mPos.x - camera.x - PLAYER_WIDTH, mPos.y - camera.y - PLAYER_HEIGHT, NULL, swordAngle, 0);
        }
    }

    renderParticles(gRedTexture, camera);
}


