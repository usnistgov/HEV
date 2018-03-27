#ifndef OSG_UTIL_H
#define OSG_UTIL_H

#include <osg/Drawable>

namespace hyd {

osg::Drawable* unit_cube(osg::Vec3f const& scale = osg::Vec3f(1.f, 1.f, 1.f),
                         osg::Vec3f const& translation = osg::Vec3f(0.f, 0.f, 0.f),
                         osg::Vec4f const& color = osg::Vec4f(1.f, 1.f, 1.f, 1.f));

} // namespace hyd

#endif // OSG_UTIL_H

