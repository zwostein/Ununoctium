#ifndef RESOURCE_MATERIAL_INCLUDED
#define RESOURCE_MATERIAL_INCLUDED

#include "AResource.hpp"

#include <GLWidget.hpp>

#include <QVector4D>


class GLWidget;
class Shader;


class MaterialQuality
{
public:
	enum type
	{
		LOW	= 0,
		MEDIUM	= 1,
		HIGH	= 2
	};
	const static int num = 3;

	static type maximum() { return sMaximum; }
	static void setMaximum( type max ) { sMaximum = max; }

private:
	static type sMaximum;
};


class MaterialShaderVariant
{
public:
	enum type
	{
		DEFAULT		= 0,
		BLOBBING	= 1
	};
	const static int num = 2;
};


class MaterialData : public AResourceData
{
public:
	MaterialData( GLWidget * glWidget, QString name );
	virtual ~MaterialData();
	virtual bool load();
	virtual void unload();

	const QString & name() const { return mName; }
	const QVector4D & ambient() const { return mAmbient; }
	const QVector4D & diffuse() const { return mDiffuse; }
	const QVector4D & specular() const { return mSpecular; }
	const QVector4D & emission() const { return mEmission; }
	const GLfloat & shininess() const { return mShininess; }
	const QMap<QString,GLuint> & textures() const { return mTextures; }
	const QMap<QString,GLfloat> & constants() const { return mConstants; }
	const QString shaderName( MaterialQuality::type quality ) const { return mShaderNames[quality]; }

	const GLclampf & alphaTestReferenceValue() const { return mAlphaTestReferenceValue; }
	const GLenum & alphaTestFunction() const { return mAlphaTestFunction; }
	const bool & alphaTestEnabled() const { return mAlphaTestEnabled; }

private:
	GLWidget * mGLWidget;
	QString mName;

	QString mShaderNames[MaterialQuality::num];

	QVector4D mAmbient;
	QVector4D mDiffuse;
	QVector4D mSpecular;
	QVector4D mEmission;
	GLfloat mShininess;
	QMap<QString,GLuint> mTextures;
	QMap<QString,GLfloat> mConstants;
	bool mAlphaTestEnabled;
	GLclampf mAlphaTestReferenceValue;
	GLenum mAlphaTestFunction;

	static GLenum alphaTestFunctionFromString( QString name );
};


class Material : public AResource<MaterialData>
{
public:
	Material( GLWidget * glWidget, QString name, MaterialShaderVariant::type type = MaterialShaderVariant::DEFAULT );
	virtual ~Material();

	void setShader( MaterialShaderVariant::type variant );

	void overrideAmbient( const QVector4D & ambient ) { glMaterial( GL_FRONT_AND_BACK, GL_AMBIENT, ambient ); }
	void overrideDiffuse( const QVector4D & diffuse ) { glMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse ); }
	void overrideSpecular( const QVector4D & specular ) { glMaterial( GL_FRONT_AND_BACK, GL_SPECULAR, specular ); }
	void overrideEmission( const QVector4D & emission ) { glMaterial( GL_FRONT_AND_BACK, GL_EMISSION, emission ); }
	void overrideAlphaTestReferenceValue( const float & referenceValue ) { glAlphaFunc( data()->alphaTestFunction(), referenceValue ); }

	void setBlobMap( GLuint blobMap ) { mBlobMap = blobMap; }
	void setCubeMap( GLuint cubeMap ) { mCubeMap = cubeMap; }

	void bind();
	void release();

	void setDefaultQuality( MaterialQuality::type q ) { mDefaultQuality = q; }

private:
	typedef struct
	{
		Shader * shader;
		QVector< QPair<int,GLuint> > textureUnits;
		QVector< QPair<int,GLfloat> > constants;
		int blobMapUniform;
		int cubeMapUniform;
	} ShaderSet;

	GLWidget * mGLWidget;
	QString mName;

	float mOpacity;

	GLuint mBlobMap;
	GLuint mCubeMap;
	
	GLclampf mAlphaTestReferenceValueOverride;
	const GLclampf * mUsedAlphaTestReferenceValue;

	ShaderSet mShaderSet[MaterialQuality::num];
	MaterialQuality::type mDefaultQuality;
	MaterialQuality::type mBoundQuality;

	MaterialQuality::type getBindingQuality();
	void setShader( MaterialQuality::type quality, QString shaderFullName );
};


#endif
