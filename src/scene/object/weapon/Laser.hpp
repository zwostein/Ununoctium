#ifndef SCENE_OBJECT_WEAPON_LASER_INCLUDED
#define SCENE_OBJECT_WEAPON_LASER_INCLUDED


#include "AWeapon.hpp"


class Material;
struct GLUquadric;


/// Caution - Laser
class Laser : public AWeapon
{
public:
	Laser( World * world );
	~Laser();

	virtual void updateSelf( const double & delta );
	virtual void drawSelf();
	virtual void draw2Self();

	virtual void triggerPressed();
	virtual void triggerReleased();

private:
	GLUquadric * mQuadric;
	bool mFired;
	float mCoolDown;
	float mRange;
	float mTrailRadius;
	float mTrailLength;
	float mTrailAlpha;
	float mDamage;
	QVector3D mTrailStart;
	QVector3D mTrailDirection;
	QVector3D mTrailEnd;
	Material * mMaterial;
};


#endif
