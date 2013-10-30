/*****************************/
/**       ----------        **/
/**        opengl.c         **/
/**       ----------        **/
/**  Rutinas necesarias pa  **/
/**  ra inicializar opengl  **/
/*****************************/

#define GL_GLEXT_PROTOTYPES 1 // Extensiones presentes
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <SDL/SDL.h>
#include "SDLMain.h"
#include <SDL_image/SDL_image.h>
#include <SDL_ttf/SDL_ttf.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <vorbis/vorbisfile.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>




/*** Variable para indicar la salida del programa ***/
GLboolean g_ExitProgram = GL_FALSE;

/*** Funciones del ejercicio ***/
// Implementadas en "Ejercicio.cpp" //
void Init( void );
void Free( void );
void Loop( float elapsed );

/* Función de diagnóstico de error de SDL */
void Error_SDL( const char* error )
{
  fprintf( stderr, "%s: %s", error, SDL_GetError() );
}
 
/*** Función inicializadora de SDL y OpenGL ***/
GLboolean InitOpenGL(int width, int height,       // Dimensiones de la ventana
		     int bpp,                     // Profundidad
		     int depth,                   // Z-buffer
		     GLboolean fullscreen)        // Fullscreen?
{
  /* Inicialización de SDL */
  if( SDL_Init( SDL_INIT_VIDEO ) != 0 )
    {
      Error_SDL( "Error initializing SDL" );
      return GL_FALSE;
    }
  
  /* Pongo el fondo en 32 bits */
  if( bpp == 32 )
    {
      SDL_GL_SetAttribute( SDL_GL_RED_SIZE  , 8 );
      SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
      SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE , 8 );
      SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
    }
  else if( bpp == 16 ) // 16 bits
    {
      SDL_GL_SetAttribute( SDL_GL_RED_SIZE  , 4 );
      SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 4 );
      SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE , 4 );
      SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 4 );
    }
  
  /* Framebuffer */
  SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, bpp );
  
  /* Z-buffer */
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, depth );
  
  /* Stencil buffer */
  SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
  
  /* Habilito el doble-buffering de OpengGL */
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  
  /* Creo la ventana con soporte OpenGL */
  if( SDL_SetVideoMode( width, height, bpp, 
			SDL_OPENGL | (fullscreen ? SDL_FULLSCREEN : 0) ) 
      == NULL )
    {
      Error_SDL( "Could not init OpenGL" );
      return GL_FALSE;
    }
  
  /* Título de la ventana */
  SDL_WM_SetCaption( "OpenGL", "OpenGL" );
  
  /* Mensajes de Input directo a la aplicación */
  SDL_WM_GrabInput( SDL_GRAB_ON );
  /* Deshabilito la repetición del teclado */
  SDL_EnableKeyRepeat( 0, SDL_DEFAULT_REPEAT_INTERVAL );
  
  /* Inicializo los número aleatorios */
  srand( time(NULL) );
  
  return GL_TRUE;
}

/*** Función liberadora de SDL y OpengGL ***/
GLboolean FreeOpenGL( void )
{
    /* Libero la interfaz a SDL */
    SDL_Quit();
    
    return GL_TRUE; // Éxito
}

/*** Función: configura openGL ***/
void SetOpenGL( int width, int height      ,  // Dimensión del la ventana
		const GLfloat* clearColor  ,  // Color de borrado
		const GLfloat* ambientColor ) // Color de luz ambiente natural
{
  // Habilito el uso de arreglos de vértices, normales y texturas
  glEnableClientState( GL_VERTEX_ARRAY );
  glEnableClientState( GL_NORMAL_ARRAY );
  glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    
  // Sólido
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

  // Habilito back-face culling
  glEnable( GL_CULL_FACE );
  // Descarte los polígonos mirando hacia atrás
  glCullFace( GL_BACK );
  // Cara delantera sentido horario
  glFrontFace( GL_CW );

  // Habilito el Z-buffer
  glEnable( GL_DEPTH_TEST );
  // La función de profundidad
  glDepthFunc( GL_LEQUAL );

  // Habilito luces
  glEnable( GL_LIGHTING );
  // Suavizado para los colores de luz
  glShadeModel( GL_SMOOTH );
  // Normalizar los vértices normales
  glEnable( GL_NORMALIZE );
  // Luz ambiente promedio
  glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambientColor );
  // Habilito las luces especulares respecto a la cámara
  glLightModeli( GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR );
  glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE );

  // Habilito texturas
  glEnable( GL_TEXTURE_2D );
  // Filtro de las texturas
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  // Especifico la forma de blending(transparencia)
  glBlendEquation( GL_FUNC_ADD );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glAlphaFunc( GL_GREATER, 0 );

  // Habilito los puntos con AA
  glEnable( GL_POINT_SMOOTH );
    
  // Mejoras
  glHint( GL_POINT_SMOOTH_HINT          , GL_NICEST );
  glHint( GL_LINE_SMOOTH_HINT           , GL_NICEST );
  glHint( GL_POLYGON_SMOOTH_HINT        , GL_NICEST );
  glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
  glHint( GL_GENERATE_MIPMAP_HINT       , GL_NICEST );
  glHint( GL_TEXTURE_COMPRESSION_HINT   , GL_NICEST );
  glHint( GL_FOG_HINT                   , GL_NICEST );

  // Color de limpiado
  glClearColor( clearColor[0], clearColor[1], clearColor[2], clearColor[3] );
  // Limpiado del Z-buffer
  glClearDepth( 1.0f );
  // Limipado del Stencil-buffer
  glClearStencil( 0 );

  // Viewport
  glViewport( 0, 0, width, height );
}

/*** Main ***/
int main( int argc, char** argv )
{
  Init();    // Inicializo recursos

  /* Conteo del tiempo */
  struct timeval iTime = { 0, 0 };
  struct timeval fTime = { 0, 0 };
  float elapsed        = 0.0f;
  gettimeofday( &iTime, NULL );

  /* Loop Init */
  while( !g_ExitProgram )
  {
      gettimeofday( &fTime, NULL );                            // Tiempo Actual
      elapsed = ( fTime.tv_usec - iTime.tv_usec ) / 1000000.0f // Microsegundos
	        + ( fTime.tv_sec - iTime.tv_sec );             // Segundos
      iTime = fTime;                                           // Actualizo el tiempo

      Loop( elapsed );    // Mensajes y Loop(video, sonido, input, etc...)
  }
  /* Loop End */

  Free();    // Libero Recursos
  
  return 0;
}
