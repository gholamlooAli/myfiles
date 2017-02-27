
//
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com
//

// Simple_Texture2D.c
//
//    This is a simple example that draws a quad with a 2D
//    texture image. The purpose of this example is to demonstrate
//    the basics of 2D texturing
//
#include <stdlib.h>
#include <stdio.h>
#include "esUtil.h"
//#include <GL/glut.h>
GLint im_width;
GLint im_height;
char *im_name;//="/home/linaro/frame6.raw";

GLint imwLoc;
//GLint imhLoc;

typedef struct
{
   // Handle to a program object
   GLuint programObject;

   // Attribute locations
   GLint  positionLoc;
   GLint  texCoordLoc;

   // Sampler location
   GLint samplerLoc;

   // Texture handle
   GLuint textureId;

} UserData;

void check_error(char *label)
{
	GLenum error;
	while ((error= glGetError()) != GL_NO_ERROR)
	{
		if(error == GL_INVALID_VALUE)
			fprintf(stderr, "%s INVALIDerror detected:%d \n",label,error);
		if(error == GL_INVALID_OPERATION)
			fprintf(stderr, "%s INVALI OP Derror detected:%d \n",label,error);
	}
}

char* esLoadRaw ( char *fileName, int width, int height )
{
    char *buffer = NULL;
    FILE *f;
    unsigned int imagesize;

    f = fopen(fileName, "rb");
    if(f == NULL) return NULL;

    imagesize = 2 * width * height;
    buffer = malloc(imagesize);
    if (buffer == NULL)
    {
        fclose(f);
        return 0;
    }

    if(fread(buffer, 1, imagesize, f) != imagesize)
    {
        free(buffer);
        return NULL;
    }
    fclose(f);
    return buffer;
}
///
// Load texture from disk
//
GLuint LoadTexture ( char *fileName ,int width ,int height)
{
   //int width,height;
   char *buffer = esLoadRaw ( fileName, width, height );
   GLuint texId;
   if ( buffer == NULL )
   {
      esLogMessage ( "Error loading (%s) image.\n", fileName );
      return 0;
   }

   glGenTextures ( 1, &texId );
   glBindTexture ( GL_TEXTURE_2D, texId );

   glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width, height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, buffer );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

   free ( buffer );

   return texId;
}


///
// Create a simple 2x2 texture image with four different colors
//
GLuint CreateSimpleTexture2D( )
{
   // Texture object handle
   GLuint textureId;

   // 2x2 Image, 3 bytes per pixel (R, G, B)
   GLubyte pixels[4 * 3] =
   {
      255,   0,   0, // Red
        0, 255,   0, // Green
        0,   0, 255, // Blue
      255, 255,   0  // Yellow
   };

   // Use tightly packed data
   glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

   // Generate a texture object
   glGenTextures ( 1, &textureId );

   // Bind the texture object
   glBindTexture ( GL_TEXTURE_2D, textureId );

   // Load the texture
   glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels );
   // Set the filtering mode
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

   return textureId;

}


///
// Initialize the shader and program object
//
int Init ( ESContext *esContext )
{
   esContext->userData = malloc(sizeof(UserData));
   UserData *userData = esContext->userData;


   GLbyte vShaderStr[] =
/*	"#version 300 es \n"
	"layout (location=0) in vec4 a_position;   \n"
	"layout (location=1) in vec2 a_texCoord;   \n"
	"out  vec2 v_texCoord;     \n"
/*/	//version 2.0
      "attribute vec4 a_position;   \n"
      "attribute vec2 a_texCoord;   \n"
      "varying vec2 v_texCoord;     \n"

      "void main()                  \n"
      "{                            \n"
      "   gl_Position = a_position; \n"
      "   v_texCoord = a_texCoord;  \n"
      "}                            \n";

   GLbyte fShaderStr[] =
     // "#version 300 es 			\n"


      "precision mediump float;                            \n"
	//"in vec2 v_texCoord;                            \n"
      	"varying vec2 v_texCoord;                            \n"

	"uniform mediump int im_h;			\n"
	"uniform mediump int uimage_width;			\n"
	"uniform sampler2D s_texture;                        \n"
	//"layout (location=0) out vec4 outColor;   \n"
	//"layout (location=1) int uimage_width;			\n"
      "void main()                                         \n"


      "{                                                   \n"

	  "float red, green, blue;       \n"
	  "vec4 luma_chroma;	       \n"
	  "float luma, chroma_u,  chroma_v;       \n"
	  "float pixelx, pixely;	       \n"
	  "float xcoord, ycoord;       \n"
	  "float texture_width, texture_height;       \n"
	  "float texel_width, texel_height;       \n"
	  "float x, y;		       \n"
	  "float u, v;		       \n"
	  "vec3 yuv;		       \n"
	  "vec4 res;		       \n"
	// note: pixelx, pixely are 0.0 to 1.0 so "next pixel horizontally"
  	//  is not just pixelx + 1; rather pixelx + texel_width.

	  "pixelx = v_texCoord.x;	\n"
	  "pixely = v_texCoord.y;	\n"
	  "texture_width=float(uimage_width);	\n"
	  "texture_height=float(im_h);	\n"
	  "texel_width=1.0/texture_width; \n"
	  "texel_height=1.0/texture_height; \n"
	  //"x = pixelx - (IMAGE_WIDTH * 0.5) / texture_width;	\n"
	  //"y = pixely - (IMAGE_HEIGHT * 0.5) / texture_height;	\n"

	  //"u = x*x - y*y;	\n"
	  //"v = 2.0*x*y;	\n"

	  //"u = u + (IMAGE_WIDTH * 0.5) / texture_width;		\n"
	  //"v = v + (IMAGE_HEIGHT * 0.5) / texture_height;	\n"

	  //"pixelx = clamp(u, 0.0, (IMAGE_WIDTH - 1.0) / texture_width);		\n"
	  //"pixely = clamp(v, 0.0, (IMAGE_HEIGHT - 1.0) / texture_height);	\n"
	  // if pixelx is even, then that pixel contains [Y U] and the
	  //    next one contains [Y V] -- and we want the V part.
	  // if  pixelx is odd then that pixel contains [Y V] and the
	  //     previous one contains  [Y U] -- and we want the U part.

	  // note: only valid for images whose width is an even number of
	  // pixels

  	  "xcoord = floor (v_texCoord.x * texture_width);	\n"

	  "luma_chroma = texture2D(s_texture, v_texCoord);	\n"

	  // just look up the brightness
	  "luma = (luma_chroma.a - 0.0625) * 1.1643;	\n"
	  //"luma=luma_chroma.a;	\n"
	  "if (0.0 == mod(xcoord , 2.0)) \n"	// even
	   " {				\n"
	   "   chroma_u = luma_chroma.r;	\n"
//	   "   chroma_v = texture2D(s_texture,vec2(pixelx + texel_width, pixely)).r;	\n"
	   "   chroma_v = texture2D(s_texture,vec2(v_texCoord.x + texel_width, v_texCoord.y)).r;	\n"
	   " }	\n"
	  "else \n"// odd
	   " {	\n"
	   "   chroma_v = luma_chroma.r;	\n"
	   "   chroma_u = texture2D(s_texture,vec2(v_texCoord.x - texel_width, v_texCoord.y)).r; 	\n"
//	   "   chroma_u = texture2D(s_texture,vec2(pixelx - texel_width, pixely)).r; 	\n"
	   "}	\n"
	  "chroma_u = chroma_u - 0.5;	\n"
	  "chroma_v = chroma_v - 0.5;	\n"

	  "res.r = luma + 1.5958 * chroma_v;	\n"
	  "res.g = luma - 0.39173 * chroma_u - 0.8129 * chroma_v;	\n"
	  "res.b = luma + 2.017 * chroma_u;	\n"
	  "res.a = 1.0;				\n"
//	  "red = chroma_u ;	\n"
//	  "green = 0.0;	\n"
//	  "blue = 0.0 ;	\n"

//	  "red = luma ;	\n"
//	  "green = luma;	\n"
//	  "blue = luma ;	\n"

	  "gl_FragColor = clamp(res, vec4(0), vec4(1));	\n"
	 //"outColor = clamp(res, vec4(0), vec4(1));	\n"
      "}                                                   \n";







   // Load the shaders and get a linked program object
   userData->programObject = esLoadProgram ( vShaderStr, fShaderStr );

   // Get the attribute locations
   userData->positionLoc = glGetAttribLocation ( userData->programObject, "a_position" );
   userData->texCoordLoc = glGetAttribLocation ( userData->programObject, "a_texCoord" );

   // Get the sampler location
   userData->samplerLoc = glGetUniformLocation ( userData->programObject, "s_texture" );



   // Load the texture
   //userData->textureId = CreateSimpleTexture2D ();
   userData->textureId=LoadTexture (im_name,im_width,im_height);
   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   return GL_TRUE;
}
void Update ( ESContext *esContext )
{
    int framesize;
    void * nextframe;

    nextframe = capture_video_frame(callback.sourceparams, &framesize);

    if (NULL != nextframe)
    {
      glutPostRedisplay();
      Recalculate_histogram = 1;
    }
    else
    {
      glutPostRedisplay();
      /* show that we're idle (no data available)  */
      /* fprintf(stderr, "Idle"); */
    }
}
}
///
// Draw a triangle using the shader pair created in Init()
//
void Draw ( ESContext *esContext )
{
   UserData *userData = esContext->userData;
   GLfloat vVertices[] = { -1.0f,  1.0f, 0.0f,  // Position 0
                            0.0f,  0.0f,        // TexCoord 0
                           -1.0f, -1.0f, 0.0f,  // Position 1
                            0.0f,  1.0f,        // TexCoord 1
                            1.0f, -1.0f, 0.0f,  // Position 2
                            1.0f,  1.0f,        // TexCoord 2
                            1.0f,  1.0f, 0.0f,  // Position 3
                            1.0f,  0.0f         // TexCoord 3
                         };
   GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

   // Set the viewport
   glViewport ( 0, 0, esContext->width, esContext->height );

   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( userData->programObject );

   // Load the vertex position
   glVertexAttribPointer ( userData->positionLoc, 3, GL_FLOAT,
                           GL_FALSE, 5 * sizeof(GLfloat), vVertices );
   // Load the texture coordinate
   glVertexAttribPointer ( userData->texCoordLoc, 2, GL_FLOAT,
                           GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3] );

   glEnableVertexAttribArray ( userData->positionLoc );
   glEnableVertexAttribArray ( userData->texCoordLoc );

   // Bind the texture
   glActiveTexture ( GL_TEXTURE0 );
   glBindTexture ( GL_TEXTURE_2D, userData->textureId );

   // Set the sampler texture unit to 0
   glUniform1i ( userData->samplerLoc, 0 );



	/*
	// Get the image_height location
	imhLoc = glGetUniformLocation ( userData->programObject, "im_h" );

	if(-1== imhLoc)
	   {
		fprintf(stderr, "warning imhLoc error\n");
		check_error("war imhLoc");
	   }
	   else
	   {
		glUniform1i(imhLoc, im_height);
		check_error("war after imhLoc");
	   }
*/
	// Get the image_width location
   	imwLoc = glGetUniformLocation ( userData->programObject, "uimage_width" );

	if(-1== imwLoc)
	   {
		fprintf(stderr, "warning imwLoc error\n");
		check_error("war imwLoc");
	   }
	else
	   {
		glUniform1i(imwLoc,im_width);
		check_error("war after imwLoc");
	   }
   glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

}

///
// Cleanup
//
void ShutDown ( ESContext *esContext )
{
   UserData *userData = esContext->userData;

   // Delete texture object
   glDeleteTextures ( 1, &userData->textureId );

   // Delete program object
   glDeleteProgram ( userData->programObject );

   free(esContext->userData);
}

void init_params(void);
void init_params()
{
   Videocapabilities_t capabilities;

   argstrut->devicename="/dev/video0");
   argstruct->encoding=UYVY;
   argstruct->image_width=1920;
   argstruct->image_height=1080;

   sourceparams->iomethod = IO_METHOD_MMAP;
}

int main ( int argc, char *argv[] )
{
    int argstat, displaystat, retval;
    Cmdargs_t argstruct; /* command line parms  */
    Sourceparams_t sourceparams; /* info about video source  */
    Displaydata_t displaydata; /* info about dest display  */
    int inWidth,inHeight,outWidth,outHeight,offsetWidth,offsetHeight;

    /* end local prototypes  */

    if(argc != 4) {

		printf ("usage=	imageFileName ,  im_width, im_height \n");
		return 1;
	}
	printf( "Hi\n" );
	im_name  = argv[1];
	im_width= atoi (argv[2]);
	im_height= atoi (argv[3]);


   ESContext esContext;
   UserData  userData;

   esInitContext ( &esContext );
   esContext.userData = &userData;
	//im_width=640;
	//im_height=480;

   esCreateWindow ( &esContext, "Simple Texture 2D",im_width,im_height, ES_WINDOW_RGB );

   if ( !Init ( &esContext ) )
      return 0;

   esRegisterDrawFunc ( &esContext, Draw );
   esRegisterUpdateFunc ( &esContext, Update );
////////////////////////////////////////////////
    memset(&sourceparams, 0, sizeof(sourceparams));

    int sourcestatus;
    Videocapabilities_t capabilities;
    int status;
    retval = -1;

    sourcestatus = init_source_device(argstruct, sourceparams, &capabilities);

    if (0 == sourcestatus)
    {
        capturestatus = set_device_capture_parms(sourceparams, &capabilities);

        if (0 == capturestatus)
        {
              retval = connect_source_buffers(sourceparams);
              if(0 == retval )
              {
                  status = start_capture_device(sourceparams);

                  if (-1 == status)
                    {
                      fprintf(stderr, "Error: unable to start capture device\n");
                    }
                  else
                    {
                      fprintf(stderr, "\nPress a key in the display window\n");
                      //glutMainLoop();
                      esMainLoop ( &esContext );

                      status = stop_capture_device(sourceparams);
                    }

              }
              else
              {
                retval=-1;
              }
        }
        else
        {
          retval=-1;
        }
    }
    else
    {
        retval = -1;
    }




   ShutDown ( &esContext );
}
