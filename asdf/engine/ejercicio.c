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
//#define WIDTH      1440
//#define HEIGHT     900

#define WIDTH      1280
#define HEIGHT     768


#define BPP        32
#define DEPTH      16
#define FULLSCREEN GL_FALSE
#define VSYNC      GL_FALSE


/* Cámara*/
CAMERA cam = {
	{50.0f, 20.0f, 630.0f},    //Posición
	{1.0f, 0.0f, 0.0f},        //Right
	{0.0f, 1.0f, 0.0f},        //Up
	{0.0f, 0.0f, -1.0f},        //Look
	
	GL_FALSE, GL_FALSE,        //Walk
	GL_FALSE, GL_FALSE,        //Strafe
	GL_FALSE, GL_FALSE,        //Fly
	GL_FALSE, GL_FALSE, 60.0f,  //Pitch
	GL_FALSE, GL_FALSE,        //Jaw
	GL_FALSE, GL_FALSE, 20.0f  //Roll
	
};

/* VECTORES DE CAMARA*/

VECTOR ACC = { 0.0f, -9.8f * 5.0f , 0.0f};
VECTOR VEL = { 0.0f, 0.0f, 0.0f};
VECTOR POS = { 0.0f, 0.0f, 0.0f};

/* VARIABLES */

GLboolean Brun = GL_FALSE;
GLboolean jump = GL_FALSE;
GLboolean under = GL_FALSE;


GLfloat run  = 1.0f ;
GLfloat velocity  =   150.0f;     //50.0f;
GLfloat velocityAgua = 0.5f;
GLfloat altura = 25.0f;
GLfloat sensibility  =30.0f;

GLfloat corriente = 3.0f;

/*
char fpstex[100];
FONT arialfont;
COLOR fontColor = WHITE;
*/


/* ESPADA*/
MODEL    modeloEspada;
VOLUMES volumesEspada;

GLboolean atk1 = GL_FALSE;
GLfloat angX;
GLfloat angY;
GLfloat angZ;


/*LUCES*/
DIRLIGHT dirLight = { GRAY, WHITE, BLACK, {0.0f, 1.0f, 0.0f } };


/* Terreno */
TERRAIN  terrain;
MATERIAL terrainMtrl = {PALE, WHITE, BLACK, BLACK, 1.0f};




/*material default*/
MATERIAL    mtrl  ={PALE,PALE,GRAY,WHITE,0.5f };


/*SkyBox*/
SKYBOX skybox;
BOX    skyboxBox = {{0.0f, 0.0f,0.0f},{64.0f * 50.0f,640.0f,64.0f * 50.0f}};
COLOR  skyboxColor=GRAY;



GLfloat vWater = 90.0f;
GLfloat vTerrain;

/*PROPIEDADES DE LA BARRA*/
GLuint oxBar ;
GLfloat oxlevel = 50.0f , ox = 50.0f ;
GLboolean oxIn = GL_FALSE;




/*HUD*/
//movimiento del agua...
GLfloat movWater;


//GLuint underTex;

GLuint mapTex,dotTex,UnderwaterTex, hudNull;
GLboolean renderUnderwater= GL_FALSE;
GLfloat zoom = 1.0f;

/* agua */
GLuint water;
MATERIAL aguaMtrl = {PALE,PALE,GRAY,WHITE,0.5f };


/*Mapa*/
GLfloat mapWidth =(GLfloat)WIDTH* 0.2f , mapHeight=(GLfloat)HEIGHT * 0.2f ;
GLfloat dotX, dotZ;


/*** Inicialización de recursos ***/
void Init( void )
{
    // Luz global
    COLOR    ambColor = BLACK;
    
    // INICIALIZANDO FUENTE
    /*
    InitFont(WIDTH, HEIGHT);
    arialfont = OpenFont("fonts/arial.ttf", 18);
    SetFontStyle(arialfont, TTF_STYLE_BOLD);
    */
    
    
    // Inicio OpenGL
    InitOpenGL( WIDTH, HEIGHT, BPP, DEPTH, FULLSCREEN );
    SetOpenGL( WIDTH, HEIGHT, (GLfloat*)&ambColor, (GLfloat*)&ambColor );
    
    // Incio OpenAL
    InitOpenAL( NULL );
    
    // Escondo el cursor
    SDL_ShowCursor( SDL_DISABLE );
    
    //Transparencia
    
    
    glBlendEquation( GL_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    

    //Espada
    glPushMatrix();
    //LoadModel( "models/flyingSword.3DS", "textures", GL_FALSE, &modeloEspada );
    LoadModel( "models/sword7.3ds", "textures", GL_FALSE, &modeloEspada );
    glTranslatef(1.0f, 0.0f, -3.0f);
    //glScalef(0.01f, 0.01f, 0.01f);

    BoundingVolumes(&modeloEspada, &volumesEspada, NULL, GL_TRUE);
    glPopMatrix();
    
    
    
    // Terreno
    InitTerrain(&terrain,"resources/hell.raw", "textures/greengrass.jpg",GL_FALSE, &terrainMtrl,64,64,50.0f,1.0f);
    // agua
//    InitTerrain(&agua,"resources/agua.raw", "textures/agua.png",GL_FALSE, &aguaMtrl,64,64,50.0f,0.1f);
    
    // SKYBOX
    InitSkybox(&skybox,"textures/skybox.png");
    
    
    //HUD
    mapTex = LoadTexture("textures/greenhell.png");
    dotTex = LoadTexture("textures/Dot.png");
    water =  LoadTexture("textures/sprite.png");
    hudNull = LoadTexture("textures/hud.png");
    UnderwaterTex =LoadTexture("textures/subacuatica.png");
    oxBar = LoadTexture("textures/bluebar.png");
    
}

/*** Liberación de recursos ***/
void Free( void )
{
    
    
    //Espada
    FreeModel( &modeloEspada );

    
    //HUD
    glDeleteTextures( 1, &mapTex );
    glDeleteTextures( 1, &dotTex );
    glDeleteTextures( 1, &UnderwaterTex );
    glDeleteTextures(1, &water);
    glDeleteTextures(1, &hudNull);
    glDeleteTextures(1, &oxBar);
    
    //Terreno
    FreeTerrain(&terrain);

    
    //Agua
   // FreeTerrain(&agua);

    
    //Skybox
    FreeSkybox(&skybox);
    
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
                case SDLK_F6:
                    CreateScreenshot( "Ejercicio" );
                    break;
                case SDLK_w:
                    cam.walk = GL_FALSE;
                    break;
                case SDLK_s:
                    cam.walkinv= GL_FALSE;
                    break;
                    
                case SDLK_d:
                    cam.strafe= GL_FALSE;
                    break;
                    
                case SDLK_a:
                    cam.strafeinv= GL_FALSE;
                    break;

                    
                    
                case SDLK_LSHIFT:
                    Brun = GL_FALSE;
                    break;
                case SDLK_SPACE:
                    jump = GL_FALSE;
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
                case SDLK_F6:
                    break;
                case SDLK_w:
                    cam.walk = GL_TRUE;
                    break;
                case SDLK_s:
                    cam.walkinv = GL_TRUE;
                    break;
                    
                case SDLK_d:
                    cam.strafe= GL_TRUE;
                    break;
                    
                case SDLK_a:
                    cam.strafeinv= GL_TRUE;
                    break;
                    

                    
                case SDLK_LSHIFT:
                    Brun = GL_TRUE;
                    
                    break;
                    
                case SDLK_SPACE:
                    jump = GL_TRUE;
                    break;
                    
                    
                default:
                    PrintError( "Unhandled KEYDOWN event", GL_TRUE );
            }
                break;
                
                /* Botones del mouse */
            case SDL_MOUSEBUTTONUP:
                switch( event.button.button )
            {
                case SDL_BUTTON_LEFT:
                    break;
                case SDL_BUTTON_RIGHT:
                    renderUnderwater = !renderUnderwater;
                    break;
                default:
                    PrintError( "Unhandled MOUSEBUTTONUP event", GL_TRUE );
            }
                break;
                
            case SDL_MOUSEBUTTONDOWN:
                switch( event.button.button )
            {
                case SDL_BUTTON_LEFT:
                    atk1 = GL_TRUE;
                    
                    break;
                default:
                    PrintError( "Unhandled MOUSEBUTTONDOWN event", GL_TRUE );
            }
                break;
                
                
                /* Movimiento del mouse */
            case SDL_MOUSEMOTION:
            {
                /* Capturo los movimientos */
                GLfloat xrel, yrel;
                xrel = (GLfloat)event.motion.xrel;
                yrel = (GLfloat)event.motion.yrel;
                
                if(xrel !=0 && xrel> -100 && xrel < 100)
                    Yaw(&cam,xrel*elapsed*sensibility,GL_TRUE);
                if(yrel !=0 && yrel> -100 && yrel < 100)
                    Pitch(&cam,yrel*elapsed*sensibility,GL_TRUE);
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
    
//    ACC = MulVector(ACC, 5);

    vTerrain = GetHeight( &terrain , cam.pos.x, cam.pos.z ) + altura;
  //  sprintf(fpstex, "%d fps", CalculateFPS(elapsed));
    
    /*Cámara*/
    
    
    // correr
    if (Brun && (cam.pos.y <= vTerrain || under)){
        //  MulVector(VEL, 1.5f);
        run = 3.5f;
    }
    else if (Brun == GL_FALSE && (cam.pos.y <= vTerrain || under)){
        run = 1.0f;
    }
    
    // confirma si esta debajo del agua
    if (cam.pos.y <= vWater)
        under = GL_TRUE;
    else
        under = GL_FALSE;
    
    
    
    //debajo del agua
    
    if (under){
        velocityAgua   = 0.3f;
        
        if (abs(VEL.x) <velocity * 0.3){
            //VEL.x = 10.0;
            ACC.x = 0.0f; //aqui agrego la corriente
        }
        else
            ACC.x = 0.0f;

    }
    else{
        velocityAgua   = 1.0f;
//        VEL.z = 150.0f;
        ACC.x = 0.0f;
        VEL.x = 0.0f;
    }
           
    
    /*
     //bool
(runState ? 0.5f : 1.0f)
     */
 
    if( cam.walk ){
        //VEL.z = velocity * run * velocityAgua;
        Walk( &cam,velocity * run * velocityAgua * elapsed, GL_TRUE);
    }
    if( cam.walkinv ){
        //VEL.z = -velocity * 0.5 * run  * velocityAgua;
        Walk( &cam, -velocity * 0.5 * run  * velocityAgua* elapsed, GL_TRUE);
    }
    if( cam.strafe ){
        //VEL.x = velocity * 0.8 * run * velocityAgua;
        Strafe( &cam, velocity * 0.8 * run * velocityAgua * elapsed, GL_TRUE);
    }    
    if( cam.strafeinv ){
       // VEL.x = -velocity * 0.8 * run * velocityAgua;
        Strafe( &cam, -velocity * 0.8 * run * velocityAgua * elapsed, GL_TRUE);
	}
    
    
/*
    if( cam.walk ){
        VEL.z = velocity * run * velocityAgua;
        Walk( &cam,VEL.z * elapsed, GL_TRUE);
    }else if (cam.pos.y <= vTerrain)
        VEL.z = 0.0f;
    
    if( cam.walkinv ){
        VEL.z = -velocity * run  * velocityAgua;
        Walk( &cam,VEL.z * elapsed, GL_TRUE);
    }else if (cam.pos.y <= vTerrain)
        VEL.z = 0.0f;
    
    if( cam.strafe ){
        VEL.x = velocity * run * velocityAgua;
        Strafe( &cam, VEL.x * elapsed, GL_TRUE);
    }else if (cam.pos.y <= vTerrain)
        VEL.x = 0.0f;
    
    if( cam.strafeinv ){
        VEL.x = -velocity * run * velocityAgua;
        Strafe( &cam, VEL.x * elapsed, GL_TRUE);
	}else if (cam.pos.y <= vTerrain)
        VEL.x = 0.0f;
    
  */
    
    //  LOCALIZACION EN MAPA
    dotX = mapWidth * (cam.pos.x / ( 64.0f * 50.0f));
	dotZ = mapHeight * (cam.pos.z / ( 64.0f * 50.0f));

	
    

    
    //fisicas
    VEL = SumVector ( VEL, MulVector ( ACC , elapsed ) );
    //POS = SumVector ( POS, MulVector ( VEL , elapsed ) );

    cam.pos.x = ( cam.pos.x + (VEL.x * elapsed ) );
    //cam.pos.y = ( cam.pos.y + (VEL.y * elapsed ) );
    // caidas y corriente
    
    
    if (under){
        cam.pos.y = cam.pos.y + (VEL.y * 0.03 * run * elapsed );
        cam.pos.x = cam.pos.x + (VEL.x * elapsed);
    }
    else{
        cam.pos.y = cam.pos.y + (VEL.y * velocityAgua * run * elapsed );
        //cam.pos.z = cam.pos.z + (VEL.z * elapsed);
    }
    
     
     
    if (jump && cam.pos.y<= vTerrain) {
        VEL.y = 40.0f;
        jump = GL_FALSE;
    }else if (jump && (cam.pos.y <= vWater + (altura * 0.1)) ){
        
            // VEL.x = -velocity * 0.8 * run * velocityAgua;
            Fly( &cam, velocity * 0.2 * run * elapsed, GL_TRUE);
            VEL.y = 0.0f;
        if (cam.pos.y > vWater)
            cam.pos.y = vWater + (altura * 0.1f);
        
        /*
        if (cam.pos.y  > vWater -  altura){
            VEL.y = 0.0f;
            cam.pos.y = vWater;

}*/

    }
//OXIGENO
    
    if ((( (Brun && (cam.walk || cam.walkinv || cam.strafe || cam.strafeinv)))  || under) && !oxIn)
        ox -= elapsed;
    else
        ox += 0.2 * elapsed;
    
    if ( ox < 0.0f || oxIn)
        oxIn = GL_FALSE;
    
    if (ox < 0.0f) {
        oxIn = GL_FALSE;
        ox = 0.0f;
    }
    if (ox > oxlevel){
        oxIn = GL_FALSE;
        ox = oxlevel;
    }
    
    
    
//ATAQUES CON LA ESPADA...
    if (atk1){
        if (angX > -70) {
        angX -= 360.0f* elapsed;
        }
        
        if (angZ < 90) {
            angZ += 360.0f* elapsed;
        }else{
        
        angX=0.0f;
        angZ = 0.0f;
            atk1= GL_FALSE;
            
        }
        /*
        if (angZ > 500) {
            angZ -= 360.0f* elapsed;
        }*/



    }
    
    //mueve textura
    movWater +=  0.4f * (elapsed);

    
    
    //no sobrepasa el limite del suelo
    if (cam.pos.y <= vTerrain)
        cam.pos.y = vTerrain;
    
    
    
	
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
    gluPerspective( 50.0f/zoom,                           // Ángulo de visión vertical
                   (GLfloat)WIDTH / (GLfloat)HEIGHT, // Aspecto
                   1.0f,                             // Plano cercano
                   2000.0f );                        // Plano lejano
    
    /* Cámara */
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    
    /* Espada */
    glPushMatrix();
    glMultTransposeMatrixf(volumesEspada.matrix);
    
  //  glTranslatef(cam.pos.x, cam.pos.y + 5, cam.pos.z + 10);
    glRotatef(angX, 1.0f, 0.0f, 0.0f);
    glRotatef(angY, 0.0f, 1.0f, 0.0f);
    glRotatef(angZ, 0.0f, 0.0f, 1.0f);
    
    glCallList(modeloEspada.modelList);
    glPopMatrix();
    /*________*/
    
    
    
   
    
        
    
    
       
    
    
	VECTOR dir = SumVector(cam.pos, cam.look);
    
    gluLookAt(  cam.pos.x, cam.pos.y,  cam.pos.z,       // Posición
                dir.x, dir.y, dir.z,                    // Dirección
                0.0f, 1.0f, 0.0f );                     // Arriba
    
    
    
    
    /*Luz*/
    glPushMatrix();
    SetDirLight(GL_LIGHT0, &dirLight);
    glEnable (GL_LIGHT0);
    glPopMatrix();
    
    
    /*Terreno*/
    glPushMatrix();
    SetMaterial( &terrain.material );
    glBindTexture(GL_TEXTURE_2D, terrain.textureID);
    glCallList( terrain.terrainList );
    glPopMatrix();
    
    
    

    /*agua*/
      /*
    glPushMatrix();
    glEnable(GL_BLEND);
    SetMaterial( &agua.material );
    glBindTexture(GL_TEXTURE_2D, agua.textureID);
    glCallList( agua.terrainList );
    glDisable(GL_BLEND);
    glPopMatrix();
    
    
*/
    
    
    
    
    
    /*texto fps*/
 //   RenderText(fpstex, arialfont, 0, 0, &fontColor, GL_FALSE);
    
    
    /*SkyBox*/
    glPushMatrix();
    RenderSkybox(&skybox, &skyboxBox, &skyboxColor);
    glPopMatrix();
    
    

    

    
  
  
    
    
    
    //**texturitas!!!**//
    
    
    /* subacuatica*/
    
    
    glPushMatrix();
    

    //glLoadIdentity();
    glEnable(GL_BLEND);
    SetMaterial(&aguaMtrl);
    
    glBindTexture(GL_TEXTURE_2D, UnderwaterTex);

    glTranslatef(cam.pos.x, vWater, cam.pos.z);
    glScalef(1000.0f, 1000.0f, 1000.0f);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    //  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER    , );

    
    glBegin(GL_QUADS);
    //Cara posterior
    
    
    
    glNormal3f( 0.0f,  1.0f, 0.0f);

    
    glTexCoord2f    ( 0.0f,  1.0f + movWater);
    glVertex3f      (-1.0f,  0.0f, -1.0f);
    glTexCoord2f    ( 0.0f,  0.0f + movWater);
    glVertex3f      ( 1.0f,  0.0f, -1.0f);
    glTexCoord2f    ( 1.0f,  0.0f + movWater);
    glVertex3f      ( 1.0f,  0.0f,  1.0f);
    glTexCoord2f    ( 1.0f,  1.0f + movWater);
    glVertex3f      (-1.0f,  0.0f,  1.0f);

    glEnd();
 
    glDisable(GL_BLEND);

    
    glPopMatrix();
    
    //_______________//
    //Transparencia


    
    
    
    
    /*HUD*/
    Begin2D(WIDTH, HEIGHT);
    


    
    if ( !renderUnderwater){
            }
    
    if (under){
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        Render2DTexture( UnderwaterTex, 0, 0, WIDTH, HEIGHT );
    }
    
    
    //RENDER DEL MAPA Y EL DOT
    glPushMatrix();
    glTranslatef(WIDTH*0.79 , HEIGHT * 0.79, 0);
    Render2DTexture( mapTex, 0, 0, mapWidth, mapHeight );
    Render2DTexture( dotTex, dotX, dotZ, dotX+10, dotZ+10  );

    glPopMatrix();
    Render2DTexture(hudNull, 0, 0, WIDTH, HEIGHT);
    Render2DTexture(oxBar, 0, 0, ox * 10.0f, 50.0f);
    End2D();


    
    
    
    
    
    
    
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
