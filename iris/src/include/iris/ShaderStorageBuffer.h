#ifndef __IRIS_SHADER_STORAGE_BUFFER
#define __IRIS_SHADER_STORAGE_BUFFER
#include <osg/BufferObject>
#include <osg/BufferIndexBinding>

namespace iris
{	  
    /**

      \brief The %ShaderStorageBufferObject class is subclassed from the
      osg::BufferObject class.
      \n \n The %ShaderStorageBufferObject binds to GL_SHADER_STORAGE_BUFFER.
      \n The GL_SHADER_STORAGE_BUFFER binding is a new buffer type defined in
      OpenGL 4 that allows shaders to perform random access reads, writes, and
      atomic memory operations on variables stored in a buffer object.
      \n The buffers are declared in shaders, but must be allocated by the
      application. The %ShaderStorageBufferObject class handles the
      application-side allocation. See the ARB_shader_storage_buffer_object
      extension:
      http://www.opengl.org/registry/specs/ARB/shader_storage_buffer_object.txt
      and the \b SHADERSTORAGEBUFFER IRIS control command for more information.

    */

    class ShaderStorageBufferObject : public osg::BufferObject
    {
    public:
        ShaderStorageBufferObject() ;
        ShaderStorageBufferObject(const ShaderStorageBufferObject& rhs,
                                  const osg::CopyOp& copyop
                                    = osg::CopyOp::SHALLOW_COPY);
        META_Object(iris, ShaderStorageBufferObject);
    protected:
        virtual ~ShaderStorageBufferObject();
    };


    /**

      \brief The %ShaderStorageBufferBinding class is subclassed from the
      osg::BufferIndexBinding class.
      \n \n The %ShaderStorageBufferBinding class is the binding index
      state for %ShaderStorageBufferObject.

    */

    class ShaderStorageBufferBinding : public osg::BufferIndexBinding
    {
    public:
        ShaderStorageBufferBinding(GLuint index = 0);
        ShaderStorageBufferBinding(GLuint index, osg::BufferObject* bo,
                                   GLintptr offset, GLsizeiptr size);
        ShaderStorageBufferBinding(const ShaderStorageBufferBinding& rhs,
                                   const osg::CopyOp& copyop
                                     = osg::CopyOp::SHALLOW_COPY);
        META_StateAttribute(iris, ShaderStorageBufferBinding,
                            (osg::StateAttribute::Type)8234); // FIXME 8234 is a random number

        virtual int compare(const osg::StateAttribute& bb) const
        {
            COMPARE_StateAttribute_Types(ShaderStorageBufferBinding, bb)

            COMPARE_StateAttribute_Parameter(_target)
            COMPARE_StateAttribute_Parameter(_index)
            COMPARE_StateAttribute_Parameter(_bufferObject)
            COMPARE_StateAttribute_Parameter(_offset)
            COMPARE_StateAttribute_Parameter(_size)
            return 0;
        }
    };

}
#endif
