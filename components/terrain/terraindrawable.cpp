#include "terraindrawable.hpp"

#include <osgUtil/CullVisitor>
#include <osg/OccluderNode>
#include <osg/Material>

#include <components/sceneutil/lightmanager.hpp>

#include "compositemaprenderer.hpp"

namespace Terrain
{

TerrainDrawable::TerrainDrawable(const TerrainDrawable &copy, const osg::CopyOp &copyop)
    : osg::Geometry(copy, copyop)
    , mPasses(copy.mPasses)
    , mLightListCallback(copy.mLightListCallback)
{

}

void TerrainDrawable::accept(osg::NodeVisitor &nv)
{
    if (nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR)
    {
        osg::Geometry::accept(nv);
    }
    else if (nv.validNodeMask(*this))
    {
        nv.pushOntoNodePath(this);
        cull(static_cast<osgUtil::CullVisitor*>(&nv));
        nv.popFromNodePath();
    }
}

inline float distance(const osg::Vec3& coord,const osg::Matrix& matrix)
{
    return -((float)coord[0]*(float)matrix(0,2)+(float)coord[1]*(float)matrix(1,2)+(float)coord[2]*(float)matrix(2,2)+matrix(3,2));
}

void TerrainDrawable::cull(osgUtil::CullVisitor *cv)
{
    const osg::BoundingBox& bb = getBoundingBox();

    if (_cullingActive && cv->isCulled(getBoundingBox()))
        return;

    osg::RefMatrix& matrix = *cv->getModelViewMatrix();

    if (cv->getComputeNearFarMode() && bb.valid())
    {
        if (!cv->updateCalculatedNearFar(matrix, *this, false))
            return;
    }

    float depth = bb.valid() ? distance(bb.center(),matrix) : 0.0f;
    if (osg::isNaN(depth))
        return;

    if (cv->getCurrentCamera()->getName() == "ShadowCamera")
    {
        cv->addDrawableAndDepth(this, &matrix, depth);
        return;
    }

    if (mCompositeMap)
    {
        mCompositeMapRenderer->setImmediate(mCompositeMap);
        mCompositeMap = nullptr;
    }

    bool pushedLight = mLightListCallback && mLightListCallback->pushLightState(this, cv);

    for (PassVector::const_iterator it = mPasses.begin(); it != mPasses.end(); ++it)
    {
        cv->pushStateSet(*it);
        cv->addDrawableAndDepth(this, &matrix, depth);
        cv->popStateSet();
    }

    if (pushedLight)
        cv->popStateSet();
}

void TerrainDrawable::setPasses(const TerrainDrawable::PassVector &passes)
{
    mPasses = passes;
}

void TerrainDrawable::setLightListCallback(SceneUtil::LightListCallback *lightListCallback)
{
    mLightListCallback = lightListCallback;
}

void TerrainDrawable::compileGLObjects(osg::RenderInfo &renderInfo) const
{
    for (PassVector::const_iterator it = mPasses.begin(); it != mPasses.end(); ++it)
    {
        osg::StateSet* stateset = *it;
        stateset->compileGLObjects(*renderInfo.getState());
    }

    osg::Geometry::compileGLObjects(renderInfo);
}

osg::OccluderNode* createOccluder(const osg::Vec3& v1,const osg::Vec3& v2,const osg::Vec3& v3,const osg::Vec3& v4)
{
    osg::ConvexPlanarOccluder* cpo = new osg::ConvexPlanarOccluder;
    osg::ConvexPlanarPolygon& occluder = cpo->getOccluder();
    occluder.add(v1);
    occluder.add(v2);
    occluder.add(v3);
    occluder.add(v4);
    osg::OccluderNode* occluderNode = new osg::OccluderNode;
    occluderNode->setOccluder(cpo);

    osg::Geometry* debug = new osg::Geometry;
    osg::Vec3Array* coords = new osg::Vec3Array(occluder.getVertexList().begin(),occluder.getVertexList().end());
    debug->setVertexArray(coords);
    debug->setCullingActive(false);
    debug->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
    static osg::ref_ptr<osg::StateSet> stateset = nullptr;
    if (!stateset){ stateset = new osg::StateSet;
    stateset->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
    osg::Material* m = new osg::Material;
    m->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4f(1,0,0,1));
      m->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4f(0,0,0,1));
        m->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4f(0,0,0,1));
        stateset->setAttributeAndModes(m, osg::StateAttribute::ON);
        stateset->setRenderBinDetails(100,"RenderBin");
    }
    debug->setStateSet(stateset);

    occluderNode->addChild(debug);

    return occluderNode;
}

void TerrainDrawable::createOccluders(const osg::Vec2f& offset)
{
    mOccluders = new osg::Group;

    const osg::BoundingBox& mybb = getBoundingBox();
    if (mybb._min.z() <= -1) return;

    osg::BoundingBox bb;
    bb._min = osg::Vec3f(mybb._min.x()+offset.x(), mybb._min.y()+offset.y(), -1);
    bb._max = osg::Vec3f(mybb._max.x()+offset.x(), mybb._max.y()+offset.y(), mybb._min.z());

    // bottom side
    mOccluders->addChild(createOccluder(bb.corner(0),
                                    bb.corner(2),
                                    bb.corner(3),
                                    bb.corner(1)));

    const float sandwichfactor = 0.1;
    if ((bb._max.z() - bb._min.z()) / (bb._max.x() - bb._min.x()) < sandwichfactor)
        return;

    // front side
    mOccluders->addChild(createOccluder(bb.corner(0),
                                  bb.corner(1),
                                  bb.corner(5),
                                  bb.corner(4)));
    // right side
    mOccluders->addChild(createOccluder(bb.corner(1),
                                   bb.corner(3),
                                  bb.corner(7),
                                  bb.corner(5)));
   // left side
   mOccluders->addChild(createOccluder(bb.corner(2),
                                  bb.corner(0),
                                  bb.corner(4),
                                   bb.corner(6)));
   // back side
   mOccluders->addChild(createOccluder(bb.corner(3),
                                  bb.corner(2),
                                   bb.corner(6),
                                  bb.corner(7)));

    // top side
    mOccluders->addChild(createOccluder(bb.corner(6),
                                    bb.corner(4),
                                    bb.corner(5),
                                    bb.corner(7)));

}

osg::Group* TerrainDrawable::getOccluders()
{
    return mOccluders.get();
}

TerrainDrawable::~TerrainDrawable() {}
TerrainDrawable::TerrainDrawable(){}

}

