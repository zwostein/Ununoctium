#ifndef SCENE_OBJECT_WEAPON_KNIFE_HPP
#define SCENE_OBJECT_WEAPON_KNIFE_HPP

#include "AWeapon.hpp"


class Material;
class StaticModel;


class Knife : public AWeapon
{
public:
	Knife( World * world );
	~Knife();

	virtual void updateSelf( const double & delta );
	virtual void drawSelf();

	virtual void triggerPressed();
	virtual void triggerReleased();
	virtual void holster();
	virtual void pull();
	virtual bool isPulled() { return mDrawn; }

private:
	StaticModel * mModel;
	bool mDrawn;
	bool mFired;
	bool mHitting;
	float mCooldown;
	float mDirection;
	float mRotation;
	float mRange;
	float mTrailLength;
	float mDamage;
	const QVector3D * mTarget;
	QVector3D mTrailStart;
	QVector3D mTrailDirection;
	QVector3D mTrailEnd;
};

#endif // SCENE_OBJECT_WEAPON_KNIFE_HPP
