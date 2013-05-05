#ifndef SCENE_INCLUDED
#define SCENE_INCLUDED

#include "objects/Eye.hpp"

#include <QGraphicsScene>
#include <QElapsedTimer>
#include <QRectF>


class QPainter;
class QGraphicsItem;
class QGraphicsProxyWidget;
class QGLWidget;
class QKeyEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;

class GLWidget;
class MouseListener;
class KeyListener;



class Scene : public QGraphicsScene
{
	Q_OBJECT
public:
	explicit Scene( GLWidget * glWidget, QObject * parent = 0 );
	~Scene();

	// Overrides:
	void drawBackground( QPainter * painter, const QRectF & rect );
	QGraphicsProxyWidget * addWidget( QWidget * widget, Qt::WindowFlags wFlags = 0 );

	GLWidget * glWidget() { return mGLWidget; }
	AObject * root() { return mEye; }
	Eye * eye() { return mEye; }
	void setEye( Eye * eye ) { mEye = eye; }
	void addKeyListener( KeyListener * listener ) { mKeyListeners.append( listener ); }
	void addMouseListener( MouseListener * listener ) { mMouseListeners.append( listener ); }
	void removeKeyListener( KeyListener * listener ) { mKeyListeners.removeOne( listener ); }
	void removeMouseListener( MouseListener * listener ) { mMouseListeners.removeOne( listener ); }

protected:
	// Overrides:
	void keyPressEvent( QKeyEvent * event );
	void keyReleaseEvent( QKeyEvent * event );
	void mousePressEvent( QGraphicsSceneMouseEvent * event );
	void mouseReleaseEvent( QGraphicsSceneMouseEvent * event );
	void mouseMoveEvent( QGraphicsSceneMouseEvent * event );
	void wheelEvent( QGraphicsSceneWheelEvent * wheelEvent );

private:
	GLWidget * mGLWidget;
	QElapsedTimer mElapsedTimer;
	float mDelta;
	int mFrameCountSecond;
	int mFramesPerSecond;
	QPoint mDrag;
	bool mDragging;
	QFont mFont;

	bool mForwardPressed;
	bool mBackwardPressed;
	bool mLeftPressed;
	bool mRightPressed;
	bool mUpPressed;
	bool mDownPressed;
	bool mSpeedPressed;

	QList<MouseListener*> mMouseListeners;
	QList<KeyListener*> mKeyListeners;
	Eye * mEye;

private slots:
	void secondPassed();
};


#endif