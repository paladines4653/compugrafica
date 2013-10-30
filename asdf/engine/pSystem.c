/******************************/
/**        -----------       **/
/**         pSystem.c        **/
/**        -----------       **/
/** Creación y renderización **/
/** de sistema de partículas **/
/******************************/

//---   Estructuras   ---//

/*** Estructura de dato: PARTICLE ***/
typedef struct particle
{
    VECTOR    pos;    // Posición
    VECTOR    vel;    // Velocidad
    VECTOR    acc;    // Aceleración
    COLOR     color;  // Color
    GLfloat   life;   // Tiempo de vida
    GLfloat   age;    // Edad(tiempo)
    GLboolean active; // Estado: viva?
}PARTICLE;

/*** Estructura de dato: PSYSTEM ***/
typedef struct pSystem
{
    GLuint    numParticles; // Número de partículas
    GLuint    batch;        // Tamaño de bloques de partículas
    GLuint    texture;      // Textura de las partículas
    PARTICLE* particles;    // Arreglo de partículas
    void*     resetFunc;    // Función que reinicia partículas
    void*     updateFunc;   // Función que actualiza partículas
}PSYSTEM;

typedef void ResetFunc( PSYSTEM*, PARTICLE* );
typedef void UpdateFunc( PSYSTEM*, PARTICLE*, float );

/*_______*/


//--- Funciones ---//

/*** Función: Inicializa un sistema de partículas ***/
void InitPSystem( PSYSTEM* pSystem,
		  GLuint   numParticles,
		  GLuint   batch,
		  char*    texture,
		  void     resetFunc( PSYSTEM*, PARTICLE* ),
		  void     updateFunc( PSYSTEM*, PARTICLE*, float ) )
{
    // Guardo los datos del sistema de partículas
    pSystem->numParticles = numParticles;
    pSystem->batch        = batch;
    pSystem->texture      = LoadTexture( texture );
    pSystem->resetFunc    = resetFunc;
    pSystem->updateFunc   = updateFunc;
    
    // Reservo memoria para las partículas
    pSystem->particles = calloc( numParticles, sizeof(PARTICLE) );
}

/*** Función: Actualiza las características de las partículas ***/
void UpdatePSystem( PSYSTEM* pSystem, float elapsed )
{
    // Actualizo todas las partículas
    unsigned int i;
    for( i = 0; i < pSystem->numParticles; i++ )
	((UpdateFunc*)pSystem->updateFunc)( pSystem, &(pSystem->particles[i]), elapsed );
}

/*** Función: Resetea las características de las partículas ***/
void ResetPSystem( PSYSTEM* pSystem )
{
    // Reseteo todas las partículas
    unsigned int i;
    for( i = 0; i < pSystem->numParticles; i++ )
	((ResetFunc*)pSystem->resetFunc)( pSystem, &(pSystem->particles[i]) );
}

/*** Función: Renderiza el sistema de partículas ***/
void RenderPSystem( PSYSTEM* pSystem )
{
    // Características de los puntos
    GLfloat attenuation[3] = {0.0f, 0.0f, 0.01f};
    GLfloat pointsize[2];
    glGetFloatv( GL_SMOOTH_POINT_SIZE_RANGE, pointsize );
    glPointSize( pointsize[1] );
    glPointParameterfv( GL_POINT_DISTANCE_ATTENUATION, attenuation );
    glPointParameterf( GL_POINT_SIZE_MIN, pointsize[0] );
    //glPointParameterf( GL_POINT_SIZE_MAX, pointsize[1] );
    glPointParameterf( GL_POINT_FADE_THRESHOLD_SIZE, pointsize[0] );
    glTexEnvi( GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE );

    // Pre-renderizado
    glPushClientAttrib( GL_CLIENT_VERTEX_ARRAY_BIT );
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );

    glPushAttrib( GL_ENABLE_BIT );
    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    glBindTexture( GL_TEXTURE_2D, pSystem->texture );
    
    // Renderizado
    VECTOR vBuffer[pSystem->batch];
    COLOR  cBuffer[pSystem->batch];
    unsigned int i, j = 0;
    // Renderizo por bloques(batch)
    for( i = 0; i <= pSystem->numParticles; i++ )
    {
	// Verifico que el bloque esté completo
	if( j == pSystem->batch || i == pSystem->numParticles )
	{
	    // Dibujo las partículas
	    glVertexPointer( 3, GL_FLOAT, 0, vBuffer );
	    glColorPointer( 4, GL_FLOAT, 0, cBuffer );
	    glEnable( GL_POINT_SPRITE );
	    glDrawArrays( GL_POINTS, 0, j );
	    glDisable( GL_POINT_SPRITE );
	    // Reinicio el Batch
	    j = 0; 
	    // La partícula actual debe ser procesada
	    if( i != pSystem->numParticles )
		i--;
	    continue;
	}

	// Verifico que la partícula esté activa
	if( pSystem->particles[i].active )
	{
	    // La copio a los bufferes
	    vBuffer[j] = pSystem->particles[i].pos;
	    cBuffer[j] = pSystem->particles[i].color;
	    j++;
	}
    }
    
    // Post-renderizado
    glPopClientAttrib();
    glPopAttrib();
}

/*** Función: Libera los recursos asocidados un sistema de partículas ***/
void FreePSystem( PSYSTEM* pSystem )
{
    glDeleteTextures( 1, &pSystem->texture );
    free( pSystem->particles );
}
