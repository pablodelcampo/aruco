#include "OgreAppLogic.h"
#include "OgreApp.h"
#include <OGRE/Ogre.h>

using namespace Ogre;

OgreAppLogic::OgreAppLogic() : mApplication(0), mAR(this)
{
	// ogre
	mSceneMgr		= 0;
	mViewport		= 0;
	mCamera         = 0;
	mCameraNode     = 0;
	mObjectNode     = 0;


	mOISListener.mParent = this;
}

OgreAppLogic::~OgreAppLogic()
{}

// preAppInit
bool OgreAppLogic::preInit(const Ogre::StringVector &commandArgs)
{

	return true;
}

// postAppInit
bool OgreAppLogic::init(string videoInput ,string cameraParamsFile ,float markerSize )
{	
	pause = false;
	//INIT AR
	//INIT Realidad aumentada
	mAR.init(videoInput,cameraParamsFile,markerSize);
	mAR.createTexture("CameraTexture");

	//Create Camera Material
	MaterialPtr material = MaterialManager::getSingleton().create("CameraMaterial", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::Technique *technique = material->createTechnique();
	technique->createPass();
	material->getTechnique(0)->getPass(0)->setLightingEnabled(false);
	material->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
	material->getTechnique(0)->getPass(0)->createTextureUnitState("CameraTexture");
	
	//INIT OGRE
	createSceneManager();
	createViewport();
	createCamera();
	//createScene();

	createCameraBackground();
	//createCameraPlane(640, 480, 45000.0f);	
	
	mApplication->getKeyboard()->setEventCallback(&mOISListener);
	mApplication->getMouse()->setEventCallback(&mOISListener);
	
	mAR.init_ogre();

	return true;
}

bool OgreAppLogic::preUpdate(Ogre::Real deltaTime)
{
	return true;
}

bool OgreAppLogic::update(Ogre::Real deltaTime)
{
	if (!pause) mAR.update();
	bool result = processInputs(deltaTime);
	return result;
}

void OgreAppLogic::shutdown(void)
{
	if(mSceneMgr)
		mApplication->getOgreRoot()->destroySceneManager(mSceneMgr);
	mSceneMgr = 0;
}

void OgreAppLogic::postShutdown(void)
{

}

//--------------------------------- Init --------------------------------

void OgreAppLogic::createSceneManager(void)
{
	mSceneMgr = mApplication->getOgreRoot()->createSceneManager(ST_GENERIC, "SceneManager");
}

void OgreAppLogic::createViewport(void)
{
	//mViewport = mApplication->getRenderWindow()->addViewport(0);
}

void OgreAppLogic::createCamera(void)
{
	mCamera = mSceneMgr->createCamera("camera");
	mCamera->setNearClipDistance(0.5);
	mCamera->setFarClipDistance(50000);
	mCamera->setPosition(0, 0, 0);
	mCamera->lookAt(0, 0, 1);
	//mCamera->setFOVy(Degree(40)); //FOVy camera Ogre = 40°
	//mCamera->setAspectRatio((float) mViewport->getActualWidth() / (float) mViewport->getActualHeight());
	//mViewport->setCamera(mCamera);
	mViewport = mApplication->getRenderWindow()->addViewport(mCamera);

	mCameraNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("cameraNode");
	//mCameraNode->setPosition(0, 1700, 0);//
	//mCameraNode->lookAt(Vector3(0, 1700, -1), Node::TS_WORLD);//
	mCameraNode->attachObject(mCamera);
	//mCameraNode->setFixedYawAxis(true, Vector3::UNIT_Y);//
}

void OgreAppLogic::createScene(void)
{
	//mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox");

	Ogre::Entity* ent = mSceneMgr->createEntity("Sinbad.mesh");	//1x1_cube.mesh //Sinbad.mesh //axes.mesh

	mObjectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("cube");
	mObjectNode->setOrientation(Quaternion(Degree(180.f), Vector3::UNIT_Y));
	Ogre::Real scale = 15;
	mObjectNode->setPosition(0, 0, 15*scale);
	mObjectNode->setScale(Ogre::Vector3::UNIT_SCALE*scale);
	mObjectNode->attachObject(ent);

}



void OgreAppLogic::createCameraPlane(int width, int height, Ogre::Real _distanceFromCamera)
{
	// Create a prefab plane dedicated to display video
	float videoAspectRatio = width / (float) height;

	float planeHeight = 2 * _distanceFromCamera * Ogre::Math::Tan(Degree(26)*0.5); //FOVy webcam = 26° (intrinsic param)
	float planeWidth = planeHeight * videoAspectRatio;

	Plane p(Vector3::UNIT_Z, 0.0);
	MeshManager::getSingleton().createPlane("VerticalPlane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, p , planeWidth, planeHeight, 1, 1, true, 1, 1, 1, Vector3::UNIT_Y);
	Entity* planeEntity = mSceneMgr->createEntity("VideoPlane", "VerticalPlane"); 
	planeEntity->setMaterialName("CameraMaterial");
	planeEntity->setRenderQueueGroup(RENDER_QUEUE_WORLD_GEOMETRY_1);

	// Create a node for the plane, inserts it in the scene
	Ogre::SceneNode* node = mCameraNode->createChildSceneNode("planeNode");
	node->attachObject(planeEntity);

	// Update position    
	Vector3 planePos = mCamera->getPosition() + mCamera->getDirection() * _distanceFromCamera;
	node->setPosition(planePos);

	// Update orientation
	node->setOrientation(mCamera->getOrientation());
}


void OgreAppLogic::createCameraBackground()
{
	// Create background rectangle covering the whole screen
	Rectangle2D* rect = new Rectangle2D(true);
	rect->setCorners(-1.0, 1.0, 1.0, -1.0);
	rect->setMaterial("CameraMaterial");
 
	// Render the background before everything else
	rect->setRenderQueueGroup(RENDER_QUEUE_BACKGROUND);
 
	// Hacky, but we need to set the bounding box to something big
	// Use infinite AAB to always stay visible
	AxisAlignedBox aabInf;
	aabInf.setInfinite();
	rect->setBoundingBox(aabInf);
 
	// Attach background to the scene
	SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode("Background");
	node->attachObject(rect);
}



//--------------------------------- update --------------------------------

bool OgreAppLogic::processInputs(Ogre::Real deltaTime)
{
	OIS::Keyboard *keyboard = mApplication->getKeyboard();
	if(keyboard->isKeyDown(OIS::KC_ESCAPE))
	{
		return false;
	}
	if(keyboard->isKeyDown(OIS::KC_SPACE))
	{
	  pause = !pause;
	  
	}
	return true;
}

bool OgreAppLogic::OISListener::mouseMoved( const OIS::MouseEvent &arg )
{
	return true;
}

bool OgreAppLogic::OISListener::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	return true;
}

bool OgreAppLogic::OISListener::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	return true;
}

bool OgreAppLogic::OISListener::keyPressed( const OIS::KeyEvent &arg )
{
	return true;
}

bool OgreAppLogic::OISListener::keyReleased( const OIS::KeyEvent &arg )
{
	return true;
}
