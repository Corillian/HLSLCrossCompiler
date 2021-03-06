#include "Shader.h"
#include <GL/glew.h>
#include "common/debug.h"

const uint_t InvalidShaderHandle = 0xFFFFFFFF;

ShaderEffect::ShaderEffect() : mCompileFlags(0), mRequestedLang(LANG_DEFAULT),
    mVertex(InvalidShaderHandle),
    mPixel(InvalidShaderHandle),
    mGeometry(InvalidShaderHandle),
    mHull(InvalidShaderHandle),
    mDomain(InvalidShaderHandle)
{
}

void ShaderEffect::Create()
{
    mProgram = glCreateProgram();

    if(HaveLimitedInOutLocationQualifier(mVSLang) == 0 || HaveLimitedInOutLocationQualifier(mPSLang) == 0)
    {
        int maxAttrib = 0;

        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttrib);

        for(int i=0; i<maxAttrib; ++i)
        {
            std::string attribName("dcl_Input");
            //Visual Studio/Windows missing int version std::to_string, so use long long.
            attribName += std::to_string((long long)i);

            glBindAttribLocation(mProgram, i, attribName.c_str());
        }

        int maxDrawBuffers = 0;

        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);

        for(int i=0; i<maxAttrib; ++i)
        {
            std::string pixelOutputName("PixOutput");
            pixelOutputName += std::to_string((long long)i);

            glBindFragDataLocation(mProgram, i, pixelOutputName.c_str());
        }
    }
}

void ShaderEffect::FromGLSLFile(uint_t eShaderType, std::string& path)
{

    char* shaderSrc;

    {
        FILE* shaderFile;
        int length;
        int readLength;
	    int success = 0;

        shaderFile = fopen(path.c_str(), "rb");

        ASSERT(shaderFile);

        fseek(shaderFile, 0, SEEK_END);
        length = ftell(shaderFile);
        fseek(shaderFile, 0, SEEK_SET);

        shaderSrc = (char*)malloc(length+1);

        readLength = fread(shaderSrc, 1, length, shaderFile);

        fclose(shaderFile);
        shaderFile = 0;

        shaderSrc[readLength] = '\0';
    }

    uint_t shader = 0;

    switch(eShaderType)
    {
        case GL_VERTEX_SHADER:
        {
            mVertex = glCreateShader(GL_VERTEX_SHADER);
            shader = mVertex;
            mVSLang = LANG_DEFAULT;
            break;
        }
        case GL_FRAGMENT_SHADER:
        {
            mPixel = glCreateShader(GL_FRAGMENT_SHADER);
            shader = mPixel;
            mPSLang = LANG_DEFAULT;
            break;
        }
        case GL_GEOMETRY_SHADER:
        {
            mGeometry = glCreateShader(GL_GEOMETRY_SHADER);
            shader = mGeometry;
            mGSLang = LANG_DEFAULT;
            break;
        }
        case GL_TESS_CONTROL_SHADER:
        {
            mHull = glCreateShader(GL_TESS_CONTROL_SHADER);
            shader = mHull;
            mHSLang = LANG_DEFAULT;
            break;
        }
        case GL_TESS_EVALUATION_SHADER:
        {
            mDomain = glCreateShader(GL_TESS_EVALUATION_SHADER);
            shader = mDomain;
            mDSLang = LANG_DEFAULT;
            break;
        }
        default:
        {
            break;
        }
    }

    glShaderSource(shader, 1, (const char **)&shaderSrc, 0);
    glCompileShader(shader);
    glAttachShader(mProgram, shader);

#ifdef _DEBUG
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if(!compiled)
    {
        char* log;
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        log = new char[length];
        glGetShaderInfoLog(shader, length, NULL, log);
        ASSERT(0);
        delete [] log;

        ASSERT(0);
    }
#endif

    delete [] shaderSrc;
}

void ShaderEffect::FromByteFile(std::string& path)
{
    GLSLShader result;

    int translated = TranslateHLSLFromFile(path.c_str(), mCompileFlags, mRequestedLang, &mDependencies, &result);

    ASSERT(translated);

    uint_t shader = 0;

    if(PixelInpterpDependency(result.GLSLLanguage))
    {
        //Must compile pixel shader first!
        if(mPixel == InvalidShaderHandle &&  result.shaderType != GL_FRAGMENT_SHADER)
        {
            ASSERT(0);
        }
    }

    switch(result.shaderType)
    {
        case GL_VERTEX_SHADER:
        {
            mVertex = glCreateShader(GL_VERTEX_SHADER);
            shader = mVertex;
            mVSLang = result.GLSLLanguage;
            break;
        }
        case GL_FRAGMENT_SHADER:
        {
            mPixel = glCreateShader(GL_FRAGMENT_SHADER);
            shader = mPixel;
            mPSLang = result.GLSLLanguage;
            break;
        }
        case GL_GEOMETRY_SHADER:
        {
            mGeometry = glCreateShader(GL_GEOMETRY_SHADER);
            shader = mGeometry;
            mGSLang = result.GLSLLanguage;
            break;
        }
        case GL_TESS_CONTROL_SHADER:
        {
            mHull = glCreateShader(GL_TESS_CONTROL_SHADER);
            shader = mHull;
            mHSLang = result.GLSLLanguage;
            break;
        }
        case GL_TESS_EVALUATION_SHADER:
        {
            mDomain = glCreateShader(GL_TESS_EVALUATION_SHADER);
            shader = mDomain;
            mDSLang = result.GLSLLanguage;

            //Hull shader must be compiled before domain in order
            //to ensure correct partitioning and primitive type information
            //is OR'ed into mCompileFlags.
            ASSERT(mHull != InvalidShaderHandle);

            break;
        }
        default:
        {
            break;
        }
    }

    glShaderSource(shader, 1, (const char **)&result.sourceCode, 0);
    glCompileShader(shader);
    glAttachShader(mProgram, shader);

#ifdef _DEBUG
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if(!compiled)
    {
        char* log;
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        log = new char[length];
        glGetShaderInfoLog(shader, length, NULL, log);
        ASSERT(0);
        delete [] log;

        ASSERT(0);
    }
#endif


#if 0
    //Set the default values for constants.
    for(uint32_t i = 0; i < result.reflection.ui32NumConstantBuffers; ++i)
    {
        ConstantBuffer* cbuf = result.reflection.psConstantBuffers+i;

        for(uint32_t k = 0; k < cbuf->ui32NumVars; ++k)
        {
            uint32_t loc = glGetUniformLocation(mProgram, cbuf->asVars[k].Name);
            glUniform1f(loc, *(float*)&cbuf->asVars[k].ui32DefaultValue);
        }
    }
#endif

    FreeGLSLShader(&result);
}


void ShaderEffect::Enable()
{
    glLinkProgram(mProgram);

#ifdef _DEBUG
    GLint linked = GL_FALSE;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
    if(!linked)
    {
        char* log;
        GLint length = 0;
        glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &length);
        log = new char[length];
        glGetProgramInfoLog(mProgram, length, NULL, log);
        ASSERT(0);
        delete [] log;
    }
#endif

    glUseProgram(mProgram);
}

void ShaderEffect::SetTexture(std::string& name, int imageUnit) {
    int loc = glGetUniformLocation(mProgram, (name).c_str());
    glUniform1i(loc, imageUnit);
}

void ShaderEffect::SetVec4(std::string& name, int count, float* v) {
    int loc = glGetUniformLocation(mProgram, (name + std::string("VS")).c_str());
    glUniform4fv(loc, count, v);

    if(mDomain)
    {
        loc = glGetUniformLocation(mProgram, (name + std::string("HS")).c_str());
        glUniform4fv(loc, count, v);

        loc = glGetUniformLocation(mProgram, (name + std::string("DS")).c_str());
        glUniform4fv(loc, count, v);
    }

    if(mGeometry)
    {
        loc = glGetUniformLocation(mProgram, (name + std::string("GS")).c_str());
        glUniform4fv(loc, count, v);
    }

    loc = glGetUniformLocation(mProgram, (name + std::string("PS")).c_str());
    glUniform4fv(loc, count, v);
}

void ShaderEffect::CreateUniformBlock(std::string& name, uint_t& ubo)
{
	GLuint bufferID;

    glGenBuffers(1, &bufferID);

    uint_t uniformBlockIndex = glGetUniformBlockIndex(mProgram, name.c_str());
        
	GLint uniformBlockSize;
    //We need to get the uniform block's size in order to back it with the
    //appropriate buffer
    glGetActiveUniformBlockiv(mProgram, uniformBlockIndex,
                                    GL_UNIFORM_BLOCK_DATA_SIZE,
                                    &uniformBlockSize);

    //Create UBO.
    glBindBuffer(GL_UNIFORM_BUFFER, bufferID);
    glBufferData(GL_UNIFORM_BUFFER, uniformBlockSize,
                    NULL, GL_DYNAMIC_DRAW);

	ubo = bufferID;
}

void ShaderEffect::SetUniformBlock(std::string& name, uint_t bufIndex)
{
    int uniformIndex = glGetUniformBlockIndex(mProgram, (name).c_str());
    glUniformBlockBinding(mProgram, uniformIndex, bufIndex);
}

void ShaderEffect::SetUniformBlock(std::string& name, uint_t bufIndex, uint_t ubo)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, bufIndex, ubo);
	SetUniformBlock(name, bufIndex);
}
