/*
 * File: EulerDegreeHVS.cpp
 * Author: Przemysław Bągard
 * Created: 2020-12-23
 *
 */

#include "EulerDegreeHVS.h"

#include <OgreMatrix3.h>

EulerDegreeHVS::EulerDegreeHVS()
{

}

EulerDegreeHVS::EulerDegreeHVS(float horizontal, float vertical, float side)
    : mHorizontal(horizontal)
    , mVertical(vertical)
    , mSide(side)
{

}

void EulerDegreeHVS::setDegrees(float horizontal, float vertical, float side)
{
    mHorizontal = Ogre::Degree(horizontal);
    mVertical = Ogre::Degree(vertical);
    mSide = Ogre::Degree(side);
}

Ogre::Quaternion EulerDegreeHVS::toQuaternion() const
{
    Ogre::Matrix3 mat;
    mat.FromEulerAnglesYXZ(mHorizontal, mVertical, mSide);
    Ogre::Quaternion quat;
    quat.FromRotationMatrix(mat);
    return quat;
}

void EulerDegreeHVS::fromQuaternion(const Ogre::Quaternion& quat)
{
    Ogre::Matrix3 mat;
    quat.ToRotationMatrix(mat);
    Ogre::Radian h, v, s;
    mat.ToEulerAnglesYXZ(h, v, s);
    mHorizontal = h;
    mVertical = v;
    mSide = s;
}

EulerDegreeHVS EulerDegreeHVS::createFromQuaternion(const Ogre::Quaternion& quat)
{
    Ogre::Matrix3 mat;
    quat.ToRotationMatrix(mat);
    Ogre::Radian h, v, s;
    mat.ToEulerAnglesYXZ(h, v, s);

    EulerDegreeHVS hvs;
    hvs.mHorizontal = h;
    hvs.mVertical = v;
    hvs.mSide = s;
    return hvs;
}
