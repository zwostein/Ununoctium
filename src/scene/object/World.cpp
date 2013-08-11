/*
 * This file is part of Splatterlinge.
 *
 * Splatterlinge is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * Splatterlinge is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Splatterlinge. If not, see <http://www.gnu.org/licenses/>.
 */

#include "World.hpp"

#include <scene/Scene.hpp>
#include <utility/RandomNumber.hpp>
#include <geometry/ParticleSystem.hpp>
#include <effect/SplatterSystem.hpp>
#include "Landscape.hpp"
#include "Teapot.hpp"
#include "Sky.hpp"

#include <QPainter>
#include <QSettings>


SplatterQuality::Type SplatterQuality::sMaximum = SplatterQuality::HIGH;

SplatterQuality::Type SplatterQuality::fromString( const QString & name )
{
	QString up = name.toUpper();
	if( up == "LOW" )
		return LOW;
	if( up == "MEDIUM" )
		return MEDIUM;
	if( up == "HIGH" )
		return HIGH;
	qWarning() << name << QT_TR_NOOP("is not a valid splatter quality type");
	return LOW;
}


QString SplatterQuality::toString( const SplatterQuality::Type & quality )
{
	switch( quality )
	{
		case SplatterQuality::HIGH:
			return "High";
		case SplatterQuality::MEDIUM:
			return "Medium";
		case SplatterQuality::LOW:
			return "Low";
	}
	return "Low";
}


World::World( Scene * scene, QString name ) :
	AObject( scene )
{
	QSettings s( "./data/world/"+name+".ini", QSettings::IniFormat );
	s.beginGroup( "World" );
		QString skyName = s.value( "skyName", "earth" ).toString();
		QString landscapeName = s.value( "landscapeName", "earth" ).toString();
	s.endGroup();

	mTimeLapse = false;
	mTimeReverse = false;
	mTimeOfDay = 0.0f;

	QTime time = QTime::currentTime();
	qsrand((uint)time.msec());

	mLandscape = QSharedPointer<Landscape>( new Landscape( scene, landscapeName ) );
	add( mLandscape );

	mSky = QSharedPointer<Sky>( new Sky( this, skyName ) );
	add( mSky );

	mPlayer = QSharedPointer<Player>( new Player( this ) );
	add( mPlayer );
	scene->eye()->attach( mPlayer );

	mTeapot = QSharedPointer<Teapot>( new Teapot( scene, 2 ) );
	mTeapot->setPositionY( mLandscape->terrain()->getHeight( QPointF(0,0) ) );
	add( mTeapot );

	mDummy = QSharedPointer<Dummy>( new Dummy( this ) );
	add( mDummy );

	qsrand( QTime::currentTime().msec() );
	foreach( Vegetation * v, mLandscape->vegetations() )
	{
		qDebug() << v->position();
		QSharedPointer<Forest> f = QSharedPointer<Forest>( new Forest( this, v->name(), v->position(), v->radius(), v->number() ) );
		f->setPositionX( v->position().x() );
		f->setPositionY( 0 );
		f->setPositionZ( v->position().y() );
		add( f );
	}

	/*
	QVector<QMatrix4x4> * instances = new QVector<QMatrix4x4>();
	QPointF min;
	QPointF max;
	for( float i=0; i<=500; i+=(float)rand()/( (float)RAND_MAX/1) )
	{
		QMatrix4x4 pos;
		float j = (float)rand()/( (float)RAND_MAX/500);

		float x = 100 + i;
		float y = mLandscape->terrain()->getHeight( QPointF(100+i,-j) ) - 1;
		float z = -j;

		if( x < min.x() || i == 0 )
			min.setX( x );
		if( x > max.x() || i == 0 )
			max.setX( x );
		if( z < min.y() || i == 0 )
			min.setY( z );
		if( z > max.y() || i == 0 )
			max.setY( z );

		pos.translate(x, y, z);
		pos.scale( 0.3f+(float)rand()/( (float)RAND_MAX/0.3 ) );
		pos.rotate( (float)rand()/(float)RAND_MAX, 0.0, 1.0, 0.0 );
		instances->append(pos);
	}
	mTree = QSharedPointer<WavefrontObject>( new WavefrontObject( scene, "data/object/tree/tree.obj", instances ) );
	mTree->setPositionX( ( min.x() + max.x() ) / 2 );
	mTree->setPositionZ( ( min.y() + max.y() ) / 2 );
	mTree->setPositionY( ( 0 ) );
	add( mTree );
	*/

	scene->addKeyListener( this );

	mTarget = QVector3D(0,0,0);

	mSplatterSystem = new SplatterSystem
	(
		scene->glWidget(),
		mLandscape->terrain(),
		"SplatterBig",
		"Splatter",
		"./data/sound/splatter.ogg"
	);
	mSplatterInteractor = new SplatterInteractor( *this );
	mSplatterSystem->particleSystem()->setInteractionCallback( mSplatterInteractor );
}


World::~World()
{
	mLightSources.clear();
	scene()->removeKeyListener( this );
	delete mSplatterSystem;
	delete mSplatterInteractor;
}


void World::addLightSource( ALightSource * lightSource )
{
	if( !mLightSources.contains( lightSource ) )
		mLightSources.append( lightSource );
}


void World::removeLightSource( ALightSource * lightSource )
{
	mLightSources.removeOne( lightSource );
}


void World::keyPressEvent( QKeyEvent * event )
{
	switch( event->key() )
	{
	case Qt::Key_PageDown:
		mTimeLapse = true;
		break;
	case Qt::Key_PageUp:
		mTimeReverse = true;
		break;
	default:
		break;
	}
}


void World::keyReleaseEvent( QKeyEvent * event )
{
	switch( event->key() )
	{
	case Qt::Key_PageDown:
		mTimeLapse = false;
		break;
	case Qt::Key_PageUp:
		mTimeReverse = false;
		break;
	default:
		break;
	}
}


void World::updateSelf( const double & delta )
{
	mTimeOfDay += 0.01f * delta;

	if( mTimeLapse )
		mTimeOfDay += 0.2f * delta;
	if( mTimeReverse )
		mTimeOfDay -= 0.2f * delta;

	if( mTimeOfDay > 1.0f )
		mTimeOfDay -= 1.0f;

	mSky->setTimeOfDay( mTimeOfDay );

	if( SplatterQuality::maximum() == SplatterQuality::MEDIUM )
		mSplatterSystem->setSplatBelow( true );
	else
		mSplatterSystem->setSplatBelow( false );
	mSplatterSystem->update( delta );
}


void World::updateSelfPost( const double & delta )
{

}


void World::drawSelf()
{
	glLight( GL_LIGHT0, GL_POSITION, mSky->sunDirection() );
	glLight( GL_LIGHT0, GL_AMBIENT, mSky->ambient() );
	glLight( GL_LIGHT0, GL_DIFFUSE, mSky->diffuse() );
	glLight( GL_LIGHT0, GL_SPECULAR, mSky->specular() );
	glEnable( GL_LIGHT0 );

	int light = 1;
	QList< ALightSource * >::iterator i;
	for( i = mLightSources.begin(); i != mLightSources.end(); ++i )
	{
		(*i)->updateLightSource( GL_LIGHT0+light );
		light++;
	}

	glFog( GL_FOG_COLOR, mSky->baseColor() );
	glFog( GL_FOG_START, scene()->eye()->farPlane()*0.9f );
	glFog( GL_FOG_END, scene()->eye()->farPlane()*1.1f );
}


void World::drawSelfPost()
{
	mSplatterSystem->draw( modelViewMatrix() );
}


void World::SplatterInteractor::particleInteraction( const double & delta, ParticleSystem::Particle & particle )
{
	bool belowWater = false;
	bool belowGround = false;

	if( particle.position().y() - mWorld.landscape()->waterHeight() < -mWorld.splatterSystem()->particleSystem()->size()/2.0f )
		belowWater = true;
	if( mWorld.landscape()->terrain()->getHeightAboveGround( particle.position() ) < -mWorld.splatterSystem()->particleSystem()->size()/2.0f )
		belowGround = true;

	if( belowWater )
	{
		particle.rVelocity() -= (mWorld.splatterSystem()->particleSystem()->gravity()/1.1) * delta;
	}

	if( belowGround )
	{
		particle.setLife( 0.0f );
		if( !belowWater && SplatterQuality::maximum() == SplatterQuality::HIGH )
			mWorld.splatterSystem()->splat( particle.position(), mWorld.splatterSystem()->particleSystem()->size() * RandomNumber::minMax( 0.5f, 2.0f ) );
	}
}
