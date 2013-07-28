#ifndef SCENE_OBJECT_TEAPOT_INCLUDED
#define SCENE_OBJECT_TEAPOT_INCLUDED

#include "AObject.hpp"


class Material;
class AudioSample;


/// It's a Teapot
/**
 *
 */
class Teapot : public AObject
{
public:
	Teapot( Scene * scene, const float & size );
	virtual ~Teapot();

	virtual void updateSelf( const double & delta );
	virtual void drawSelf();

	virtual QVector<AObject*> collideSphere( const AObject * exclude, const float & radius, QVector3D & center, QVector3D * normal = NULL );

private:
	Material * mMaterial;
	AudioSample * mAudioSample;
	float mSize;
	QVector3D mLastWorldPosition;
};


#endif
