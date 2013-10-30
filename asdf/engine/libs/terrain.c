/***************************/
/**      -----------      **/
/**       terrain.c       **/
/**      -----------      **/
/**  Creación y rende-    **/
/**  rización de terrenos **/
/***************************/

//---   Estructuras   ---//

/*** Estructura de dato: TERRAIN ***/
typedef struct terrain
{
    unsigned char*     heightMap;
    GLuint             vertsPerRow;
    GLuint             vertsPerCol;
    GLfloat            cellSpacing;
    GLuint*            indexBuffer;
    NORMAL_TEX_VERTEX* vertexBuffer;
    MATERIAL           material;
    GLuint             textureID;
    GLuint             terrainList;
}TERRAIN;

/*_______*/


//--- Funciones ---//

/*** Función: Inicializa un terreno ***/
void InitTerrain( TERRAIN*  terrain,
		  char*     terrainFile,
		  char*     terrainTexture,
		  GLboolean repeatTex,
		  MATERIAL* terrainMtrl,
		  GLuint    vertsPerRow,
		  GLuint    vertsPerCol,
		  GLuint    cellSpacing,
		  GLfloat   heightScale )
{
    /* Características */
    terrain->vertsPerRow = vertsPerRow;
    terrain->vertsPerCol = vertsPerCol;
    terrain->cellSpacing = cellSpacing;
    terrain->material    = *terrainMtrl;

    /* Memoria para el heightMap */
    terrain->heightMap = (unsigned char*)calloc( vertsPerRow * vertsPerCol,
						 sizeof(unsigned char) );

    /* Leo el heightMap */
    FILE* file = fopen( terrainFile, "r" );
    fread( terrain->heightMap, sizeof(unsigned char), vertsPerRow * vertsPerCol, file );
    fclose( file );

    /* Escalo el heightmap */
    unsigned int h;
    for( h = 0; h < vertsPerRow * vertsPerCol; h++ )
	terrain->heightMap[h] *= heightScale;


    /* Calculo los vértices */
    terrain->vertexBuffer = (NORMAL_TEX_VERTEX*)calloc( vertsPerRow * vertsPerCol,
							sizeof(NORMAL_TEX_VERTEX) );
    GLfloat uTexDelta = 1.0f / (vertsPerRow - 1);
    GLfloat vTexDelta = 1.0f / (vertsPerCol - 1);
    unsigned int i, j;

    for( i = 0; i < vertsPerCol; i++ )
    {
	for( j = 0; j < vertsPerRow; j++ )
	{
	    NORMAL_TEX_VERTEX v;
	    v.p.x = j * cellSpacing;
	    v.p.y = terrain->heightMap[ (i * vertsPerRow) + j ];
	    v.p.z = i * cellSpacing;
	    v.n.x = 0.0f;
	    v.n.y = 0.0f;
	    v.n.z = 0.0f;
	    v.t.u = j * ( repeatTex? 1 : uTexDelta );
	    v.t.v = i * ( repeatTex? 1 : vTexDelta );
	    terrain->vertexBuffer[ (i * vertsPerRow) + j ] = v;
	}
    }

    /* Calculo las normales */
    for( i = 0; i < vertsPerCol - 1; i++ )
    {
	for( j = 0; j < vertsPerRow - 1; j++ )
	{
	    /*
	      A---B
	      |  /|
	      | / |
	      |/  |
	      C---D
	    */
	    VECTOR n;
	    POINT A, B, C;

	    A = terrain->vertexBuffer[ ((i + 0) * vertsPerRow) + (j + 0) ].p;
	    B = terrain->vertexBuffer[ ((i + 0) * vertsPerRow) + (j + 1) ].p;
	    C = terrain->vertexBuffer[ ((i + 1) * vertsPerRow) + (j + 0) ].p;

	    VECTOR v1 = {C.x - A.x, C.y - A.y, C.z - A.z}; //C-A
	    VECTOR v2 = {B.x - A.x, B.y - A.y, B.z - A.z}; //B-A
	    
	    n = CrossProduct( v1, v2 ); 
	    n = NormalizeVector( n );
	    terrain->vertexBuffer[ (i * vertsPerRow) + j ].n = n;
	}
    }

    // Última fila
    for( j = 0; j < vertsPerRow - 1; j++ )
    {
	/*
	  A---B
	  |  /|
	  | / |
	  |/  |
	  C---D
	*/
	VECTOR n;
	POINT C, A, B;
	
	C = terrain->vertexBuffer[ (vertsPerCol - 1) * vertsPerRow + j + 0 ].p;
	A = terrain->vertexBuffer[ (vertsPerCol - 2) * vertsPerRow + j + 0 ].p;
	B = terrain->vertexBuffer[ (vertsPerCol - 2) * vertsPerRow + j + 1 ].p;
	
	VECTOR v1 = {B.x - C.x, B.y - C.y, B.z - C.z}; //B-C
	VECTOR v2 = {A.x - C.x, A.y - C.y, A.z - C.z}; //A-C
	
	n = CrossProduct( v1, v2 ); 
	n = NormalizeVector( n );
	terrain->vertexBuffer[ (vertsPerCol - 1) * vertsPerRow + j ].n = n;
    }

    // Última columna
    for( i = 0; i < vertsPerCol - 1; i++ )
    {
	/*
	  A---B
	  |  /|
	  | / |
	  |/  |
	  C---D
	*/
	VECTOR n;
	POINT B, D, C;
	
	B = terrain->vertexBuffer[ (vertsPerRow * (i + 0)) + (vertsPerCol - 1) ].p;
	D = terrain->vertexBuffer[ (vertsPerRow * (i + 1)) + (vertsPerCol - 1) ].p;
	C = terrain->vertexBuffer[ (vertsPerRow * (i + 1)) + (vertsPerCol - 2) ].p;
	
	VECTOR v1 = {C.x - B.x, C.y - B.y, C.z - B.z}; //C-B
	VECTOR v2 = {D.x - B.x, D.y - B.y, D.z - B.z}; //D-B
	
	n = CrossProduct( v1, v2 ); 
	n = NormalizeVector( n );
	terrain->vertexBuffer[ (vertsPerRow * (i + 0)) + (vertsPerCol - 1) ].n = n;
    }

    // Normal: extrema derecha inferior
    {
	/*
	  A---B
	  |  /|
	  | / |
	  |/  |
	  C---D
	*/
	VECTOR n;
	POINT D, B, C;
	
	D = terrain->vertexBuffer[ (vertsPerCol - 1) * vertsPerRow + vertsPerRow - 1 ].p;
	B = terrain->vertexBuffer[ (vertsPerCol - 2) * vertsPerRow + vertsPerRow - 1 ].p;
	C = terrain->vertexBuffer[ (vertsPerCol - 1) * vertsPerRow + vertsPerRow - 2 ].p;
	
	VECTOR v1 = {B.x - D.x, B.y - D.y, B.z - D.z}; //B-D
	VECTOR v2 = {C.x - D.x, C.y - D.y, C.z - D.z}; //C-D
	
	n = CrossProduct( v1, v2 ); 
	n = NormalizeVector( n );
	terrain->vertexBuffer[ (vertsPerCol - 1) * vertsPerRow + vertsPerRow - 1 ].n = n;
    }

    /* Índices */
    terrain->indexBuffer = (GLuint*)calloc( (vertsPerRow - 1) * (vertsPerCol - 1) * 2 * 3,
					    sizeof( GLuint ) );

    for( i = 0; i < vertsPerCol - 1; i++ )
    {
	for( j = 0; j < vertsPerRow - 1; j++ )
	{
	    /*
	      A---B
	      |  /|
	      | / |
	      |/  |
	      C---D
	    */
	    unsigned int base = (i * (vertsPerRow - 1) + j) * 2 * 3;
	    // Triángulo 1(ABC)
	    terrain->indexBuffer[ base + 0 ] = ((i + 0) * vertsPerRow + j) + 0;
	    terrain->indexBuffer[ base + 1 ] = ((i + 0) * vertsPerRow + j) + 1;
	    terrain->indexBuffer[ base + 2 ] = ((i + 1) * vertsPerRow + j) + 0;
	    // Triángulo 2(BDC)
	    terrain->indexBuffer[ base + 3 ] = ((i + 0) * vertsPerRow + j) + 1;
	    terrain->indexBuffer[ base + 4 ] = ((i + 1) * vertsPerRow + j) + 1;
	    terrain->indexBuffer[ base + 5 ] = ((i + 1) * vertsPerRow + j) + 0;
	}
    }

    /* Coloreado de terreno */
    if( terrainTexture != NULL )
	// Cargo la textura
	terrain->textureID = LoadTexture( terrainTexture );
    else
    {
	// Genero la textura
	glGenTextures( 1, &(terrain->textureID) );

	GLubyte pixelData[ vertsPerRow * vertsPerCol * sizeof(GLubyte) * 3 ];
	for( i = 0; i < vertsPerCol; i++ )
	{
	    for( j = 0; j < vertsPerRow; j++ )
	    {
		GLfloat height = terrain->heightMap[ (i * vertsPerRow) + j ];
		COLOR c;
		if( height < 42.5f )
		{
		    COLOR t = BEACH_SAND;
		    c = t;
		}
		else if( height < 85.0f )
		{
		    COLOR t = LIGHT_YELLOW_GREEN;
		    c = t;
		}
		else if( height < 127.5f )
		{
		    COLOR t = PUREGREEN;
		    c = t;
		}
		else if( height < 170.0f )
		{
		    COLOR t = DARK_YELLOW_GREEN;
		    c = t;
		}
		else if( height < 212.5f )
		{
		    COLOR t = DARKBROWN;
		    c = t;
		}
		else
		{
		    COLOR t = WHITE;
		    c = t;
		}
		// Pongo el color en entero
		pixelData[ (i * vertsPerRow + j) * 3 + 0 ] = (GLubyte)(c.r * 255.0f);
		pixelData[ (i * vertsPerRow + j) * 3 + 1 ] = (GLubyte)(c.g * 255.0f);
		pixelData[ (i * vertsPerRow + j) * 3 + 2 ] = (GLubyte)(c.b * 255.0f);
	    }
	}

	// Cargo la textura en memoria
	glBindTexture( GL_TEXTURE_2D, terrain->textureID );
	// Se deben generar mipmaps
	glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );
	// Clamp
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );


	glTexImage2D( GL_TEXTURE_2D,
		      0,
		      GL_RGB,
		      vertsPerRow,
		      vertsPerCol,
		      0,
		      GL_RGB,
		      GL_UNSIGNED_BYTE,
		      pixelData );
    }

    /* Compilación de la lista de dibujo */
    // Creo la lista
    terrain->terrainList = glGenLists( 1 );
    
    // Habilito el uso de arreglos de vértices, normales y texturas
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );

    // Buffers
    // Pongo el buffer de vértices en memoria de video
    glVertexPointer( 3, GL_FLOAT, sizeof(NORMAL_TEX_VERTEX), &terrain->vertexBuffer[0].p );
    // Pongo el buffer de normales en memoria de video
    glNormalPointer( GL_FLOAT, sizeof(NORMAL_TEX_VERTEX), &terrain->vertexBuffer[0].n );
    // Pongo el buffer de coordenadas de textura en memoria de video
    glTexCoordPointer( 2, GL_FLOAT, sizeof(NORMAL_TEX_VERTEX), &terrain->vertexBuffer[0].t );

    /* Inicia los comandos de la lista */
    glNewList( terrain->terrainList, GL_COMPILE );
    glPushAttrib( GL_ENABLE_BIT | GL_TEXTURE_BIT );

    // Textura
    glEnable( GL_TEXTURE_2D );
    if( repeatTex )
    {
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    }
    else
    {
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    }

    // Renderización
    glDrawElements( GL_TRIANGLES, (terrain->vertsPerRow - 1) * (terrain->vertsPerCol - 1) * 2 * 3,
		    GL_UNSIGNED_INT, terrain->indexBuffer );

    /* Terminan los comandos de la lista */
    glPopAttrib();
    glEndList();
}

/*** Función: Libera los recursos de un terreno ***/
void FreeTerrain( TERRAIN* terrain )
{
    free( terrain->heightMap );
    free( terrain->vertexBuffer );
    free( terrain->indexBuffer );
    glDeleteTextures( 1, &terrain->textureID );
    glDeleteLists( terrain->terrainList, 1 );
}

/*** Función: Obtener altura con una coordenada(XZ) ***/
GLfloat GetHeight( TERRAIN* terrain, GLfloat x, GLfloat z )
{
    x /= terrain->cellSpacing;
    z /= terrain->cellSpacing;

    /* Coordenadas del cuadro */
    GLuint row = floorf(x);
    GLuint col = floorf(z);

    /*
      A---B
      |1 /|
      | / |
      |/ 2|
      C---D
    */
    GLfloat A = terrain->heightMap[ ((col + 0) * terrain->vertsPerRow) + (row + 0) ];
    GLfloat B = terrain->heightMap[ ((col + 0) * terrain->vertsPerRow) + (row + 1) ];
    GLfloat C = terrain->heightMap[ ((col + 1) * terrain->vertsPerRow) + (row + 0) ];
    GLfloat D = terrain->heightMap[ ((col + 1) * terrain->vertsPerRow) + (row + 1) ];
    
    GLfloat height = 0.0f;
    GLfloat dx     = x - row;
    GLfloat dz     = z - col;

    /* Triángulo 1 */
    if( dx + dz < 1.0f )
    {
	GLfloat Hz = (C - A)*dz;
	GLfloat Hx = (B - A)*dx;
	height     = A + Hx + Hz;
    }
    /* Triángulo 2 */
    else
    {
	GLfloat Hz = (B - D)*(1.0f - dz);
	GLfloat Hx = (C - D)*(1.0f - dx);
	height     = D + Hx + Hz;
    }
    
    return height;
}
