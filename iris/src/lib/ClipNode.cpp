#include <dtk.h>
#include <osg/ClipPlane>
#include <iris/ClipNode.h>
#include <iris/SceneGraph.h>
#include <iris/Pane.h>
#include <iris/Utils.h>

namespace iris
{	  
  ClipNode::ClipNode() : osg::ClipNode(), _num(-1), _enabled(false), _valid(false)
  {
    setDataVariance(osg::Object::DYNAMIC) ;
    for (int i=0; i<IRIS_CLIP_NODE_MAX_NUMBER; i++)
      {
	if (!_clipNodes[i])
	  {
	    _num = i ;
	    break ;
	  }
      }
    if (_num<0)
      {
	dtkMsg.add(DTKMSG_ERROR, "iris::ClipNode: ERROR- no clip plane available\n") ;
	return ;
      }
    osg::ref_ptr<osg::ClipPlane> cp = new osg::ClipPlane(_num , 0.f, 1.f, 0.f, 0.f) ;
    cp->setDataVariance(osg::Object::DYNAMIC) ;
    addClipPlane(cp) ;
    setCullingActive(false) ;
    _clipNodes[_num] = this ;

    // create and initialize iris_ClipPlaneEnabled uniform
    if (!_clipEnableUniform) 
      {
	_clipEnableUniform = SceneGraph::instance()->getSceneNode()->getOrCreateStateSet()->getOrCreateUniform("iris_ClipPlaneEnabled", osg::Uniform::BOOL, IRIS_CLIP_NODE_MAX_NUMBER) ;
	for (unsigned int k=0; k<IRIS_CLIP_NODE_MAX_NUMBER; k++)
	  {
	    _clipEnableUniform->setElement(k, _enabled) ;
	    dtkMsg.add(DTKMSG_INFO, "iris::ClipNode: clip plane %d = %d\n",k, _enabled) ; 
	  }
      }

    _clipEnableUniform->setElement(_num, _enabled) ;

    _valid = true ;
  }

  ////////////////////////////////////////////////////////////////////////
  ClipNode::~ClipNode()
  {
    disable() ;
    _clipNodes[_num] = NULL ;
  }

  ////////////////////////////////////////////////////////////////////////
  void ClipNode::enable() 
  {
    dtkMsg.add(DTKMSG_INFO, "iris::ClipNode: turning on clipping plane %d\n",_num) ;
    SceneGraph::instance()->getSceneNode()->getOrCreateStateSet()->setMode(GL_CLIP_PLANE0+_num, osg::StateAttribute::ON);
    getClipPlane(0)->setClipPlane(0.f, 1.f, 0.f, 0.f) ;
    _enabled = true ;
    _clipEnableUniform->setElement(_num, _enabled) ;
  }
  
  ////////////////////////////////////////////////////////////////////////
  void ClipNode::disable() 
  {
    dtkMsg.add(DTKMSG_INFO, "iris::ClipNode: turning off clipping plane %d\n",_num) ;
    SceneGraph::instance()->getSceneNode()->getOrCreateStateSet()->setMode(GL_CLIP_PLANE0+_num, osg::StateAttribute::OFF);
    // this can signal shaer programs that the clipping plane has been disabled
    getClipPlane(0)->setClipPlane(0.f, 0.f, 0.f, 0.f) ;
    _enabled = false ;
    _clipEnableUniform->setElement(_num, _enabled) ;
  }

  ////////////////////////////////////////////////////////////////////////
  std::vector<ClipNode*> ClipNode::getClipNodes()
  {
    std::vector<ClipNode*> nodes ;
    for (int i=0; i<IRIS_CLIP_NODE_MAX_NUMBER; i++)
      {
	if (_clipNodes[i] != NULL) nodes.push_back(_clipNodes[i]) ;
      }
    return nodes ;
  }
  ////////////////////////////////////////////////////////////////////////
  ClipNode* ClipNode::_clipNodes[IRIS_CLIP_NODE_MAX_NUMBER] =  {NULL, NULL, NULL, NULL, NULL, NULL} ;
  osg::Uniform* ClipNode::_clipEnableUniform = NULL ;
}
