#ifndef ARWrapper_H_
#define  ARWrapper_H_


#include <iostream>
#include <OGRE/Ogre.h>
#include <aruco/markerdetector.h>
#include <aruco/cameraparameters.h>

class OgreAppLogic;



#define VISOBJECTMAX 64

class ARWrapper
{
public:
    ARWrapper(OgreAppLogic *);
    ~ARWrapper();

    void init(string videoPath,string cameraParamsFile,float markerSize) throw(cv::Exception);
    void init_ogre();
    
    void createTexture(const std::string name);

    bool update();



protected:
  OgreAppLogic *owner;
     int mWidth;
    int mHeight;
    size_t mBufferSize;
    
    Ogre::SceneNode *vis_object[VISOBJECTMAX];
    Ogre::Entity    *vis_entity[VISOBJECTMAX];
    Ogre::ManualObject *vis_geometry[VISOBJECTMAX];

    unsigned char* mBuffer;

    Ogre::TexturePtr mTexture;
    Ogre::PixelBox mPixelBox;


    cv::Mat TheInputImage,TheInputImageUnd;
    cv::VideoCapture TheVideoCapturer;
    aruco::MarkerDetector MDetector;
    aruco::CameraParameters CameraParams,CameraParamsUnd ;
    float _markerSize;
    vector<aruco::Marker> TheMarkers;


};
#endif
