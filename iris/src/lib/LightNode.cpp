#include <dtk.h>
#include <osg/Light>
#include <iris/LightNode.h>
#include <iris/SceneGraph.h>
#include <iris/Utils.h>

namespace iris
{	  
  LightNode::LightNode() : osg::LightSource(), _num(-1), _enabled(false), _valid(false)
  {

    // data variance is DYNAMIC because object might change in callback
    setDataVariance(osg::Object::DYNAMIC) ;

    // defaults
    const osg::Vec4 pos = osg::Vec4(0.0f, -1.f, 0.f, 0.f) ;
    const osg::Vec3 dir = osg::Vec3(0.0f, 1.0f, 0.f) ;
    const float ambient = 0.f ;
    const float diffuse = 0.8f ;
    const float specular = 1.0f ;

    for (int i=0; i<IRIS_LIGHT_NODE_MAX_NUMBER; i++)
      {
	if (!_lightNodes[i])
	  {
	    _num = i ;
	    break ;
	  }
      }
    if (_num<0)
      {
	dtkMsg.add(DTKMSG_ERROR, "iris::LightNode: ERROR- no light available\n") ;
	return ;
      }
    osg::ref_ptr<osg::Light> light = new osg::Light ;
    // data variance is DYNAMIC because object might change in callback
    light->setDataVariance(osg::Object::DYNAMIC) ;

    light->setLightNum(_num);
    light->setPosition(pos);
    light->setDirection(dir) ;
    light->setAmbient(osg::Vec4(ambient, ambient, ambient, 1.f));
    light->setDiffuse(osg::Vec4(diffuse, diffuse, diffuse, 1.f));
    light->setSpecular(osg::Vec4(specular, specular, specular, 1.f));
    setCullingActive(false) ;
    setLight(light.get()) ;

    _lightNodes[_num] = this ;

    // create and initialize iris_LightEnabled uniform
    if (!_lightEnableUniform) 
      {
	_lightEnableUniform = SceneGraph::instance()->getSceneNode()->getOrCreateStateSet()->getOrCreateUniform("iris_LightEnabled", osg::Uniform::BOOL, IRIS_LIGHT_NODE_MAX_NUMBER) ;
	for (unsigned int k=0; k<IRIS_LIGHT_NODE_MAX_NUMBER; k++)
	  {
	    _lightEnableUniform->setElement(k, _enabled) ;
	    dtkMsg.add(DTKMSG_INFO, "iris::LightNode: light plane %d = %d\n",k, _enabled) ; 
	  }
      }

    _lightEnableUniform->setElement(_num, _enabled) ;

    _valid = true ;
  }

  ////////////////////////////////////////////////////////////////////////
  LightNode::~LightNode()
  {
    disable() ;
    _lightNodes[_num] = NULL ;
  }

  /* 
     http://www.mail-archive.com/osg-users@lists.openscenegraph.org/msg07336.html

     The only thing I've used setLocalStateSetModes for is to turn the light
     on/off. I've never used setStateSetModes, but it looks like it's just a
     convenience function for adding the LightSource's light to the
     rootStateSet. Essentially it's including the light for rendering.
       
     If you want to apply a light to selective branches of the scene you would
     add the light to the branches stateset, something like this.
       
     osg::StateSet * ss = branch->getOrCreateStateSet();
     if (onOff)
     {
     ss->setMode(GL_LIGHT0+_light->getLightNum(), osg::StateAttribute::ON);
     }
     else
     {
     ss->setMode(GL_LIGHT0+_light->getLightNum(), osg::StateAttribute::OFF);
     }
    
     To turn the light on/off you would turn the lightsource on/off.
    
     lightsource->setLocalStateSetModes(ON/OFF)
       
  */

  ////////////////////////////////////////////////////////////////////////
  void LightNode::enable() 
  {
    dtkMsg.add(DTKMSG_INFO, "iris::LightNode: turning on light %d\n",_num) ;

    //setLocalStateSetModes(osg::StateAttribute::ON);

    osg::ref_ptr<osg::StateSet> sceneStateSet = SceneGraph::instance()->getSceneNode()->getOrCreateStateSet() ;
    setStateSetModes(*sceneStateSet,osg::StateAttribute::ON);

    _enabled = true ;

    _lightEnableUniform->setElement(_num, _enabled) ;
  }

  ////////////////////////////////////////////////////////////////////////
  void LightNode::disable() 
  {
    dtkMsg.add(DTKMSG_INFO, "iris::LightNode: turning off light %d\n",_num) ;

    //setLocalStateSetModes(osg::StateAttribute::OFF) ;

    osg::ref_ptr<osg::StateSet> sceneStateSet = SceneGraph::instance()->getSceneNode()->getOrCreateStateSet() ;
    setStateSetModes(*sceneStateSet,osg::StateAttribute::OFF);

    //osg::ref_ptr<osg::StateSet> sceneStateSet = SceneGraph::instance()->getSceneNode()->getOrCreateStateSet() ;
    //sceneStateSet->setMode(GL_LIGHT0+_num, osg::StateAttribute::OFF) ;

    _enabled = false ;

    _lightEnableUniform->setElement(_num, _enabled) ;
  }

  ////////////////////////////////////////////////////////////////////////
  std::vector<LightNode*> LightNode::getLightNodes()
  {
    std::vector<LightNode*> nodes ;
    for (int i=0; i<IRIS_LIGHT_NODE_MAX_NUMBER; i++)
      {
	if (_lightNodes[i] != NULL) nodes.push_back(_lightNodes[i]) ;
      }
    return nodes ;
  }

  ////////////////////////////////////////////////////////////////////////
  LightNode* LightNode::_lightNodes[IRIS_LIGHT_NODE_MAX_NUMBER] =  {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL} ;
  osg::Uniform* LightNode::_lightEnableUniform = NULL ;

}
