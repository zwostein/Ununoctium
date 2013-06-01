#include "Sky.hpp"

#include <utility/interpolate.hpp>
#include <resource/Shader.hpp>
#include <scene/Scene.hpp>

#include <QGLShaderProgram>
#include <QSettings>
#include <QString>
#include <QImage>
#include <GL/glu.h>
#include <math.h>
#include <QDebug>


static void TexImage( QGLWidget * glWidget, GLenum target, QString path )
{
	QImage image( path );
	if( image.isNull() )
	{
		qFatal( "\"%s\" not found!", path.toLocal8Bit().constData() );
	}
	QImage glImage = QGLWidget::convertToGLFormat( image );
	glTexImage2D( target, 0, GL_RGBA8, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits() );
}


const GLfloat Sky::sCubeVertices[] =
{
	-1.0,  1.0,  1.0,
	-1.0, -1.0,  1.0,
	 1.0, -1.0,  1.0,
	 1.0,  1.0,  1.0,
	-1.0,  1.0, -1.0,
	-1.0, -1.0, -1.0,
	 1.0, -1.0, -1.0,
	 1.0,  1.0, -1.0
};

QGLBuffer Sky::sCubeVertexBuffer;


const GLushort Sky::sCubeIndices[] =
{
	0, 1, 2, 3,
	3, 2, 6, 7,
	7, 6, 5, 4,
	4, 5, 1, 0,
	0, 3, 7, 4,
	5, 6, 2, 1
};

QGLBuffer Sky::sCubeIndexBuffer;


Sky::Sky( Scene * scene, QString name ) :
	AObject(scene),
	mTimeOfDay(0)
{
	QSettings s( "./data/sky/"+name+"/sky.ini", QSettings::IniFormat );

	s.beginGroup( "Sky" );
		QString skyDomePath = "./data/sky/"+name+"/"+s.value( "domeMapPath", "dome.png").toString();
		mAxis = QVector3D(
			s.value( "axisX", 1.0f ).toFloat(),
			s.value( "axisY", 0.0f ).toFloat(),
			s.value( "axisZ", 1.0f ).toFloat()
		).normalized();
		mSunInitialDir = QVector3D(
			s.value( "sunInitialX", 0.0f ).toFloat(),
			s.value( "sunInitialY", 0.0f ).toFloat(),
			s.value( "sunInitialZ", -1.0f ).toFloat()
		).normalized();
		mSunSpotPower = s.value( "sunSpotPower", 10.0f ).toFloat();
		mDiffuseFactorDay = s.value( "diffuseFactorDay", 3.0f ).toFloat();
		mDiffuseFactorNight = s.value( "diffuseFactorNight", 0.2f ).toFloat();
		mDiffuseFactorMax = s.value( "diffuseFactorMax", 2.0f ).toFloat();
		mSpecularFactorDay = s.value( "specularFactorDay", 2.0f ).toFloat();
		mSpecularFactorNight = s.value( "specularFactorNight", 0.3f ).toFloat();
		mSpecularFactorMax = s.value( "specularFactorMax", 2.0f ).toFloat();
		mAmbientFactorDay = s.value( "ambientFactorDay", 0.3f ).toFloat();
		mAmbientFactorNight = s.value( "ambientFactorNight", 0.1f ).toFloat();
		mAmbientFactorMax = s.value( "ambientFactorMax", 0.2f ).toFloat();
	s.endGroup();

	s.beginGroup( "StarMap" );
		QString starMapPathPX = "./data/sky/"+name+"/"+s.value( "positiveX", "star.px.png").toString();
		QString starMapPathNX = "./data/sky/"+name+"/"+s.value( "negativeX", "star.nx.png").toString();
		QString starMapPathPY = "./data/sky/"+name+"/"+s.value( "positiveY", "star.py.png").toString();
		QString starMapPathNY = "./data/sky/"+name+"/"+s.value( "negativeY", "star.ny.png").toString();
		QString starMapPathPZ = "./data/sky/"+name+"/"+s.value( "positiveZ", "star.pz.png").toString();
		QString starMapPathNZ = "./data/sky/"+name+"/"+s.value( "negativeZ", "star.nz.png").toString();
	s.endGroup();

	if( !sCubeVertexBuffer.isCreated() )
	{
		sCubeVertexBuffer = QGLBuffer( QGLBuffer::VertexBuffer );
		sCubeVertexBuffer.create();
		sCubeVertexBuffer.bind();
		sCubeVertexBuffer.setUsagePattern( QGLBuffer::StaticDraw );
		sCubeVertexBuffer.allocate( sCubeVertices, sizeof(sCubeVertices) );
	}

	if( !sCubeIndexBuffer.isCreated() )
	{
		sCubeIndexBuffer = QGLBuffer( QGLBuffer::IndexBuffer );
		sCubeIndexBuffer.create();
		sCubeIndexBuffer.bind();
		sCubeIndexBuffer.setUsagePattern( QGLBuffer::StaticDraw );
		sCubeIndexBuffer.allocate( sCubeIndices, sizeof(sCubeIndices) );
	}
	
	mDomeShader = new Shader( scene->glWidget(), "skydome" );
	mDomeShader_sunDir = mDomeShader->program()->uniformLocation( "sunDir" );
	mDomeShader_timeOfDay = mDomeShader->program()->uniformLocation( "timeOfDay" );
	mDomeShader_sunSpotPower = mDomeShader->program()->uniformLocation( "sunSpotPower" );
	mDomeShader_diffuseMap = mDomeShader->program()->uniformLocation( "diffuseMap" );

	mDomeImage = QImage( skyDomePath );
	if( mDomeImage.isNull() )
	{
		qFatal( "\"%s\" not found!", skyDomePath.toLocal8Bit().constData() );
	}

	mDomeMap =  scene->glWidget()->bindTexture( mDomeImage );
	if( mDomeMap >= 0 )
	{
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, mDomeMap );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	}

	mStarCubeShader = new Shader( scene->glWidget(), "cube" );

	glGenTextures( 1, &mStarCubeMap );
	glBindTexture( GL_TEXTURE_CUBE_MAP, mStarCubeMap );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	TexImage( scene->glWidget(), GL_TEXTURE_CUBE_MAP_POSITIVE_X, starMapPathPX );
	TexImage( scene->glWidget(), GL_TEXTURE_CUBE_MAP_NEGATIVE_X, starMapPathNX );
	TexImage( scene->glWidget(), GL_TEXTURE_CUBE_MAP_POSITIVE_Y, starMapPathPY );
	TexImage( scene->glWidget(), GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, starMapPathNY );
	TexImage( scene->glWidget(), GL_TEXTURE_CUBE_MAP_POSITIVE_Z, starMapPathPZ );
	TexImage( scene->glWidget(), GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, starMapPathNZ );
}


Sky::~Sky()
{
	scene()->glWidget()->deleteTexture( mDomeMap );
	scene()->glWidget()->deleteTexture( mStarCubeMap );
	delete mDomeShader;
	delete mStarCubeShader;
	sCubeVertexBuffer.destroy();
	sCubeIndexBuffer.destroy();
}


void Sky::setTimeOfDay( const float & timeOfDay )
{
	mTimeOfDay = timeOfDay;
	if( mTimeOfDay > 1.0f )
		mTimeOfDay -= floorf( mTimeOfDay );
	else if( mTimeOfDay < 0.0f )
		mTimeOfDay -= floorf( mTimeOfDay );
}


void Sky::updateSelf( const double & delta )
{
	float angle = mTimeOfDay*(M_PI*2.0f);
	QMatrix4x4 m;
	m.setToIdentity();
	m.rotate( angle*(180.0/M_PI), mAxis );
	mSunDirection = m.map( mSunInitialDir );

	float skyMapTime = mTimeOfDay * (float)mDomeImage.width() - 0.5f;

	int xA = (int)floorf( skyMapTime );
	if( xA < 0 )
		xA += mDomeImage.width();
	QRgb colorA = mDomeImage.pixel( xA, mDomeImage.height()-1 );

	int xB = xA + 1;
	if( xB >= mDomeImage.width() )
		xB -= mDomeImage.width();
	QRgb colorB = mDomeImage.pixel( xB, mDomeImage.height()-1 );

	QVector4D colorAF( qRed(colorA), qGreen(colorA), qBlue(colorA), qAlpha(colorA) );
	colorAF /= 255.0f;
	QVector4D colorBF( qRed(colorB), qGreen(colorB), qBlue(colorB), qAlpha(colorB) );
	colorBF /= 255.0f;

	mBaseColor = interpolateLinear( colorAF, colorBF, skyMapTime-floorf( skyMapTime ) );

	float diffuseFactor = interpolateLinear( mDiffuseFactorDay, mDiffuseFactorNight, (float)((sunDirection().y()+1.0f)*0.5f) );
	if( diffuseFactor > mDiffuseFactorMax )
		diffuseFactor = mDiffuseFactorMax;

	float specularFactor = interpolateLinear( mSpecularFactorDay, mSpecularFactorNight, (float)((sunDirection().y()+1.0f)*0.5f) );
	if( diffuseFactor > mSpecularFactorMax )
		diffuseFactor = mSpecularFactorMax;

	float ambientFactor = interpolateLinear( mAmbientFactorDay, mAmbientFactorNight, (float)((sunDirection().y()+1.0f)*0.5f) );
	if( ambientFactor > mAmbientFactorMax )
		ambientFactor = mAmbientFactorMax;

	mDiffuse = mBaseColor.toVector3D() * diffuseFactor;
	mSpecular = mBaseColor.toVector3D() * specularFactor;
	mAmbient = mBaseColor.toVector3D().normalized() * ambientFactor;
}


void Sky::drawSelf()
{
	scene()->eye()->disableClippingPlanes();
	glPushAttrib( GL_VIEWPORT_BIT | GL_DEPTH_BUFFER_BIT );
	glPushMatrix();
	glTranslatef( scene()->eye()->position().x(), scene()->eye()->position().y(), scene()->eye()->position().z() );

	glDepthMask( GL_FALSE );
	glDepthFunc( GL_EQUAL );
	glDisable( GL_CULL_FACE );
	glDepthRange( 1.0, 1.0 );

	drawStarCube();
	drawSky();

	glPopMatrix();
	glPopAttrib();
	scene()->eye()->enableClippingPlanes();
}


void Sky::drawStarCube()
{
	glPushMatrix();
	float angle = mTimeOfDay*(360.0f);
	glRotatef( angle, mAxis.x(), mAxis.y(), mAxis.z() );
	mStarCubeShader->bind();
	sCubeVertexBuffer.bind();
	sCubeIndexBuffer.bind();
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_INDEX_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, 0 );
	glDrawElements( GL_QUADS, sizeof(sCubeIndices)/sizeof(GLushort), GL_UNSIGNED_SHORT, 0 );
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_INDEX_ARRAY );
	sCubeVertexBuffer.release();
	sCubeIndexBuffer.release();
	mStarCubeShader->release();
	glPopMatrix();
}


void Sky::drawSky()
{
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	mDomeShader->bind();
	mDomeShader->program()->setUniformValue( mDomeShader_sunDir, mSunDirection.toVector3D() );
	mDomeShader->program()->setUniformValue( mDomeShader_timeOfDay, mTimeOfDay );
	mDomeShader->program()->setUniformValue( mDomeShader_sunSpotPower, mSunSpotPower );
	mDomeShader->program()->setUniformValue( mDomeShader_diffuseMap, 0 );
	glActiveTexture( GL_TEXTURE0 );	glBindTexture( GL_TEXTURE_2D, mDomeMap );
	sCubeVertexBuffer.bind();
	sCubeIndexBuffer.bind();
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_INDEX_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, 0 );
	glDrawElements( GL_QUADS, sizeof(sCubeIndices)/sizeof(GLushort), GL_UNSIGNED_SHORT, 0 );
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_INDEX_ARRAY );
	sCubeVertexBuffer.release();
	sCubeIndexBuffer.release();
	mDomeShader->release();
	glDisable( GL_BLEND );
}
