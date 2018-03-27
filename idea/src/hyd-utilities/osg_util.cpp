#include "osg_util.h"

#include <osg/Geometry>

namespace hyd {

osg::Drawable* unit_cube(osg::Vec3f const& scale,
                         osg::Vec3f const& translation,
                         osg::Vec4f const& color) {
    static size_t const nVertices = 24;
    static osg::Vec3f const vertices[] = {
        osg::Vec3f(-0.5,0.5,0.5),
        osg::Vec3f(-0.5,-0.5,0.5),
        osg::Vec3f(0.5,-0.5,0.5),
        osg::Vec3f(0.5,0.5,0.5),
        osg::Vec3f(-0.5,0.5,0.5),
        osg::Vec3f(-0.5,0.5,-0.5),
        osg::Vec3f(-0.5,-0.5,-0.5),
        osg::Vec3f(-0.5,-0.5,0.5),
        osg::Vec3f(0.5,-0.5,0.5),
        osg::Vec3f(0.5,-0.5,-0.5),
        osg::Vec3f(0.5,0.5,-0.5),
        osg::Vec3f(0.5,0.5,0.5),
        osg::Vec3f(0.5,0.5,0.5),
        osg::Vec3f(0.5,0.5,-0.5),
        osg::Vec3f(-0.5,0.5,-0.5),
        osg::Vec3f(-0.5,0.5,0.5),
        osg::Vec3f(-0.5,-0.5,0.5),
        osg::Vec3f(-0.5,-0.5,-0.5),
        osg::Vec3f(0.5,-0.5,-0.5),
        osg::Vec3f(0.5,-0.5,0.5),
        osg::Vec3f(-0.5,-0.5,-0.5),
        osg::Vec3f(-0.5,0.5,-0.5),
        osg::Vec3f(0.5,0.5,-0.5),
        osg::Vec3f(0.5,-0.5,-0.5),
    };

    static size_t const nNormals = 24;
    static osg::Vec3f const normals[] = {
        osg::Vec3f(0,-0,1),
        osg::Vec3f(0,-0,1),
        osg::Vec3f(0,-0,1),
        osg::Vec3f(0,-0,1),
        osg::Vec3f(-1,-0,0),
        osg::Vec3f(-1,-0,0),
        osg::Vec3f(-1,-0,0),
        osg::Vec3f(-1,-0,0),
        osg::Vec3f(1,-0,0),
        osg::Vec3f(1,-0,0),
        osg::Vec3f(1,-0,0),
        osg::Vec3f(1,-0,0),
        osg::Vec3f(0,1,0),
        osg::Vec3f(0,1,0),
        osg::Vec3f(0,1,0),
        osg::Vec3f(0,1,0),
        osg::Vec3f(0,-1,0),
        osg::Vec3f(0,-1,0),
        osg::Vec3f(0,-1,0),
        osg::Vec3f(0,-1,0),
        osg::Vec3f(0,-0,-1),
        osg::Vec3f(0,-0,-1),
        osg::Vec3f(0,-0,-1),
        osg::Vec3f(0,-0,-1),
    };

    static size_t const nIndicesPerStrip = 4;
    static unsigned short const s0[] = {1,2,0,3};
    static unsigned short const s1[] = {7,4,6,5};
    static unsigned short const s2[] = {23,20,22,21};
    static unsigned short const s3[] = {19,16,18,17};
    static unsigned short const s4[] = {13,14,12,15};
    static unsigned short const s5[] = {9,10,8,11};

    osg::Matrixf const s = osg::Matrixf::scale(scale.x(), scale.y(), scale.z());
    osg::Matrixf const t = osg::Matrixf::translate(translation);
    osg::Matrixf const x = s*t;

    osg::Vec3Array* v = new osg::Vec3Array(nVertices, vertices);
    for (size_t i=0; i<nVertices; ++i) (*v)[i] = (*v)[i]*x;

    osg::Geometry* d = new osg::Geometry;
    d->setVertexArray(v);
    d->setNormalArray(new osg::Vec3Array(nNormals, normals), osg::Array::BIND_PER_VERTEX);
    d->setColorArray(new osg::Vec4Array(1, &color), osg::Array::BIND_OVERALL);

    d->addPrimitiveSet(
        new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP, nIndicesPerStrip, s0));
    d->addPrimitiveSet(
        new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP, nIndicesPerStrip, s1));
    d->addPrimitiveSet(
        new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP, nIndicesPerStrip, s2));
    d->addPrimitiveSet(
        new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP, nIndicesPerStrip, s3));
    d->addPrimitiveSet(
        new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP, nIndicesPerStrip, s4));
    d->addPrimitiveSet(
        new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP, nIndicesPerStrip, s5));

    return d;
}

} // namespace hyd

