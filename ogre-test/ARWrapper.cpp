#include "ARWrapper.h"
#include <OGRE/OgreTextureManager.h>
#include <OGRE/OgreMaterialManager.h>
#include <OGRE/OgreTechnique.h>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <aruco/cvdrawingutils.h>

#include "OgreAppLogic.h"

ARWrapper::ARWrapper(OgreAppLogic *_owner) : owner(_owner)
{
}

ARWrapper::~ARWrapper()
{
}

void ARWrapper::init(string videoPath,string cameraParamsFile,float markerSize)throw(cv::Exception)
{
  if (videoPath=="")
    TheVideoCapturer.open(0);//open first camera
    else 
      TheVideoCapturer.open(videoPath);//open first camera
    if (!TheVideoCapturer.isOpened())
    {
        cerr<<"COULD Not open input video"<<endl;
        exit(0);
    }
    TheVideoCapturer.grab();
    TheVideoCapturer.retrieve(TheInputImage);
  if (cameraParamsFile!="")
    try {
        CameraParams.readFromXMLFile(cameraParamsFile);
        cout<<"PARAMS="<<CameraParams.CameraMatrix<<" "<<CameraParams.Distorsion<<endl;
        //(CameraParamsUnd.setParams(CameraParams.CameraMatrix,cv::Mat::zeros(1,4,CV_32F),CameraParams.CamSize);
    } catch (std::exception &ex) {
        cout<<ex.what()<<endl;
        exit(0);
    }
    cv::undistort(TheInputImage,TheInputImageUnd,CameraParams.CameraMatrix,CameraParams.Distorsion);
    _markerSize=markerSize;
    //El ancho y el alto de una camara (No es accesible)
    mWidth=TheInputImage .cols;
    mHeight=TheInputImage.rows;
    mBuffer=TheInputImageUnd.ptr<uchar>(0);
    //Contenedor de Pixel (Convierte cualquier formato a una imagen para ogre)
    //Defino el formato de la imagen (Probar si vale BAYER Nearest)
    mPixelBox = Ogre::PixelBox(mWidth, mHeight, 1, Ogre::PF_R8G8B8, mBuffer);  
}

void ARWrapper::init_ogre()
{
    const float scale = 0.00025f;
    Ogre::Light *L = owner->mSceneMgr->createLight("L");
    L->setPosition(0, 1, -10);
    owner->mSceneMgr->getRootSceneNode()->attachObject(L);
    
    
    
    
    const float M2  = _markerSize * 0.5f;
    
    for (unsigned int  k = 0; k < VISOBJECTMAX; ++k)
    {
      std::stringstream EN; EN << "esdasd"<<k;
      //vis_entity[k] = owner->mSceneMgr->createEntity(EN.str(), "cube.mesh");
      //vis_entity[k]->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
      
      
      vis_geometry[k] = owner->mSceneMgr->createManualObject(EN.str());
      vis_geometry[k]->begin("blank", Ogre::RenderOperation::OT_LINE_LIST);
      /*vis_geometry[k]->position(-M2,-M2,0);
      vis_geometry[k]->colour(1, 0, 0);
      vis_geometry[k]->position(M2,-M2,0);
      vis_geometry[k]->colour(1, 1, 0);
      vis_geometry[k]->position(M2,M2,0);
      vis_geometry[k]->colour(1, 1, 0);
      vis_geometry[k]->position(-M2,M2,0);
      vis_geometry[k]->colour(1, 1, 0);
      vis_geometry[k]->position(-M2,-M2,0);
      vis_geometry[k]->colour(1, 0, 0);*/
      
      vis_geometry[k]->position(M2,0,0);
      vis_geometry[k]->colour(1, 0, 0);
      vis_geometry[k]->position(0,0,0);
      vis_geometry[k]->colour(1, 0, 0);
      
      vis_geometry[k]->position(0,M2,0);
      vis_geometry[k]->colour(0, 1, 0);
      vis_geometry[k]->position(0,0,0);
      vis_geometry[k]->colour(1, 1, 0);
      
      vis_geometry[k]->position(0,0,M2);
      vis_geometry[k]->colour(0, 0, 1);
      vis_geometry[k]->position(0,0,0);
      vis_geometry[k]->colour(0, 0, 1);
      
      vis_geometry[k]->end();
      
      vis_object[k] = owner->mSceneMgr->getRootSceneNode()->createChildSceneNode();
      vis_object[k]->setPosition(0, 0, 0);
      //vis_object[k]->setScale(scale,scale,scale);
      vis_object[k]->attachObject(vis_geometry[k]);
      
    } 
}

void ARWrapper::createTexture(const std::string name)
{
    mTexture = Ogre::TextureManager::getSingleton().createManual(
                   name,
                   Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                   Ogre::TEX_TYPE_2D,
                   mWidth,
                   mHeight,
                   0,
                   Ogre::PF_R8G8B8,
                   Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
}

bool ARWrapper::update()
{
    bool updated = false;

    cv::Mat  cameraFrame;
    if (TheVideoCapturer.grab())//¿Hay alguna imagen para procesar?
    {
        //Recorta la imagen de 1 camara (OPTIMIZAR)
        //deberia de poder acceder a una camara para que me de la imagen directamente
        TheVideoCapturer.retrieve ( TheInputImage );
        cv::undistort(TheInputImage,TheInputImageUnd,CameraParams.CameraMatrix,CameraParams.Distorsion);

        MDetector.detect(TheInputImageUnd,TheMarkers,CameraParams,_markerSize);
        /*for (unsigned int i=0;i<TheMarkers.size();i++) {
            cout<<TheMarkers[i]<<endl;
            TheMarkers[i].draw(TheInputImageUnd,cv::Scalar(0,0,255),2);
        }*/
        CameraParamsUnd=CameraParams;
        CameraParamsUnd.Distorsion=cv::Mat::zeros(4,1,CV_32F);

//         for (unsigned int i=0;i<TheMarkers.size();i++) {
//             aruco::CvDrawingUtils::draw3dCube(TheInputImageUnd,TheMarkers[i],CameraParamsUnd);
//             aruco::CvDrawingUtils::draw3dAxis(TheInputImageUnd,TheMarkers[i],CameraParamsUnd);
//         }
        
        
        // -------------------------------------------------
        
	owner->mCamera->setNearClipDistance(0.01f);
        owner->mCamera->setFarClipDistance(10.0f);
       
        
        double _pm[16];
	CameraParamsUnd.glGetProjectionMatrix(TheInputImage.size(),cv::Size(mWidth,mHeight),_pm,0.01,10);
	
	
	Ogre::Matrix4 PM(_pm[0],_pm[1],_pm[2],_pm[3],_pm[4],_pm[5],_pm[6],_pm[7],_pm[8],
			 _pm[9],_pm[10],_pm[11],_pm[12],_pm[13],_pm[14],_pm[15]);
	PM = PM.transpose();
        owner->mCamera->setCustomProjectionMatrix(true, PM);
	owner->mCamera->setCustomViewMatrix(true, Ogre::Matrix4::IDENTITY);
        
	for (unsigned int  k = 0; k < VISOBJECTMAX; ++k)
	{
	  vis_object[k]->setVisible(false);
	}
	
	double _mv[16];
        for (unsigned int i=0;i<TheMarkers.size();i++) {
	    TheMarkers[i].glGetModelViewMatrix(_mv);
	    Ogre::Matrix4 VM(_mv[0],_mv[1],_mv[2],_mv[3],_mv[4],_mv[5],_mv[6],_mv[7],_mv[8],
			 _mv[9],_mv[10],_mv[11],_mv[12],_mv[13],_mv[14],_mv[15]);
	    
	    VM = VM.transpose();
	    
	    const float M2  = _markerSize * 0.5f;
	    vis_geometry[i]->clear();
	    vis_geometry[i]->begin("blank", Ogre::RenderOperation::OT_LINE_LIST);
 
      
	    vis_geometry[i]->position(VM * Ogre::Vector3(M2,0,0));
	    //vis_geometry[i]->colour(1, 0, 0);
	    vis_geometry[i]->position(VM * Ogre::Vector3(0,0,0));
	    //vis_geometry[i]->colour(1, 0, 0);
	    
	    vis_geometry[i]->position(VM * Ogre::Vector3(0,M2,0));
	    //vis_geometry[i]->colour(0, 1, 0);
	    vis_geometry[i]->position(VM * Ogre::Vector3(0,0,0));
	    //vis_geometry[i]->colour(1, 1, 0);
	    
	    vis_geometry[i]->position(VM * Ogre::Vector3(0,0,M2));
	    //vis_geometry[i]->colour(0, 0, 1);
	    vis_geometry[i]->position(VM * Ogre::Vector3(0,0,0));
	    //vis_geometry[i]->colour(0, 0, 1);
	    
	    vis_geometry[i]->end();
	    
	    
	    
	    //vis_object[i]->setOrientation(Q);
	    
	    
	    vis_object[i]->setVisible(true);
	}
        
        // -------------------------------------------------

        if (!mTexture.isNull())
        {
            //Pedimos a ogre que actualice la imagen desde el PixelBox
            Ogre::HardwarePixelBufferSharedPtr pixelBuffer = mTexture->getBuffer();
            pixelBuffer->blitFromMemory(mPixelBox);
            updated = true;
        }
        //char c;cin>>c;
    }
    return updated;
}

