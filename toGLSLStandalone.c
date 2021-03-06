
#include "toGLSL.h"
#include "languages.h"
#include "stdlib.h"
#include "stdio.h"
#include "bstrlib.h"

#include "timer.h"

#define VALIDATE_OUTPUT

#if defined(VALIDATE_OUTPUT) && defined(_WIN32)
#if defined(_WIN32)
#include <windows.h>
#include <gl/GL.h>

 #pragma comment(lib, "opengl32.lib")

	typedef char GLcharARB;		/* native character */
	typedef unsigned int GLhandleARB;	/* shader object handle */
#define GL_OBJECT_COMPILE_STATUS_ARB      0x8B81
#define GL_OBJECT_LINK_STATUS_ARB         0x8B82
#define GL_OBJECT_INFO_LOG_LENGTH_ARB        0x8B84
	typedef void (WINAPI * PFNGLDELETEOBJECTARBPROC) (GLhandleARB obj);
	typedef GLhandleARB (WINAPI * PFNGLCREATESHADEROBJECTARBPROC) (GLenum shaderType);
	typedef void (WINAPI * PFNGLSHADERSOURCEARBPROC) (GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
	typedef void (WINAPI * PFNGLCOMPILESHADERARBPROC) (GLhandleARB shaderObj);
	typedef void (WINAPI * PFNGLGETINFOLOGARBPROC) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
	typedef void (WINAPI * PFNGLGETOBJECTPARAMETERIVARBPROC) (GLhandleARB obj, GLenum pname, GLint *params);
	typedef GLhandleARB (WINAPI * PFNGLCREATEPROGRAMOBJECTARBPROC) (void);
	typedef void (WINAPI * PFNGLATTACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB obj);
	typedef void (WINAPI * PFNGLLINKPROGRAMARBPROC) (GLhandleARB programObj);
	typedef void (WINAPI * PFNGLUSEPROGRAMOBJECTARBPROC) (GLhandleARB programObj);
    typedef void (WINAPI * PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei* length, GLcharARB* infoLog);

	static PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
	static PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
	static PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
	static PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
	static PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
	static PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
	static PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
	static PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
	static PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
	static PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;
    static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;

#define WGL_CONTEXT_DEBUG_BIT_ARB 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB 0x2093
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define ERROR_INVALID_VERSION_ARB 0x2095
#define ERROR_INVALID_PROFILE_ARB 0x2096

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126

typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int* attribList);
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

void InitOpenGL()
{
    HGLRC rc;

	// setup minimal required GL
	HWND wnd = CreateWindowA(
							 "STATIC",
							 "GL",
							 WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS |	WS_CLIPCHILDREN,
							 0, 0, 16, 16,
							 NULL, NULL,
							 GetModuleHandle(NULL), NULL );
	HDC dc = GetDC( wnd );
	
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
		PFD_TYPE_RGBA, 32,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		16, 0,
		0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};
	
	int fmt = ChoosePixelFormat( dc, &pfd );
	SetPixelFormat( dc, fmt, &pfd );
	
	rc = wglCreateContext( dc );
	wglMakeCurrent( dc, rc );

    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    if(wglCreateContextAttribsARB)
    {
        const int OpenGLContextAttribs [] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
    #if defined(_DEBUG)
            //WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB,
    #else
            //WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
    #endif
            //WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0, 0
        };

        const HGLRC OpenGLContext = wglCreateContextAttribsARB( dc, 0, OpenGLContextAttribs );

        wglMakeCurrent(dc, OpenGLContext);

        wglDeleteContext(rc);

        rc = OpenGLContext;
    }

    glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");
    glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
    glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
    glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
    glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");
    glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
    glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
    glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress("glAttachObjectARB");
    glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress("glLinkProgramARB");
    glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress("glUseProgramObjectARB");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)("glGetShaderInfoLog");
}
#endif

int TryCompileShader(GLenum eGLSLShaderType, char* inFilename, char* shader, double* pCompileTime)
{
    GLint iCompileStatus;
    GLuint hShader;
    Timer_t timer;

    InitTimer(&timer);

    InitOpenGL();

    hShader = glCreateShaderObjectARB(eGLSLShaderType);
    glShaderSourceARB(hShader, 1, (const char **)&shader, NULL);

    ResetTimer(&timer);
    glCompileShaderARB(hShader);
    *pCompileTime = ReadTimer(&timer);

    /* Check it compiled OK */
    glGetObjectParameterivARB (hShader, GL_OBJECT_COMPILE_STATUS_ARB, &iCompileStatus);

    if (iCompileStatus != GL_TRUE)
    {
        FILE* errorFile;
        GLint iInfoLogLength = 0;
        char* pszInfoLog;
		bstring filename = bfromcstr(inFilename);
		char* cstrFilename;

        glGetObjectParameterivARB (hShader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &iInfoLogLength);

        pszInfoLog = malloc(iInfoLogLength);

        printf("Error: Failed to compile GLSL shader\n");

		glGetInfoLogARB (hShader, iInfoLogLength, NULL, pszInfoLog);

        printf(pszInfoLog);

		bcatcstr(filename, "_compileErrors.txt");

		cstrFilename = bstr2cstr(filename, '\0');

        //Dump to file
        errorFile = fopen(cstrFilename, "w");
        fprintf(errorFile, pszInfoLog);
        fclose(errorFile);

		bdestroy(filename);
		free(cstrFilename);
        free(pszInfoLog);

        return 0;
    }

    return 1;
}
#endif

int fileExists(const char* path)
{
    FILE* shaderFile;
    shaderFile = fopen(path, "rb");

    if(shaderFile)
    {
        fclose(shaderFile);
        return 1;
    }
    return 0;
}

GLLang LanguageFromString(const char* str)
{
    if(strcmp(str, "es100")==0)
    {
        return LANG_ES_100;
    }
    if(strcmp(str, "es300")==0)
    {
        return LANG_ES_300;
    }
    if(strcmp(str, "120")==0)
    {
        return LANG_120;
    }
    if(strcmp(str, "130")==0)
    {
        return LANG_130;
    }
    if(strcmp(str, "140")==0)
    {
        return LANG_140;
    }
    if(strcmp(str, "150")==0)
    {
        return LANG_150;
    }
    if(strcmp(str, "330")==0)
    {
        return LANG_330;
    }
    if(strcmp(str, "400")==0)
    {
        return LANG_400;
    }
    if(strcmp(str, "410")==0)
    {
        return LANG_410;
    }
    if(strcmp(str, "420")==0)
    {
        return LANG_420;
    }
    if(strcmp(str, "430")==0)
    {
        return LANG_430;
    }
    return LANG_DEFAULT;
}

int main(int argc, char** argv)
{
    FILE* outputFile;
    GLSLShader result;
    GLLang language = LANG_DEFAULT;
	int returnValue = 0;//EXIT_SUCCESS
    Timer_t timer;
    int compiledOK = 0;
    double crossCompileTime = 0;
    double glslCompileTime = 0;

    printf("args: bytecode-file [output-file] [language override - es100 es300 120 130 etc.]\n");

    if(argc < 2 || !fileExists(argv[1]))
    {
        printf("Bad args. Supply a valid shader path, optionaly followed by the output path\n");
        return 1;//EXIT_FAILURE
    }

    if(argc > 3)
    {
        language = LanguageFromString(argv[3]);
    }

    InitTimer(&timer);

    ResetTimer(&timer);
    compiledOK = TranslateHLSLFromFile(argv[1], 0, language, NULL, &result);
    crossCompileTime = ReadTimer(&timer);

    if(compiledOK)
    {
        printf("cc time: %.2f us\n", crossCompileTime);

        if(argc > 2)
        {
            //Dump to file
            outputFile = fopen(argv[2], "w");
            fprintf(outputFile, result.sourceCode);
            fclose(outputFile);
        }

#if defined(VALIDATE_OUTPUT)
        compiledOK = TryCompileShader(result.shaderType, (argc > 2) ? argv[2] : "", result.sourceCode, &glslCompileTime);
        
        if(!compiledOK)
		{
			returnValue = 1;//EXIT_FAILURE
		}
        else
        {
            printf("glsl time: %.2f us\n", glslCompileTime);
        }
#endif

        bcstrfree(result.sourceCode);
    }

	return returnValue;
}
