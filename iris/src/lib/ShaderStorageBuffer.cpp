#include <osg/BufferObject>
#include <osg/BufferIndexBinding>
#include <iris/ShaderStorageBuffer.h>
#include <iris/SceneGraph.h>

#ifndef GL_ARB_shader_storage_buffer_object
#define GL_SHADER_STORAGE_BUFFER          0x90D2
#define GL_SHADER_STORAGE_BUFFER_BINDING  0x90D3
#define GL_SHADER_STORAGE_BUFFER_START    0x90D4
#define GL_SHADER_STORAGE_BUFFER_SIZE     0x90D5
#define GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS 0x90D6
#define GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS 0x90D7
#define GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS 0x90D8
#define GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS 0x90D9
#define GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS 0x90DA
#define GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS 0x90DB
#define GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS 0x90DC
#define GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS 0x90DD
#define GL_MAX_SHADER_STORAGE_BLOCK_SIZE  0x90DE
#define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT 0x90DF
#define GL_SHADER_STORAGE_BARRIER_BIT     0x2000
#define GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS
/* reuse GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS */
#endif

namespace iris
{

    ShaderStorageBufferObject::ShaderStorageBufferObject()
    {
        setTarget(GL_SHADER_STORAGE_BUFFER);
        setUsage(GL_DYNAMIC_DRAW);
    }

    ShaderStorageBufferObject::ShaderStorageBufferObject(
        const ShaderStorageBufferObject& rhs, const osg::CopyOp& copyop)
        : BufferObject(rhs, copyop)
    {
    }

    ShaderStorageBufferObject::~ShaderStorageBufferObject()
    {
    }

    ShaderStorageBufferBinding::ShaderStorageBufferBinding(GLuint index)
        : BufferIndexBinding(GL_SHADER_STORAGE_BUFFER, index)
    {
    }

    ShaderStorageBufferBinding::ShaderStorageBufferBinding(
        GLuint index, osg::BufferObject* bo, GLintptr offset, GLsizeiptr size)
        : BufferIndexBinding(GL_SHADER_STORAGE_BUFFER, index, bo, offset, size)
    {
    }

    ShaderStorageBufferBinding::ShaderStorageBufferBinding(
        const ShaderStorageBufferBinding& rhs, const osg::CopyOp& copyop)
        : BufferIndexBinding(rhs, copyop)
    {
    }

}
