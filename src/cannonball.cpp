/*****************************************************************************/
#include "cannonball.h"
/*****************************************************************************/
clsCannonball::clsCannonball() {
  /////////////////////////////////////////////////
  /// @brief The default Constructor for the cannonballs. It has to call
  ///         the values as default so we can create it in the array. The values are changed
  ///         later. The default values are as follows:
  ///         * deltat = 1 / 60 (for 60 fps)
  ///         * acc.x = 0
  ///         * acc.y = Global::Physics::fGravity
  ///         * props.radius = 5
  ///         * props.density = Global::Physics::uBallDensity
  ///         * place = 0
  ///         * dblLoc = place casted as a double
  ///         * Vel = 0
  /////////////////////////////////////////////////

  //Put some default values in; on the off chance I forget to set them later.
  blndragenabled_ = false;
  blnstarted_ = false;
  deltat_ = (1.00 / 60.00) ;
  acc_.x = 0.00;
  acc_.y = global::physics::kGravity;

  props_.radius = 5.0; //in meters
  props_.density = global::physics::kBallDensity; //density of steel in kg/m^3

  place_.x = 0;
  place_.y = 0;
  dblLOC_.x = (double) place_.x;
  dblLOC_.y = (double) place_.y;

  vel_.x = 0;
  vel_.y = 0;

  srand (time(NULL));

  color_.Red = rand() % 255;
  color_.Green = rand() % 255;
  color_.Blue = rand() % 255;

  if(global::config.values.blnDrawPathOnScreen) {
    // Reserve element spots equal to the number of max
    // locations we are going to keep track of
    path_.reserve(global::config.values.uintMaxNumPastPoints);

    // fill the new vector with "empty" values
    for(uchar i = 0; i < global::config.values.uintMaxNumPastPoints; ++i) {
      path_.push_back({0,0});
    } // end for max past points
    if (global::blnDebugMode) {printf("Path values initialized.\n");}
  } // end if draw path

  if (global::blnDebugMode) {printf("Ball created.\n");}
}
/*****************************************************************************/
clsCannonball::~clsCannonball() {
  // clear the vector path
  path_.clear();
}
/*****************************************************************************/
void clsCannonball::dragCalcValues(void) {
  /////////////////////////////////////////////////
  /// @brief This will calculate additional properties of the cannonball based on its radius.
  ///        These include:
  ///        * area
  ///        * volume
  ///        * mass
  ///
  /// @param void
  /// @return void
  ///
  /////////////////////////////////////////////////

	props_.area = (double) (2.0 * M_PI * pow(props_.radius,2));
	props_.volume = (double) ((4.0/3.0) * M_PI * pow(props_.radius,3));
	props_.mass = props_.density * props_.volume;
}
/*****************************************************************************/
void clsCannonball::dragUpdateAcc(void) {
  /////////////////////////////////////////////////
  /// @brief This will update the acceleration of the ball due to Friction
  ///        and Drag (if enabled)
  ///
  /// @return void
  ///
  /////////////////////////////////////////////////

  acc_.x = 0.0;
  acc_.y = global::physics::kGravity;
  if (vel_.x != 0.0 && vel_.y != 0.0 && props_.mass != 0.0) {
    double flow_velocity = sqrt(pow(vel_.x,2) + pow(vel_.y,2));
    double drag_force = (double) (0.5 * global::physics::kDensityAir *
                                  flow_velocity * global::physics::kDragCofficient
                                  * props_.area);
    double drag_acc = (double) (drag_force / props_.mass);
    double angle;

    angle = (vel_.x != 0.0) ? atan(vel_.y / vel_.x) : M_PI / 2.0;
    angle += vel_.x < 0.0 ? M_PI : 0.0;

    acc_.x += drag_acc * cos (angle);
    acc_.y += drag_acc * sin (angle);
    //Please recall fGravity is negative
    double friction = (double)global::physics::kKineticFriction *
                      (double)global::physics::kGravity;

    //Update acc for Friction values
    if ( dblLOC_.y <= screen_place_.h || dblLOC_.y >= screen::screenatt.height ) {
      //Ball is in contact with floor or ceiling update x acc
      acc_.x += friction * (vel_.x < 0.0 ? 1.0 : -1.0);
    }

    if ( dblLOC_.x <= 0.0 || dblLOC_.x >= screen::screenatt.width - screen_place_.w ) {
      //Ball is in contact with the wall update y acc.
      acc_.y += friction * (vel_.y < 0.0 ? 1.0 : -1.0);
    }
  } //end if things don't equal 0
}
/*****************************************************************************/
void clsCannonball::update(double newdeltat) {
  /////////////////////////////////////////////////
  /// @brief This will do the following:
  ///        * Update ball's position
  ///        * Update ball's velocity
  ///        * Call clsCannonball::Drag_updateacc if blnDragEnabled is true
  ///        * Log the ball's location (if enabled)
  ///        * Update CollisionBox
  ///        * Set the ball's blnstarted to false (stopping future updates) if total velocity is less than Global::Physics::fMinVelocity or equals NaN
  ///
  /// @param newdeltat = the time (in seconds) that have passed since the last update (see tick.cpp)
  /// @return void
  ///
  /////////////////////////////////////////////////

  deltat_ = newdeltat;

  if (blndragenabled_) { dragUpdateAcc(); }

  dblLOC_.x = dblLOC_.x + vel_.x * deltat_ + 0.5 * acc_.x * pow(deltat_,2);
	vel_.x = (vel_.x + acc_.x * deltat_);
	//Recoil the ball if it is past an edge
	if (dblLOC_.x <= 0.0 || dblLOC_.x >= screen::screenatt.width - screen_place_.w) {
    vel_.x *= global::physics::kRecoil;
    dblLOC_.x += dblLOC_.x <= 0.0 ? 1.0 : -1.0;
  } //end if hitting x edges

	dblLOC_.y = dblLOC_.y + vel_.y * deltat_ + 0.5 * acc_.y * pow(deltat_,2);
	vel_.y = (vel_.y + acc_.y * deltat_);
	//Recoil the ball if it is past an edge
	if (dblLOC_.y <= screen_place_.h || dblLOC_.y >= screen::screenatt.height) {
    vel_.y *= global::physics::kRecoil;
    dblLOC_.y += dblLOC_.y <= screen_place_.y ? 1.0 : -1.0;
  }//end if hitting y edges

  place_.x = dblLOC_.x < 0.0 ? 0 : round(dblLOC_.x);
  place_.y = dblLOC_.y < 0.0 ? 0 : round(dblLOC_.y);

	if (global::config.values.blnLogging) {
    FILE* logfile = fopen("logfile.log","a");
    fprintf(logfile,"Ball %3u \t (%.3f, %.3f)\n",ballID_, dblLOC_.x,dblLOC_.y);
    fclose(logfile);
	}

	//Update the collision box for the new location
	collisionbox_.left = place_.x;
	collisionbox_.top = screen::screenatt.height - place_.y;
	collisionbox_.right = collisionbox_.left + screen_place_.w;
	collisionbox_.bottom = collisionbox_.top + screen_place_.h;

  double total_v;
  total_v = sqrt( pow(vel_.x,2) + pow(vel_.y,2) );
  if (total_v < global::physics::kMinVelocity || isnan(total_v) ) {
    blnstarted_ = false;
    if (global::blnDebugMode) {
      if ( isnan(total_v) ) {printf("Ball velocity is NaN; killing it\n");}
      else {printf("Ball moving too slow; killing it\n");}
    } //end if debug mode
  } //end if should kill
	show(); //show the ball on the screen
}
/*****************************************************************************/
void clsCannonball::show() {
  /////////////////////////////////////////////////
  /// @brief Places the ball on the SDL Screen
  /// @return void
  /////////////////////////////////////////////////

  screen_place_.x = place_.x;
  screen_place_.y = screen::screenatt.height - place_.y;

  if (global::config.values.blnDrawPathOnScreen) { drawPath(place_); }

  Uint8 alpha = 0xFF;
  double dblAlpha;

  dblAlpha = (double) global::equations::kMassAlphaRatio * log(props_.mass) +
              global::equations::kMassAlphaOffset;

  alpha = dblAlpha < (double) global::equations::kAlphaMinimum ?
                      (Uint8) global::equations::kAlphaMinimum : (Uint8) dblAlpha;

  alpha = dblAlpha > 255.0 ? 255 : (Uint8) dblAlpha;

  //set the ball alpha
  SDL_SetTextureAlphaMod(screen::screenatt.ball, alpha);
  SDL_SetTextureColorMod(screen::screenatt.ball, color_.Red, color_.Green, color_.Blue);

  //Place the ball
  SDL_RenderCopy(screen::screenatt.ren,screen::screenatt.ball,NULL,&screen_place_);

  //reset ball Alpha and color so it doesn't effect the next ball
  SDL_SetTextureAlphaMod(screen::screenatt.ball, 0xFF);
  SDL_SetTextureColorMod(screen::screenatt.ball, 0xFF, 0xFF, 0xFF);

}
/*****************************************************************************/
void clsCannonball::setValues(double r, LOC init_place,
                              double init_vel, double init_angle, int newID) {
  /////////////////////////////////////////////////
  /// @brief Changes the values of the ball to whatever is entered.
  ///
  /// @param r = radius of the ball.
  /// @param init_place = its starting location.
  /// @param init_vel = its starting velocity.
  /// @param init_angle = its starting angle (in radians).
  /// @return void
  ///
  /////////////////////////////////////////////////

  ballID_ = newID;

  props_.radius = r; //in meters

  // Get the width and height of the texture
  SDL_QueryTexture(screen::screenatt.ball,NULL,NULL, &screen_place_.w, &screen_place_.h);

  acc_.x = 0.00;
  acc_.y = global::physics::kGravity;

  place_ = init_place;
  dblLOC_.x = (double) place_.x;
  dblLOC_.y = (double) place_.y;

  vel_.x = (double)(init_vel) * (cos(init_angle));
	vel_.y = (double)(init_vel) * (sin(init_angle));
	blnstarted_ = true;

  dragCalcValues();
	if (global::config.values.blnDragMode) {
    blndragenabled_ = true;
    if (global::blnDebugMode) {printf("Drag enabled.\n");}
  } //end if drag mode
}
/*****************************************************************************/
LOC clsCannonball::getplace() {
  /////////////////////////////////////////////////
  /// @brief Returns the ball's place
  /////////////////////////////////////////////////
  return place_;
}
/*****************************************************************************/
dblXY clsCannonball::getVelocity() {
  /////////////////////////////////////////////////
  /// @brief Returns the ball's velocity
  /////////////////////////////////////////////////
  return vel_;
}
/*****************************************************************************/
void clsCannonball::setVelocity(dblXY newvel) {
  /////////////////////////////////////////////////
  /// @brief Sets the ball's velocity
  /////////////////////////////////////////////////
  vel_.x = newvel.x;
  vel_.y = newvel.y;
}
/*****************************************************************************/
PP clsCannonball::getPhysicalProps() {
  /////////////////////////////////////////////////
  /// @brief Returns the ball's physical properties
  /////////////////////////////////////////////////
  return props_;
}
/*****************************************************************************/
void clsCannonball::setplace(LOC newplace) {
  /////////////////////////////////////////////////
  /// @brief Set the ball's place
  /////////////////////////////////////////////////
  dblLOC_.x = newplace.x;
  dblLOC_.y = newplace.y;
}
/*****************************************************************************/
void clsCannonball::setPhysicalProps(PP newprops) {
  /////////////////////////////////////////////////
  /// @brief Set the ball's physical properties
  ///        (only used on CollidePerfectInelastic)
  /////////////////////////////////////////////////
  props_ = newprops;
}
/*****************************************************************************/
BOX clsCannonball::getBOX() {
  /////////////////////////////////////////////////
  /// @brief Returns the ball's box
  /////////////////////////////////////////////////
  return collisionbox_;
}
/*****************************************************************************/
void clsCannonball::drawPath(LOC newplace) {
  /////////////////////////////////////////////////
  /// @brief Will draw the path the ball as taken based on kMaxNumPastPoints
  /// @param newplace = The last place the ball was at
  /// @return void
  /////////////////////////////////////////////////

  static uint updatesSinceLast;

  //If there have been enough updates since the last time the path was updated,
  //then update the path array otherwise inc updates
  SDL_SetTextureColorMod(screen::screenatt.pixel, color_.Red, color_.Green, color_.Blue);
  if ( updatesSinceLast >= global::config.values.uintPastDelay ) {
    updatesSinceLast = 0;
    // put new value into array
    path_.push_back(newplace);
    //delete the oldest (first spot) array value
    path_.erase(path_.begin());
  } else { updatesSinceLast++; } //end if update points

  //Now draw the path
  SDL_Rect dst;
  dst.w = dst.h = 1;
  for (uint i = 0; i < global::config.values.uintMaxNumPastPoints; ++i) {
    dst.y = screen::screenatt.height - (uint)(path_[i].y - screen_place_.h / 2);
    dst.x = (uint)(path_[i].x + screen_place_.w / 2);
    SDL_RenderCopy(screen::screenatt.ren, screen::screenatt.pixel, NULL, &dst);
  } //end for
  SDL_SetTextureColorMod(screen::screenatt.pixel, 0xFF, 0xFF, 0xFF);
}
/*****************************************************************************/
