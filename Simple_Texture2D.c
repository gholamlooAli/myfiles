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
GLint im_width,im_height;
GLfloat image_width,image_height;
GLfloat	image[2];//={640.0f,480.0f};
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
		fprintf(stderr, "%s error detected: \n",label);  
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
   GLint imwLoc;
   GLint imhLoc;	
   
   GLbyte vShaderStr[] =  
      "attribute vec4 a_position;   \n"
      "attribute vec2 a_texCoord;   \n"
//"uniform float image_width;			\n"
      "varying vec2 v_texCoord;     \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = a_position; \n"
      "   v_texCoord = a_texCoord;  \n"
      "}                            \n";
   
   GLbyte fShaderStr[] =  
      "precision mediump float;                            \n"
      "varying vec2 v_texCoord;                            \n"
      "uniform sampler2D s_texture;                        \n"
	"uniform vec2 uimage_width;			\n"
//	"uniform vec1 image_height;			\n"
      "void main()                                         \n"
	

      "{                                                   \n"
      //"  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
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
	  "texture_width=uimage_width.x;	\n"
	  "texture_height=uimage_width.y;	\n"
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
      "}                                                   \n";


  
  	



   // Load the shaders and get a linked program object
   userData->programObject = esLoadProgram ( vShaderStr, fShaderStr );

   // Get the attribute locations
   userData->positionLoc = glGetAttribLocation ( userData->programObject, "a_position" );
   userData->texCoordLoc = glGetAttribLocation ( userData->programObject, "a_texCoord" );
   
   // Get the sampler location
   userData->samplerLoc = glGetUniformLocation ( userData->programObject, "s_texture" );

   // Get the image_width location
   imwLoc = glGetUniformLocation ( userData->programObject, "uimage_width" );
   if(-1== imwLoc)
   {
	fprintf(stderr, "warning imwLoc error\n");
	check_error("war imwLoc");
   }
   else
   {
	glUniform2fv(imwLoc,1,image);
	check_error("war after imwLoc");	
   }   
			
   // Get the image_height location
 /*  imhLoc = glGetUniformLocation ( userData->programObject, "image_height" );
   if(-1== imhLoc)
   {
	fprintf(stderr, "warning imhLoc error\n");
		check_error("war imhLoc");
   }
   else
   {
	glUniform1fv(imhLoc, im_height);
		check_error("war after imhLoc");
   }  
 */
   // Load the texture
   //userData->textureId = CreateSimpleTexture2D ();
   userData->textureId=LoadTexture ("/home/linaro/frame6.raw",im_width,im_height);
   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   return GL_TRUE;
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

int main ( int argc, char *argv[] )
{
   ESContext esContext;
   UserData  userData;

   esInitContext ( &esContext );
   esContext.userData = &userData;
	im_width=640;
	im_height=480;
	image[0]=(GLfloat)im_width;
	image[1]=(GLfloat)im_height;
	//image_width=640.0;//(float)im_width;

	//image_height=480.0;//(float)im_height;

   esCreateWindow ( &esContext, "Simple Texture 2D",im_width,im_height/*640, 480*/, ES_WINDOW_RGB );

   if ( !Init ( &esContext ) )
      return 0;

   esRegisterDrawFunc ( &esContext, Draw );

   esMainLoop ( &esContext );

   ShutDown ( &esContext );
}
