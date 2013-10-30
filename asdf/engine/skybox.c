/***************************/
/**      -----------      **/
/**        skybox.c       **/
/**      -----------      **/
/**  Creación y rende-    **/
/**  rización de skyboxes **/
/***************************/

//---   Estructuras   ---//

/*** Estructura de dato: SKYBOX ***/
typedef struct skybox
{
    GLuint skyTex;     // Textura del skybox
    GLuint skyboxList; // Lista de ejecución del skybox
} SKYBOX;

/*_________*/


//--- Funciones ---//

/*** Función: Inicializa un skybox ***/
void InitSkybox( SKYBOX* skybox, char* skyTexture )
{    
    /* Buffer de vértices y normales */
    VECTOR vertex_buffer [] = {
	/* Cara Frontal */
	{ 0.0f, 0.0f, 0.0f}, { 0.0f, 1.0f, 0.0f}, { 1.0f, 1.0f, 0.0f}, { 1.0f, 0.0f, 0.0f},
	/* Cara Trasera */
	{ 1.0f, 0.0f, 1.0f}, { 1.0f, 1.0f, 1.0f}, { 0.0f, 1.0f, 1.0f}, { 0.0f, 0.0f, 1.0f},
	/* Cara Superior */ 
	{ 0.0f, 1.0f, 0.0f}, { 0.0f, 1.0f, 1.0f}, { 1.0f, 1.0f, 1.0f}, { 1.0f, 1.0f, 0.0f},
	/* Cara Inferior */
	{ 0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, 0.0f}, { 1.0f, 0.0f, 0.0f}, { 1.0f, 0.0f, 1.0f},
	/* Cara Izquierda */
	{ 0.0f, 0.0f, 1.0f}, { 0.0f, 1.0f, 1.0f}, { 0.0f, 1.0f, 0.0f}, { 0.0f, 0.0f, 0.0f},
	/* Cara Derecha */
	{ 1.0f, 0.0f, 0.0f}, { 1.0f, 1.0f, 0.0f}, { 1.0f, 1.0f, 1.0f}, { 1.0f, 0.0f, 1.0f}
    };
    
    TEXCOORD texcoord_buffer [] = {
	/* Cara Frontal */
	{ 0.25f, 1.00f }, { 0.25f, 0.75f }, { 0.75f, 0.75f }, { 0.75f, 1.00f },
	/* Cara Trasera */
	{ 0.75f, 0.00f }, { 0.75f, 0.25f }, { 0.25f, 0.25f }, { 0.25f, 0.00f },
	/* Cara Superior */ 
	{ 0.25f, 0.75f }, { 0.25f, 0.25f }, { 0.75f, 0.25f }, { 0.75f, 0.75f },
	/* Cara Inferior */
	{ 0.00f, 0.00f }, { 0.00f, 0.00f }, { 0.00f, 0.00f }, { 0.00f, 0.00f },
	/* Cara Izquierda */
	{ 0.00f, 0.25f }, { 0.25f, 0.25f }, { 0.25f, 0.75f }, { 0.00f, 0.75f },
	/* Cara Derecha */
	{ 1.00f, 0.75f }, { 0.75f, 0.75f }, { 0.75f, 0.25f }, { 1.00f, 0.25f }
    };

    /* Buffer de índices */
    GLuint index_buffer [] = {
	/* Cara Frontal */
	0, 1, 2,
	0, 2, 3,
	/* Cara Trasera */
	4, 5, 6,
	4, 6, 7,
	/* Cara Superior */
	8, 9, 10,
	8, 10, 11,
	/* Cara Inferior */
	12, 13, 14,
	12, 14, 15,
	/* Cara Izquierda */
	16, 17, 18,
	16, 18, 19,
	/* Cara Derecha */
	20, 21, 22,
	20, 22, 23
    };

    // Cargo la texura
    skybox->skyTex = LoadTexture( skyTexture );

    // Habilito el uso de arreglos de vértices, normales y texturas
    glPushClientAttrib( GL_CLIENT_VERTEX_ARRAY_BIT | GL_ENABLE_BIT );
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );

    // Buffers
    // Pongo el buffer de vértices en memoria de video
    glVertexPointer( 3, GL_FLOAT, sizeof(VECTOR), vertex_buffer );
    // Pongo el buffer de coordenadas de textura en memoria de video
    glTexCoordPointer( 2, GL_FLOAT, sizeof(TEXCOORD), texcoord_buffer );

    // Creo la lista de ejecución
    skybox->skyboxList = glGenLists( 1 );
    glNewList( skybox->skyboxList, GL_COMPILE );

    // Renderización
    glBindTexture( GL_TEXTURE_2D, skybox->skyTex );
    glFrontFace( GL_CW );
    glDrawElements( GL_TRIANGLES,  sizeof(index_buffer) / sizeof(GLuint),
		    GL_UNSIGNED_INT, index_buffer );
    
    glEndList();
    glPopAttrib();
}

/*** Función: Libera un skybox ***/
void FreeSkybox( SKYBOX* skybox )
{
    glDeleteTextures( 1, &skybox->skyTex );
    glDeleteLists( skybox->skyboxList, 1 );
}

/*** Función: Renderiza un skybox ***/
void RenderSkybox( SKYBOX* skybox, BOX* box, COLOR* color )
{
    // Guardo atributos y cámara
    glPushAttrib( GL_ENABLE_BIT );

    // Deshabilito la iluminación
    glDisable( GL_LIGHTING );
    glColor4f( color->r, color->g, color->b, color->a );
    
    // Transformaciones
    glTranslatef( box->min.x, box->min.y, box->min.z  );
    glScalef( fabsf(box->max.x - box->min.x),
	      fabsf(box->max.y - box->min.y),
	      fabsf(box->max.z - box->min.z) );
    
    // Renderizo
    glCallList( skybox->skyboxList );
    
    // Restauro atributos
    glPopAttrib();
}
