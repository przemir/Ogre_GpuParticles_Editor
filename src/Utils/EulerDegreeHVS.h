/*
 * File: EulerDegreeHVS.h
 * Author: Przemysław Bągard
 * Created: 2020-12-23
 *
 */

#ifndef EULERDEGREEHVS_H
#define EULERDEGREEHVS_H

#include <OgreQuaternion.h>

/// Horizontal (Y axis), Vertical (X axis), Side (Z axis). In degrees.
class EulerDegreeHVS
{
public:
    EulerDegreeHVS();
    EulerDegreeHVS(float horizontal, float vertical, float side);

    Ogre::Degree mHorizontal;
    Ogre::Degree mVertical;
    Ogre::Degree mSide;

    void setDegrees(float horizontal, float vertical, float side);

    Ogre::Quaternion toQuaternion() const;
    void fromQuaternion(const Ogre::Quaternion& quat);

    static EulerDegreeHVS createFromQuaternion(const Ogre::Quaternion& quat);
};

#endif
