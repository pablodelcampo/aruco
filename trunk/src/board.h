/*****************************
Copyright 2011 Rafael Muñoz Salinas. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Rafael Muñoz Salinas ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Rafael Muñoz Salinas OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Rafael Muñoz Salinas.
********************************/
#ifndef _Aruco_board_h
#define _Aruco_board_h
//#include <opencv2/opencv.hpp>
//#include <string>
//#include <vector>
//#include "exports.h"
#include "marker.h"
//using namespace std;
namespace aruco
{
/**
 * @brief 3d representation of a marker.
 *
 * Example of detailed description.
 */
struct ARUCO_EXPORTS MarkerInfo:public std::vector<cv::Point3f>
{
  MarkerInfo() {}
  MarkerInfo(int _id)
  {
    id=_id;
  }
  MarkerInfo(const MarkerInfo&MI): std::vector<cv::Point3f>(MI)
  {
    id=MI.id;
  }
  MarkerInfo & operator=(const MarkerInfo&MI)
  {
    std::vector<cv::Point3f> ::operator=(MI);
    id=MI.id;
    return *this;
  }
  int id;//maker id
};

/**\brief This class defines a board with several markers.
 * A Board contains several markers so that they are more robustly detected.
 *
 * In general, a board is a set of markers. So BoardConfiguration is only a list
 * of the id of the markers along with the position of their corners.
 *
 * The position of the corners can be specified either in pixels (in a non-specific size) or
 * in meters.
 *
 * The first is the typical case in which you generate the image of  board  and the print it.
 * Since you do not know in advance the real size of the markers, their corners are specified
 * in pixels, and then, the translation to meters can be made once you know the real size.
 *
 * On the other hand, you may want to have the information of your boards in meters. The
 * BoardConfiguration allows you to do so.
 *
 * The point is in the mInfoType variable. It can be either PIX or METERS according to your needs.
 *
*/


class ARUCO_EXPORTS  BoardConfiguration: public std::vector<MarkerInfo>
{
  friend class Board;
  public:

    /*! @brief Indicates if the data in MakersInfo is expressed in meters or in pixels
     * so as to do conversion internally.
     */
    enum MarkerInfoType
    {
      NONE=-1,
      PIX=0,
      METERS=1
    };

    int mInfoType; ///< indicates the type of data in MakersInfo (to do conversion internally)

    /*!
     */
    BoardConfiguration();

    /*!
    */
    BoardConfiguration(const BoardConfiguration  &T);

    /*!
    */
    BoardConfiguration & operator=(const BoardConfiguration  &T);

    /*! @brief Saves the board info to a file.
    */
    void saveToFile(std::string sfile) const throw (cv::Exception);

    /** @brief Reads board info from a file.
    */
    void readFromFile(std::string sfile)throw (cv::Exception);
    /**Indicates if the corners are expressed in meters
     */
    bool isExpressedInMeters()const
    {
      return mInfoType==METERS;
    }
    /**Indicates if the corners are expressed in meters
     */
    bool isExpressedInPixels()const
    {
      return mInfoType==PIX;
    }
    /**Returns the index of the marker with id indicated, if is in the list
     */
    int getIndexOfMarkerId(int id)const;
    /**Returns the Info of the marker with id specified. If not in the set, throws exception
     */
    const MarkerInfo& getMarkerInfo(int id)const throw (cv::Exception);

    /**@brief Set in the list passed the set of the ids.
     * @param[out] ids Output vector with ids.
     */
    void getIdList(vector<int> &ids,bool append=true)const;
  private:
    /**Saves the board info to a file
    */
    void saveToFile(cv::FileStorage &fs) const throw (cv::Exception);
    /**Reads board info from a file
    */
    void readFromFile(cv::FileStorage &fs)throw (cv::Exception);
};

/** @brief Represetation of a board with several Marker (s)
*/
class ARUCO_EXPORTS Board:public std::vector<Marker>
{
  public:
    /// @brief Constructor
    Board()
    {
      Rvec.create(3,1,CV_32FC1);
      Tvec.create(3,1,CV_32FC1);
      for (int i=0; i<3; i++)
        Tvec.at<float>(i,0)=Rvec.at<float>(i,0)=-999999;
    }

   /////////////// ACCESSORS //////////////////////

    const cv::Mat & getRvec() const
    {
        return Rvec;
    }

    cv::Mat & getRvec()
    {
        return Rvec;
    }

    const cv::Mat & getTvec() const
    {
        return Tvec;
    }

    cv::Mat & getTvec()
    {
        return Tvec;
    }

    const BoardConfiguration & getBoardConf() const
    {
        return conf;
    }

    /**Save this from a file
     */
    void saveToFile(std::string filePath) const throw(cv::Exception);

    /////////////// MUTATORS //////////////////////

    void setBoardConf(const BoardConfiguration &bc)
    {
        conf = bc;
    }

    /**
     * Returns position vector and orientation quaternion for an Ogre scene node or entity.
     *  Use:
     * ...
     * Ogre::Vector3 ogrePos (position[0], position[1], position[2]);
     * Ogre::Quaternion  ogreOrient (orientation[0], orientation[1], orientation[2],
     * orientation[3]);
     * mySceneNode->setPosition( ogrePos  );
     * mySceneNode->setOrientation( ogreOrient  );
     * ...
     * \todo check if the method can be const
     */
    void OgreGetPoseParameters(  double position[3], double orientation[4] )throw(cv::Exception);

    /** @brief Given the extrinsic camera parameters returns the GL_MODELVIEW matrix for opengl.
     *
     * Setting this matrix, the reference corrdinate system will be set in this board
     * \todo check if the method can be const
     */
    void glGetModelViewMatrix(double modelview_matrix[16])throw(cv::Exception);

    /**Read  this from a file
     */
    void readFromFile(std::string filePath)throw(cv::Exception);

  private:
    BoardConfiguration conf;
    //matrices of rotation and translation respect to the camera
    cv::Mat Rvec,Tvec;
};

}

#endif
