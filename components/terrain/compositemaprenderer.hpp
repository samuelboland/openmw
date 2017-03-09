#ifndef OPENMW_COMPONENTS_TERRAIN_COMPOSITEMAPRENDERER_H
#define OPENMW_COMPONENTS_TERRAIN_COMPOSITEMAPRENDERER_H

#include <osg/Drawable>

#include <OpenThreads/Mutex>

#include <deque>

namespace osg
{
    class FrameBufferObject;
    class RenderInfo;
    class Texture2D;
}

namespace Terrain
{

    class CompositeMap : public osg::Referenced
    {
    public:
        ~CompositeMap();
        std::vector<osg::ref_ptr<osg::Drawable> > mDrawables;
        osg::ref_ptr<osg::Texture2D> mTexture;
    };

    /**
     * @brief The CompositeMapRenderer is responsible for updating composite map textures in a blocking or non-blocking way.
     */
    class CompositeMapRenderer : public osg::Drawable
    {
    public:
        CompositeMapRenderer();

        virtual void drawImplementation(osg::RenderInfo& renderInfo) const;

        void compile(CompositeMap& compositeMap, osg::RenderInfo& renderInfo, double* timeLeft) const;

        /// Set the available time in seconds for compiling (non-immediate) composite maps each frame
        void setTimeAvailableForCompile(double time);

        /// Add a composite map to be rendered
        void addCompositeMap(CompositeMap* map, bool immediate=false);

        /// Mark this composite map to be required for the current frame
        void setImmediate(CompositeMap* map);

    private:
        double mTimeAvailable;

        typedef std::set<osg::ref_ptr<CompositeMap> > CompileSet;

        mutable CompileSet mCompileSet;
        mutable CompileSet mImmediateCompileSet;

        mutable CompileSet mCompiled;

        mutable OpenThreads::Mutex mMutex;

        osg::ref_ptr<osg::FrameBufferObject> mFBO;
    };

}

#endif