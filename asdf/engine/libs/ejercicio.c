#include "opengl.c"
#include "openal.c"
#include "math.c"
#include "model.c"
#include "fonts.c"
#include "camera.c"
#include "terrain.c"
#include "pSystem.c"
#include "sprite.c"
#include "shader.c"
#include "shadow.c"
#include "skybox.c"
#include "collision.c"

/* Especificaciones */
#define WIDTH      1366
#define HEIGHT     768
#define BPP        32
#define DEPTH      16
#define FULLSCREEN GL_TRUE


/*** Fuente  ***/
FONT  fontArial;
COLOR fontColor  = WHITE;

/*** FPS and Text***/
GLfloat totalTime = 0.0f;
char    fpsText[10];
char    timeText[15];
char    collisionText[20];
char    posText[50];

/*** Colisiones **/
GLboolean collision  = GL_FALSE;
GLboolean near       = GL_FALSE;
GLfloat   epsilon    = 0.1f;
GLfloat   iterations = 10;
VECTOR    gravity    = { 0.0f, -9.8f, 0.0f };

/*** Modelos ***/
MODEL    model;
VOLUMES  modelVolumes;
VOLUMES  cameraVolumes;
// Bounding Box
GLuint   boundingBox;
MATERIAL boxMaterial = {BLUE, BLUE, BLACK, BLACK, 0.0f};
// Lista de objetos
GLuint   nObjs = 2;
MODEL*   objList[2]    = { NULL, &model };
VOLUMES* objVolumes[2] = { &cameraVolumes, &modelVolumes };

/*** Camara ***/
CAMERA cam = { {  50.0f, 20.0f, 600.0f }, // pos
	       {   1.0f,  0.0f,   0.0f }, // right
	       {   0.0f,  1.0f,   0.0f }, // up
	       {   0.0f,  0.0f,  -1.0f }, // look
	       GL_FALSE, GL_FALSE,        // walk
	       GL_FALSE, GL_FALSE,        // strafe
	       GL_FALSE, GL_FALSE,        // fly
	       GL_FALSE, GL_FALSE, 60.0f, // pitch
	       GL_FALSE, GL_FALSE,        // yaw
	       GL_FALSE, GL_FALSE, 20.0f  // roll
};
GLfloat vel = 30.0f; // Velocidad de movimiento
GLfloat sen = 10.0f; // Sensibilidad del mouse
VECTOR  cameraAxes = { 2.0f, 5.0f, 2.0f }; // Ejes de elipse rodeante

/*** Luces ***/
// Sol
COLOR    ambColor = BLACK;
DIRLIGHT dirLight = { WHITE, WHITE, BLACK,
		      {0.0f, 1.0f, 1.0f} };

/*** Terreno ***/
TERRAIN  terrain;
MATERIAL terrainMtrl = {GRAY, GRAY, BLACK, BLACK, 0.0f};

/*** Skybox ***/
SKYBOX skybox;
BOX    skyboxBox   = {{0.0f, 0.0f, 0.0f}, {630.0f, 300.0f, 630.0f}};
COLOR  skyboxColor = WHITE;


/*** Inicialización de recursos ***/
void Init( void )
{
  // Inicio OpenGL
  InitOpenGL( WIDTH, HEIGHT, BPP, DEPTH, FULLSCREEN );
  SetOpenGL( WIDTH, HEIGHT, (GLfloat*)&ambColor, (GLfloat*)&ambColor );
  
  // Incio OpenAL
  InitOpenAL( NULL );

  // Escondo el cursor
  SDL_ShowCursor( SDL_DISABLE );

  // Inicio y cargo la fuente
  InitFont( WIDTH, HEIGHT );
  fontArial = OpenFont( "arial.ttf", 16 );
  SetFontStyle( fontArial, TTF_STYLE_NORMAL );

  // Modelo
  LoadModel( "models/Wooden Box.obj", "textures", GL_FALSE, &model );
  BoundingVolumes( &model, &modelVolumes, NULL, GL_FALSE );
  // Bounding Box
  boxMaterial.diffuse.a = 0.2f;
  boundingBox = RenderBoundingBox( &modelVolumes.box.min, 
				   &modelVolumes.box.max,
				   &boxMaterial );
  
  // Cámara
  CreateCameraVolume( cam, cameraAxes, &cameraVolumes );

  // Terreno
  InitTerrain( &terrain, "coastMountain64.raw", "textures/grass.png", 
	       GL_FALSE, &terrainMtrl, 64, 64, 10.0f, 1.0f );

  // Skybox
  InitSkybox( &skybox, "textures/skybox.png" );
}

/*** Liberación de recursos ***/
void Free( void )
{
  // Libero la letra y las fuentes
  CloseFont( fontArial ); 
  FreeFont();

  // Libero el modelo
  FreeModel( &model );
  glDeleteLists( boundingBox, 1 );

  // Libero el terreno
  FreeTerrain( &terrain );

  // Libero el skybox
  FreeSkybox( &skybox );

  // Último
  FreeOpenAL();
  FreeOpenGL();
}

/*** Procesamiento de los dispositivos de entrada ***/
void Input( float elapsed )
{
  SDL_Event event;
  /* Proceso los eventos de input hasta que no haya más */
  while( SDL_PollEvent( &event ) )
    {
      switch( event.type )
	{
	case SDL_KEYUP:
	  switch( event.key.keysym.sym )
	    {
	    case SDLK_ESCAPE:
	      g_ExitProgram = GL_TRUE;
	      break;
	    case SDLK_PRINT:
	      CreateScreenshot( "Ejercicio" );
	      break;
	    case SDLK_w:
	      cam.walk      = GL_FALSE;
	      break;
	    case SDLK_s:
	      cam.walkinv   = GL_FALSE;
	      break;
	    case SDLK_d:
	      cam.strafe    = GL_FALSE;
	      break;
	    case SDLK_a:
	      cam.strafeinv = GL_FALSE;
	      break;
	    default:
	      PrintError( "Unhandled KEYUP event", GL_TRUE );
	    }
	  break;

	case SDL_KEYDOWN:
	  switch( event.key.keysym.sym )
	    {
	    case SDLK_ESCAPE:
	      break;
	    case SDLK_PRINT:
	      break;
	    case SDLK_w:
	      cam.walk      = GL_TRUE;
	      break;
	    case SDLK_s:
	      cam.walkinv   = GL_TRUE;
	      break;
	    case SDLK_d:
	      cam.strafe    = GL_TRUE;
	      break;
	    case SDLK_a:
	      cam.strafeinv = GL_TRUE;
	      break;
	    default:
	      PrintError( "Unhandled KEYDOWN event", GL_TRUE );
	    }
	  break;

	/* Movimiento del mouse */
	case SDL_MOUSEMOTION:
	  {
	    /* Capturo los movimientos */
	    GLfloat xrel, yrel;
	    xrel = (GLfloat)event.motion.xrel;
	    yrel = (GLfloat)event.motion.yrel;
	    
	    /* Movimiento en x */
	    if( xrel != 0 && xrel < 100 && xrel > -100 )
	      Yaw( &cam, xrel * elapsed * sen, GL_TRUE );
	    
	    /* Movimiento en y */
	    if( yrel != 0 && yrel < 100 && yrel > -100 )
	      Pitch( &cam, yrel * elapsed * sen, GL_TRUE );
	  }
	  break;
	  
	default:
	  PrintError( "Unhandled type of event", GL_TRUE );
	}
      
      /* Borro el mensaje para evitar procesarlo de nuevo */
      memset( &event, 0, sizeof(SDL_Event) );
    }
}

/*** Procesamiento de la lógica del juego ***/
void Logic( float elapsed )
{
  /*** Modelos ***/
  static GLboolean passed = GL_FALSE;
  if( !passed )
    {
      // Escalación
      glScalef( 4.0f, 4.0f, 4.0f );
      BoundingVolumes( objList[1], objVolumes[1], NULL, GL_FALSE );
      // Traslación
      VECTOR disp = { 50.0f, GetHeight( &terrain, 50.0f, 550.0f ), 550.0f };
      UpdateVolumes( objVolumes[1], disp );

      passed = !passed;
    }

  /*** Movimiento ***/
  VECTOR origin = cam.pos;
  /* Walk Fordward */
  if( cam.walk )
    Walk( &cam, elapsed * vel, GL_TRUE );
  /* Walk Backward */
  if( cam.walkinv )
    Walk( &cam, elapsed * -vel, GL_TRUE );
  /* Strafe Right */
  if( cam.strafe )
    Strafe( &cam, elapsed * vel, GL_TRUE );
  /* Strafe Left */
  if( cam.strafeinv )
    Strafe( &cam, elapsed * -vel, GL_TRUE );

  VECTOR disp  = ResVector( cam.pos, origin );

  /* Colisión de esferas */
  GLfloat time;
  VECTOR  cPos;
  if( SphereCollision( objVolumes[0], objVolumes[1],
		       disp, &time, &cPos ) )
    near = !near;

  /* Detección de colisión de la cámara */
  /* Desplazamiento */
  cam.pos = ResVector( cam.pos, disp );
  CollisionAndResponse( iterations, epsilon,
			&cam, &terrain, objList, objVolumes,
			nObjs, 0, disp );

  /* Gravedad */
  disp = MulVector( gravity, elapsed );
  CollisionAndResponse( iterations, epsilon,
			&cam, &terrain, objList, objVolumes,
			nObjs, 0, disp );

  /* Tiempo total transcurrido */
  totalTime += elapsed;
}

/*** Procesamiento de cada cuadro de sonido ***/
void Sound( float elapsed )
{
}

/*** Procesamiento de cada cuadro ***/
void Video( float elapsed )
{
  /* Limpio la pantalla */
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

  /* Perspectiva */
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluPerspective( 45.0f,                            // Ángulo de visión vertical
		  (GLfloat)WIDTH / (GLfloat)HEIGHT, // Aspecto
		  1.0f,                             // Plano cercano
		  1500.0f );                        // Plano lejano

  /* Cámara */
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  VECTOR dir = SumVector( cam.pos, cam.look );
  gluLookAt( cam.pos.x , cam.pos.y , cam.pos.z , // Posición
	     dir.x     , dir.y     , dir.z,      // Dirección
	     0.0f, 1.0f, 0.0f );                 // Arriba(Persona)

  // Luces
  SetDirLight( GL_LIGHT0, &dirLight );
  glEnable( GL_LIGHT0 );
  
  /*** Modelo ***/
  glPushMatrix();

  glMultTransposeMatrixf( modelVolumes.matrix );
  glCallList( model.modelList );
  //glCallList( boundingBox );

  glPopMatrix();
  /*________*/

  /*** Terreno ***/
  glPushMatrix(); // Guardo la cámara

  SetMaterial( &terrain.material );
  glBindTexture( GL_TEXTURE_2D, terrain.textureID );
  glCallList( terrain.terrainList );

  glPopMatrix(); // Restauro la cámara
  /*________*/

  /*** Skybox ***/
  glPushMatrix(); // Guardo la cámara
  RenderSkybox( &skybox, &skyboxBox, &skyboxColor );
  glPopMatrix(); // Restauro la cámara

  /*________*/

  /* FPS */
  sprintf( fpsText, "%d fps", CalculateFPS( elapsed ) );
  RenderText( fpsText, fontArial, 0, 0, &fontColor, GL_FALSE );

  /* Time */
  sprintf( timeText, "Time: %.2fs", totalTime );
  RenderText( timeText, fontArial, 0,
  	      GetFontLineSkip( fontArial ), &fontColor, GL_FALSE );

  /* Colisión */
  sprintf( collisionText, "Near: %s", near ? "True" : "False" );
  RenderText( collisionText, fontArial, 0,
	      GetFontLineSkip( fontArial ) * 3, &fontColor, GL_FALSE );
  sprintf( collisionText, "Colliding: %s", collision ? "True" : "False" );
  RenderText( collisionText, fontArial, 0,
	      GetFontLineSkip( fontArial ) * 4, &fontColor, GL_FALSE );

  /* Posición */
  sprintf( posText, "Camera pos: x=%.1f y=%.1f z=%.1f", 
	   cameraVolumes.sphere.center.x, 
	   cameraVolumes.sphere.center.y, 
	   cameraVolumes.sphere.center.z );
  RenderText( posText, fontArial, WIDTH - 280, 0, &fontColor, GL_FALSE );
  sprintf( posText, "Sphere pos: x=%.1f y=%.1f z=%.1f", 
	   modelVolumes.sphere.center.x, 
	   modelVolumes.sphere.center.y, 
	   modelVolumes.sphere.center.z );
  RenderText( posText, fontArial, WIDTH - 280,
	      GetFontLineSkip( fontArial ) , &fontColor, GL_FALSE );
    
  /* Ejecutar comandos en cola */
  glFlush();

  /* BackBuffer=FrontBuffer(flip) */
  SDL_GL_SwapBuffers();
}

/*** Loop ***/
void Loop( float elapsed )
{
  /* Input, Lógica, Sonido y Video */
  Input( elapsed );
  Logic( elapsed );
  Sound( elapsed );
  Video( elapsed );
}
