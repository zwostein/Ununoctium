#ifndef SCENE_OBJECT_WORLD_INCLUDED
#define SCENE_OBJECT_WORLD_INCLUDED


#include <scene/AKeyListener.hpp>
#include <scene/AMouseListener.hpp>
#include <geometry/ParticleSystem.hpp>

#include "AObject.hpp"
#include "creature/Player.hpp"
#include "creature/Dummy.hpp"
#include "Sky.hpp"
#include "Landscape.hpp"
#include "WavefrontObject.hpp"

#include <QTime>
#include <QVector>
#include <QMatrix4x4>

class Shader;
class TextureRenderer;
class Material;
class SplatterSystem;


/// Splatter quality settings
class SplatterQuality
{
	SplatterQuality() {}
	~SplatterQuality() {}
public:
	enum Type
	{
		LOW	= 0,
		MEDIUM	= 1,
		HIGH	= 2
	};
	const static int num = 3;

	static Type fromString( const QString & name );
	static QString toString( const Type & quality );
	static const Type & maximum() { return sMaximum; }
	static void setMaximum( const Type & max ) { sMaximum = max; }

private:
	static Type sMaximum;
};


/// World object
/**
 *
 */
class World : public AObject, public AKeyListener
{
public:
	World( Scene * scene, QString name );
	virtual ~World();

	virtual void updateSelf( const double & delta );
	virtual void updateSelfPost( const double & delta );
	virtual void drawSelf();
	virtual void drawSelfPost();

	virtual void keyPressEvent( QKeyEvent * event );
	virtual void keyReleaseEvent( QKeyEvent * event );

	void addLightSource( ALightSource * lightSource );
	void removeLightSource( ALightSource * lightSource );

	SplatterSystem * splatterSystem() { return mSplatterSystem; }

	QSharedPointer<Landscape> landscape() { return mLandscape; }
	QSharedPointer<Sky> sky() { return mSky; }

	QSharedPointer<Player> player() { return mPlayer; }
	QSharedPointer<Teapot> teapot() { return mTeapot; }

private:
	class SplatterInteractor : public ParticleSystem::Interactable
	{
	public:
		SplatterInteractor( World & world ) : mWorld(world) {}
		virtual ~SplatterInteractor() {}
		virtual void particleInteraction( const double & delta, ParticleSystem::Particle & particle );
	private:
		World & mWorld;
	};
	SplatterInteractor * mSplatterInteractor;
	bool mTimeLapse;
	bool mTimeReverse;
	float mTimeOfDay;
	QSharedPointer<Sky> mSky;
	QSharedPointer<Landscape> mLandscape;
	QSharedPointer<Teapot> mTeapot;
	QSharedPointer<Player> mPlayer;
	QSharedPointer<Dummy> mDummy;
	QSharedPointer<WavefrontObject> mTable;
	QSharedPointer<WavefrontObject> mTree;
	QVector3D mTarget;
	QVector3D mTargetNormal;
	SplatterSystem * mSplatterSystem;
	QList< ALightSource * > mLightSources;
};


#endif
