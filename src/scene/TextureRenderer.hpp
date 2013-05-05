#ifndef TEXRENDERER_INCLUDED
#define TEXRENDERER_INCLUDED


#include <GLWidget.hpp>


///FBO texture renderer.
class TextureRenderer
{
	private:
		GLuint mFrameBuffer;
		GLuint mDepthBuffer;
		GLuint mTex;
		GLuint mDepth;
		GLWidget * mGLWidget;
		bool mHasDepthBuffer;
		QSize mSize;

	public:
		TextureRenderer( GLWidget * glWidget, const QSize & size, bool depthBuffer );
		~TextureRenderer();

		void bind();
		void release();

		GLuint texID() const { return mTex; }
		GLuint depthID() const { return mDepth; }
		bool hasDepthBuffer() const { return mHasDepthBuffer; }
		const QSize & size() const { return mSize; }
};


#endif