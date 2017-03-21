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
//    This  is a simple example that draws a quad with a 2D
//    texture image. The purpose of this example is to demonstrate 
//    the basics of 2D texturing
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h> /* memset  */
#include <malloc.h>
//#include "conformal.h"
#include "esUtil.h"
//#include "parseargs.h"
//#include "display.h"
#include "capabilities.h"
//#include "testpattern.h" /* init_test_pattern */
#include "device.h" /* init_source_device, set_device_capture_parms  */
//#incluede <EGL/eglext.h>
#include "/usr/include/EGL/eglext.h"
#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>
//#include <GL/glut.h>

/* local prototypes  */
void init_params(Cmdargs_t *args);
int setup_capture_source(Cmdargs_t argstruct, Sourceparams_t * sourceparams);
int init_source_device(Cmdargs_t argstruct, Sourceparams_t * sourceparams,
		      Videocapabilities_t * capabilities);
//int set_capture_parameters(Sourceparams_t * sourceparams, Videocapabilities_t * capabilities);

/* end local prototypes  */

GLint im_width;
GLint im_height;
GLint win_width;
GLint win_height;
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
	"gl_FragColor = texture2D(s_texture, v_texCoord);	\n"
	  //"gl_FragColor = clamp(res, vec4(0), vec4(1));	\n"
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

///
// Draw a triangle using the shader pair created in Init()
//
void Draw ( ESContext *esContext ,Sourceparams_t * sourceparams)
{
	#define _height 1080
#define _width 1920
#define _bitsperpixel 24
#define _planes 1
#define _compression 0
#define _pixelbytesize _height*_width*_bitsperpixel/8
#define _filesize _pixelbytesize+sizeof(bitmap)
#define _xpixelpermeter 0x130B //2835 , 72 DPI
#define _ypixelpermeter 0x130B //2835 , 72 DPI
#define pixel 0xFF
	
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
     int framesize;
	void * nextframe;
	
	
typedef struct{
    uint8_t signature[2];
    uint32_t filesize;
    uint32_t reserved;
    uint32_t fileoffset_to_pixelarray;
} fileheader;

typedef struct{
    uint32_t dibheadersize;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitsperpixel;
    uint32_t compression;
    uint32_t imagesize;
    uint32_t ypixelpermeter;
    uint32_t xpixelpermeter;
    uint32_t numcolorspallette;
    uint32_t mostimpcolor;
} bitmapinfoheader;

typedef struct {
    fileheader fileheader;
    bitmapinfoheader bitmapinfoheader;
} bitmap;
	
	//make pixmap
	bitmap *pbitmap  = (bitmap*)calloc(1,sizeof(bitmap));
    uint8_t *mybuf = (uint8_t*)malloc(_pixelbytesize+sizeof(bitmap));
    strcpy(pbitmap->fileheader.signature,"BM");
    pbitmap->fileheader.filesize = _filesize;
    pbitmap->fileheader.fileoffset_to_pixelarray = sizeof(bitmap);
    pbitmap->bitmapinfoheader.dibheadersize =sizeof(bitmapinfoheader);
    pbitmap->bitmapinfoheader.width = _width;
    pbitmap->bitmapinfoheader.height = _height;
    pbitmap->bitmapinfoheader.planes = _planes;
    pbitmap->bitmapinfoheader.bitsperpixel = _bitsperpixel;
    pbitmap->bitmapinfoheader.compression = _compression;
    pbitmap->bitmapinfoheader.imagesize = _pixelbytesize;
    pbitmap->bitmapinfoheader.ypixelpermeter = _ypixelpermeter ;
    pbitmap->bitmapinfoheader.xpixelpermeter = _xpixelpermeter ;
    pbitmap->bitmapinfoheader.numcolorspallette = 0;	
	
	memcpy(mybuf,pbitmap,sizeof(bitmap));
	uint8_t *pixpointer;
	pixpointer=mybuf+sizeof(bitmap);
	int ii,jj;
	for(ii=0;ii<1920;ii++){
		for(jj=0;jj<1080;jj++){
			*pixpointer=(uint8_t)120;	//red
			pixpointer++;
			*pixpointer=(uint8_t)120; //blue
			pixpointer++;
			*pixpointer=(uint8_t)220; //blue
			pixpointer++;
				
		}
	}
	

	EGLint attribList[] =
   {
       EGL_RED_SIZE,       5,
       EGL_GREEN_SIZE,     6,
       EGL_BLUE_SIZE,      5,
       EGL_ALPHA_SIZE,     (ES_WINDOW_RGB & ES_WINDOW_ALPHA) ? 8 : EGL_DONT_CARE,
       EGL_DEPTH_SIZE,     (ES_WINDOW_RGB & ES_WINDOW_DEPTH) ? 8 : EGL_DONT_CARE,
       EGL_STENCIL_SIZE,   (ES_WINDOW_RGB & ES_WINDOW_STENCIL) ? 8 : EGL_DONT_CARE,
       EGL_SAMPLE_BUFFERS, (ES_WINDOW_RGB & ES_WINDOW_MULTISAMPLE) ? 1 : 0,
       EGL_NONE
   };
    
   fprintf(stderr, "-----------------error1 image egl\n");   	
   
   
 		EGLImageKHR eglImage;
 	     eglImage = eglCreateImageKHR(esContext->eglDisplay, EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR,(EGLClientBuffer)&mybuf, attribList);
 	     fprintf(stderr, "-----------------error2 image egl\n");   	
 	     GLint err = glGetError();
 	     if (err != GL_NO_ERROR)
			fprintf(stderr, "-----------------error0000 image egl\n");   	
 	      
 	     GLuint textureId;
		 glGenTextures(1, &textureId);
		 glBindTexture(GL_TEXTURE_2D, textureId);
	 
 	     glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImage);
 	     err = glGetError();
 	     if (err != GL_NO_ERROR)
			fprintf(stderr, "-----------------error image egl\n");   	
		userData->textureId=textureId;	
 	       //  textureIsBound = true;
 	     // Once the egl image is bound, the texture becomes a new sibling image and we can safely
 	     // destroy the EGLImage we created for the pixmap:
			if (eglImage != EGL_NO_IMAGE_KHR)
				eglDestroyImageKHR(esContext->eglDisplay, eglImage);
 	     fprintf(stderr, "-----------------error000022222222 image egl\n");   	

	struct timeval t1, t2;
    struct timezone tz;
    float deltatime;
    
    
	
	
   
	
	nextframe = next_device_frame(sourceparams, &framesize);

	if (NULL != nextframe)
{ /*
	gettimeofday ( &t1 , &tz );
	
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, sourceparams->image_width,
	  sourceparams->image_height,/*(GLenum)displaydata->pixelformat*///GL_LUMINANCE_ALPHA,GL_UNSIGNED_BYTE, sourceparams->captured.start);
	/*
	gettimeofday(&t2, &tz);
    deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
    fprintf(stderr, "time spend to subimage2d=%f\n",deltatime);   	
   */
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
}

///
void Update ( ESContext *esContext,Sourceparams_t * sourceparams)
{
	int framesize;
	void * nextframe;
	
	nextframe = next_device_frame(sourceparams, &framesize);

	if (NULL != nextframe)
	{//glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width, height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, buffer );
		//we have aframe
		//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, sourceparams->image_width,
		//  sourceparams->image_height,/*(GLenum)displaydata->pixelformat*/GL_LUMINANCE_ALPHA,GL_UNSIGNED_BYTE, sourceparams->captured.start);
		//fprintf(stderr, "not Idle valid frame");
		//glutPostRedisplay();
		//Recalculate_histogram = 1;
	}
	else
	{
	  //glutPostRedisplay();
	  //fprintf(stderr, "Idle not valid frame");
	  /* show that we're idle (no data available)  */
	  /* fprintf(stderr, "Idle"); */
	}
}
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

void init_params(Cmdargs_t *args)
{
   Videocapabilities_t capabilities;

   strcpy(args->devicename,"/dev/video0");
   args->encoding=UYVY;
   args->image_width=1920;
   args->image_height=1080;


}
int main ( int argc, char *argv[] )
{
	int inWidth,inHeight,outWidth,outHeight,offsetWidth,offsetHeight;
	if(argc != 6) {
		printf ("usage=	imageFileName ,  im_width, im_height, win_width, win_height \n");
		return 1;
	}
	printf( "Hi\n" );
	im_name  = argv[1];
	im_width= atoi (argv[2]);
	im_height= atoi (argv[3]);    
	win_width= atoi (argv[4]);
	win_height= atoi (argv[5]); 

   ESContext esContext;
   UserData  userData;

   
	
////////////////////////////////////////////////
    int argstat, displaystat, retval;
    Cmdargs_t argstruct; /* command line parms  */
    Sourceparams_t sourceparams; /* info about video source  */
    Displaydata_t displaydata; /* info about dest display  */

    memset(&sourceparams, 0, sizeof(sourceparams));
    init_params(&argstruct);
    sourceparams.iomethod = IO_METHOD_MMAP;
    int sourcestatus,capturestat;
    Videocapabilities_t capabilities;
    int status;
    retval = -1;
    
    
    esInitContext ( &esContext );
   esContext.userData = &userData;
	//im_width=640;
	//im_height=480;
	
   esCreateWindow ( &esContext, "Simple Texture 2D", win_width, win_height, ES_WINDOW_RGB );

   if ( !Init ( &esContext ) )
      return 0;

   esRegisterDrawFunc ( &esContext, Draw );
   //esRegisterUpdateFunc ( &esContext, Update ); 

	capturestat = setup_capture_source(argstruct, &sourceparams);

	if (0 == capturestat)
	{

	  status = start_capture_device(&sourceparams);

	  if (-1 == status)
		{
		  fprintf(stderr, "Error: unable to start capture device\n");
		}
	  else
		{
		  fprintf(stderr, "\nPress a key in the display window\n");
		  //glutMainLoop();
		  esMainLoop ( &esContext , &sourceparams);

		  status = stop_capture_device(&sourceparams);
		}


    }
    else
    {
        retval = -1;
    }

   //esMainLoop ( &esContext , &sourceparams);

   ShutDown ( &esContext );
   return 0;
}


/* *************************************************************************
   NAME:  setup_capture_source
   USAGE:
   int status;
   Cmdargs_t argstruct;
   Sourceparams_t  sourceparams;
   argstat =  parse_command_line(argc, argv, &argstruct);
   status =  setup_capture_source(argstruct, &sourceparams);
   returns: int
   DESCRIPTION:
                 given the data from the command line, set up the source for
		 the video. if a device file was given, we can  set up
		 that device. if none was given we can set up a test pattern.
		 return 0 if all's well
		        -1 and complain if it's not
   REFERENCES:
   LIMITATIONS:
   GLOBAL VARIABLES:
      accessed:
      modified:
   FUNCTIONS CALLED:
   REVISION HISTORY:
        STR                  Description of Revision                 Author
      1-Jan-07               initial coding                           gpk
 ************************************************************************* */

int setup_capture_source(Cmdargs_t argstruct, Sourceparams_t * sourceparams)
{
    int sourcestatus, capturestatus, retval;
    Videocapabilities_t capabilities;

    retval = -1;

    sourcestatus = init_source_device (argstruct, sourceparams, &capabilities);

    if (0 == sourcestatus)
    {
        capturestatus = set_device_capture_parms(sourceparams, &capabilities);

        if (0 == capturestatus)
        {
          retval = connect_source_buffers(sourceparams);
        }
    }

  return(retval);

}



/* *************************************************************************
   NAME:  init_source_device
   USAGE:
   int some_int;
   Cmdargs_t argstruct;
   Sourceparams_t sourceparams;
   Videocapabilities_t  capabilities;
   some_int =  init_source_device(argstruct, &sourceparams, &capabilities);
   if (0 == some_int)
   -- continue
   else
   -- handle an error
   returns: int
   DESCRIPTION:
                 open the video device given in argstruct.devicename;
		 store its capabilities in capabilities
		 return 0 if all's well
		       -1 if malloc fails or we can't read the device's
		             capture capabilities
   REFERENCES:
   LIMITATIONS:
   GLOBAL VARIABLES:
      accessed: none
      modified: none
   FUNCTIONS CALLED:
   REVISION HISTORY:
        STR                  Description of Revision                 Author
      4-Jan-07               initial coding                           gpk
      5-Jan-08  iomethod now set in separate function instead of      gpk
                being hardcoded.
     20-Jan-08	removed captured.start buffer after profiling showed  gpk
                copying data into it to be a hot spot. now just
		point captured.start to the data buffers from the
		device or test pattern.
 ************************************************************************* */

int init_source_device(Cmdargs_t argstruct, Sourceparams_t * sourceparams,
		       Videocapabilities_t * p_capabilities)
{
  int fd, retval, buffersize;

  /* open it and make sure it's a character device  */

  fd = verify_and_open_device(argstruct.devicename);

  if (0 > fd)
    {
      retval = -1; /* error  */
    }
  else
    {
      /* fill in sourceparams with the image size and encoding  */
      /* from the command line.  */
      sourceparams->source = LIVESOURCE;
      sourceparams->fd = fd;
      sourceparams->encoding = argstruct.encoding;
      sourceparams->image_width = argstruct.image_width;
      sourceparams->image_height = argstruct.image_height;

      /* start here  */
      /* now allocate a buffer to hold the data we read from  */
      /* this device.   */
         /* ali
          buffersize = compute_bytes_per_frame(argstruct.image_width,
                           argstruct.image_height,
                           argstruct.encoding);
          sourceparams->captured.start = NULL;
          sourceparams->captured.length = buffersize;
        */
        /* 4 bytes represents 2 pixels: UYVY  */
		buffersize = argstruct.image_width * argstruct.image_height * 2;
		sourceparams->captured.start = NULL;
        sourceparams->captured.length = buffersize;
      /* now get the device capabilities and select the io method  */
      /* based on them.   */

      retval = get_device_capabilities(argstruct.devicename, fd, p_capabilities);
/*
      if (0 == retval)
        {
          select_io_method(sourceparams, capabilities);
        }
 */   }

  //retval=0;
  return(retval);
}
