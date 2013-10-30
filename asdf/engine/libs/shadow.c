/****************************/
/**       ----------       **/
/**        shadow.c        **/
/**       ----------       **/
/**  Interfaz para hacer   **/
/**  Shadow Mapping        **/
/****************************/


GLint currentViewPort[4]; // Actual viewport

//--- Estructuras ---//

/*** Estructura de dato: SHADOWMAP ***/
typedef struct shadowmap
{
    GLuint  tex;
    GLuint  size;
    GLfloat texMatrix[16];
} SHADOWMAP;

/*________*/

//--- Funciones ---//

/*** Función: Crea e inicializa una estructura SHADOWMAP ***/
// mapSize = tamaño del shadow map, +grande -> +calidad
void CreateShadowMap( SHADOWMAP* sm, GLuint mapSize )
{
    /* Creo y guardo la textura con sus atributos */
    sm->size = mapSize;
    glGenTextures( 1, &sm->tex );
    glBindTexture( GL_TEXTURE_2D, sm->tex );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, sm->size, sm->size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL );
    glBindTexture( GL_TEXTURE_2D, 0 );
}

/*** Función: Crea el shadow map respecto a la luz ***/
// isPoint    = TRUE:point light  -  FALSE:dir light
// lightPos   = posición de la luz
// lightDir   = dirección de la luz
// lightDepth = Que tan lejos o profunda llega la luz
// lightRange = Que tan abierta es la proyeción de la luz
void BeginShadowMap( SHADOWMAP* sm, GLboolean isPoint, GLfloat* lightPos, GLfloat* lightDir, GLfloat lightDepth, GLfloat lightRange )
{
    /* Apago las luces */
    glPushAttrib( GL_LIGHTING_BIT );
    glDisable( GL_LIGHTING );
    
    /* Look */
    GLfloat look[3];
    look[0] = lightPos[0] + lightDir[0];
    look[1] = lightPos[1] + lightDir[1];
    look[2] = lightPos[2] + lightDir[2];

    /* Matrices de la luz */

    // Proyección
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    if( isPoint)
	gluPerspective( 2.0f * ( atan( 0.5f * lightRange / lightDepth ) * 180.0f * M_1_PI ),
			1.0f, 1.0f, lightDepth );
    else
	glOrtho( -lightRange/2.0f, lightRange/2.0f,
		 -lightRange/2.0f, lightRange/2.0f,
		 0.0f, lightDepth );

    // Cámara
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    gluLookAt( lightPos[0], lightPos[1], lightPos[2],
	       look[0], look[1], look[2],
	       0.0f, 1.0f, 0.0f );

    // Viewport
    glGetIntegerv( GL_VIEWPORT, currentViewPort );
    glViewport( 0, 0, sm->size, sm->size );

    /* Matrix para generar las coordenadas */
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( 0.5f, 0.5f, 0.5f );
    glScalef( 0.5f, 0.5f, 0.5f );
    if( isPoint )
	gluPerspective( 2.0f * ( atan( 0.5f * lightRange / lightDepth ) * 180.0f * M_1_PI ),
			1.0f, 1.0f, lightDepth );
    else
	glOrtho( -lightRange/2.0f, lightRange/2.0f,
		 -lightRange/2.0f, lightRange/2.0f,
		 0.0f, lightDepth );
    gluLookAt( lightPos[0], lightPos[1], lightPos[2],
	       look[0], look[1], look[2],
	       0.0f, 1.0f, 0.0f );
    glGetFloatv( GL_TRANSPOSE_MODELVIEW_MATRIX, sm->texMatrix );
    glPopMatrix();
    
    /* Caras traseras, sin cambios en el frame buffer */
    glCullFace( GL_FRONT );
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

/*** Función: Termina el shadow map ***/
// Entre 'BeginShadowMap' y 'EndShadowMap' se renderiza
// la escena que se necesita calcular sus sombras
void EndShadowMap( SHADOWMAP* sm )
{
    /* Copio el shadow map */
    glBindTexture( GL_TEXTURE_2D, sm->tex );
    glCopyTexSubImage2D( GL_TEXTURE_2D, 0,     // Level
			 0, 0,                 // Offset
			 0, 0,                 // Corner
			 sm->size, sm->size ); // Size
    glBindTexture( GL_TEXTURE_2D, 0 );

    /* Reestablezco los settings */
    glClear( GL_DEPTH_BUFFER_BIT );
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    glCullFace( GL_BACK );

    /* Reestablezco matrices de la cámara*/
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    /* Viewport */
    glViewport( currentViewPort[0], currentViewPort[1],
		currentViewPort[2], currentViewPort[3] );

    /* Reestablezco los valores de luz */
    glPopAttrib( );
}

/*** Función: Empieza la renderización de sombras ***/
void BeginShadowRender( SHADOWMAP* sm )
{
    /* Activo la textura y sus características */
    glPushAttrib( GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT );
    glBindTexture( GL_TEXTURE_2D, sm->tex );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
    glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY );

    /* Cuando se aplique la texura, genero las coordenadas de textura
       con la matriz que convierten del espacio de la luz al espacio
       de la cámara */
    glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR );
    glTexGenfv( GL_S, GL_EYE_PLANE, &sm->texMatrix[0] );
    glEnable( GL_TEXTURE_GEN_S );
    
    glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR );
    glTexGenfv( GL_T, GL_EYE_PLANE, &sm->texMatrix[4] );
    glEnable( GL_TEXTURE_GEN_T );

    glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR );
    glTexGenfv( GL_R, GL_EYE_PLANE, &sm->texMatrix[8] );
    glEnable( GL_TEXTURE_GEN_R );

    glTexGeni( GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR );
    glTexGenfv( GL_Q, GL_EYE_PLANE, &sm->texMatrix[12] );
    glEnable( GL_TEXTURE_GEN_Q );

    glEnable( GL_TEXTURE );
    glAlphaFunc( GL_EQUAL, 1.0f );
    glEnable( GL_ALPHA_TEST );
}

/*** Función: Termina la renderización de sombras ***/
// Entre 'BeginShadowRender' y 'EndShadowRender' se renderiza
// la escena que se necesita dibujar sus sombras
void EndShadowRender( void )
{
    /* Reestrablezco atributos */
    glPopAttrib();
}



/*** Matriz de generación de coordenadas de texturas ***/

// La matriz toma cada vértice en la escena y genera una coordenada de textura
// Por lo tanto debe transformar los vértices a su correspondiente en el shadow map
// El shadow map no puede utilizar filtros ya que la sombra se puede distorcionar

////***   Mt = Tr x Sc x Pl x Vl x Vc-¹   ***////

// Vc-¹ = Inversa del ojo de la camara (opengl la multiplica al final automáticamente cuando se especifica GL_EYE_LINEAR)
// Vl   = Ojo de la luz
// Pl   = Perspectiva de la luz
// Sc   = Escalamiento a la mita(1/2)
// Tr   = Traslación por 0.5 en cada sentido

///////////////////////////////////////////////////////////////////////////////////////
// Se aplica de derecha a izquerda
// 1) Transforma los vértices del 'eye space' de la cámara al 'world space'
// 2) Transforma los vértices del 'world space' al 'eye space' de la luz
// 3) Transforma los vértices del 'eye space' al 'window space' de la luz
// 4) Escala cada vértice a la mitad ya que la profundidad; z=[-1,1] ---> z=[-0.5,0.5]
// 5) Corre los vértices 0.5 para cuadrar la profundidad; z=[-0.5,0.5] ---> z=[0,1]
///////////////////////////////////////////////////////////////////////////////////////

// Cuando ya se generaran las coordenadas las correspondencias quedan así:
// x = S, y = T, z = R, w = Q
// Por lo tanto hay que comparar cada R(profundidad) con la profundidad producidad en el shadow map
// Si son menores en el shadow map entonces quiere decir que hay un objeto que lo ocluye(antes en la línea de visión de la luz)
// GL_TEXTURE_COMPARE_MODE = GL_COMPARE_R_TO_TEXTURE, GL_TEXTURE_COMPARE_FUNCTION = GL_LEQUAL
// R >  D ---> 0.0f    D = Valor en la textura
// R <= D ---> 1.0f    (LEQUAL)
// El resultado de lo anterior debe ser adjudicado a la intensidad del color(transparencia y luminosidad al mismo tiempo)
// GL_DEPTH_TEXTURE_MODE = GL_INTENSITY
// Con ayuda del canal alfa, solo proceso los pixeles que no estén sombreados
// glAlphaFunc( GL_EQUAL, 1.0f )
