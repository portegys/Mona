// For conditions of distribution and use, see copyright notice in pong.hpp

#include "pong.hpp"

const float Pong::Paddle::DEFAULT_WIDTH  = 0.05f;
const float Pong::Paddle::DEFAULT_LENGTH = 0.2f;

// Paddle constructor.
Pong::Paddle::Paddle()
{
   width    = DEFAULT_WIDTH;
   length   = DEFAULT_LENGTH;
   position = 0.5f;
}


// Set paddle width.
void Pong::Paddle::setWidth(float w)
{
   width = w;
}


// Set paddle length.
void Pong::Paddle::setLength(float l)
{
   length = l;
}


// Set paddle position.
void Pong::Paddle::setPosition(float y)
{
   position = y;
}


void Pong::Paddle::load(FILE *fp)
{
   FREAD_FLOAT(&width, fp);
   FREAD_FLOAT(&length, fp);
   FREAD_FLOAT(&position, fp);
}


void Pong::Paddle::save(FILE *fp)
{
   FWRITE_FLOAT(&width, fp);
   FWRITE_FLOAT(&length, fp);
   FWRITE_FLOAT(&position, fp);
}


const float Pong::Ball::DEFAULT_RADIUS = 0.05f;

// Ball constructor.
Pong::Ball::Ball()
{
   radius   = DEFAULT_RADIUS;
   position = Vector(0.5f, 0.5f, 0.0f);
   velocity = Vector(0.0f, 0.0f, 0.0f);
}


// Set ball radius.
void Pong::Ball::setRadius(float r)
{
   radius = r;
}


// Set ball position.
void Pong::Ball::setPosition(float x, float y)
{
   position = Vector(x, y, 0.0f);
}


// Set ball speed.
void Pong::Ball::setSpeed(float speed)
{
   velocity.Normalize(speed);
}


// Set ball velocity.
void Pong::Ball::setVelocity(float dx, float dy)
{
   velocity = Vector(dx, dy, 0.0f);
}


// Step ball.
Pong::STEP_OUTCOME Pong::Ball::step(Paddle& paddle)
{
   Pong::STEP_OUTCOME outcome = Pong::CLEAR;

   if ((position.x - radius + velocity.x) <= 0.0f)
   {
      velocity.x = -velocity.x;
   }
   if ((position.x + radius + velocity.x) >= 1.0f)
   {
      outcome = Pong::SCORE;
   }
   if ((position.y - radius + velocity.y) <= 0.0f)
   {
      velocity.y = -velocity.y;
   }
   if ((position.y + radius + velocity.y) >= 1.0f)
   {
      velocity.y = -velocity.y;
   }
   float l2 = paddle.length / 2.0f;
   if (((position.x + radius + velocity.x) >= (1.0f - paddle.width)) &&
       (position.y <= (paddle.position + l2)) && (position.y >= (paddle.position - l2)))
   {
      velocity.x = -velocity.x;
      outcome    = Pong::STRIKE;
   }
   position.x += velocity.x;
   position.y += velocity.y;
   return(outcome);
}


void Pong::Ball::load(FILE *fp)
{
   FREAD_FLOAT(&radius, fp);
   FREAD_FLOAT(&position.x, fp);
   FREAD_FLOAT(&position.y, fp);
   FREAD_FLOAT(&velocity.x, fp);
   FREAD_FLOAT(&velocity.y, fp);
}


void Pong::Ball::save(FILE *fp)
{
   FWRITE_FLOAT(&radius, fp);
   FWRITE_FLOAT(&position.x, fp);
   FWRITE_FLOAT(&position.y, fp);
   FWRITE_FLOAT(&velocity.x, fp);
   FWRITE_FLOAT(&velocity.y, fp);
}


// Pong constructor.
Pong::Pong(float paddleLength, float ballRadius)
{
   paddle.setLength(paddleLength);
   ball.setRadius(ballRadius);
}


// Set paddle position.
void Pong::setPaddlePosition(float y)
{
   paddle.setPosition(y);
}


// Set ball position.
void Pong::setBallPosition(float x, float y)
{
   ball.setPosition(x, y);
}


// Set ball speed.
void Pong::setBallSpeed(float speed)
{
   ball.setSpeed(speed);
}


// Set ball velocity.
void Pong::setBallVelocity(float dx, float dy)
{
   ball.setVelocity(dx, dy);
}


// Step: returns true if ball scores.
Pong::STEP_OUTCOME Pong::step()
{
   return(ball.step(paddle));
}


// Load pong.
void Pong::load(char *filename)
{
   FILE *fp;

   if ((fp = FOPEN_READ(filename)) == NULL)
   {
      fprintf(stderr, "Cannot load pong from file %s\n", filename);
      exit(1);
   }
   load(fp);
   FCLOSE(fp);
}


void Pong::load(FILE *fp)
{
   paddle.load(fp);
   ball.load(fp);
}


// Save pong.
void Pong::save(char *filename)
{
   FILE *fp;

   if ((fp = FOPEN_WRITE(filename)) == NULL)
   {
      fprintf(stderr, "Cannot save pong to file %s\n", filename);
      exit(1);
   }
   save(fp);
   FCLOSE(fp);
}


void Pong::save(FILE *fp)
{
   paddle.save(fp);
   ball.save(fp);
}
