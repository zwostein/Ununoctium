#ifndef WORLD_INCLUDED
#define WORLD_INCLUDED


#include "../KeyListener.hpp"
#include "../MouseListener.hpp"
#include "AObject.hpp"


class Sky;
class Landscape;
class Shader;
class TextureRenderer;


class World : public AObject, public KeyListener
{
public:
	World( Scene * scene );
	virtual ~World();
	
	virtual void updateSelf( const float & delta );
	virtual void drawSelf();
	virtual void drawSelfPost();

	virtual void keyPressEvent( QKeyEvent * event );
	virtual void keyReleaseEvent( QKeyEvent * event );

private:
	bool mTimeLapse;
	float mTimeOfDay;
	Sky * mSky;
	QSharedPointer<Landscape> mLandscape;
};


#endif