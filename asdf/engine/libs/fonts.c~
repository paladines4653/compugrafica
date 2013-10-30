/******************************/
/**       -----------        **/
/**         fonts.c          **/
/**       -----------        **/
/**  Inicialización y ren-   **/
/**  derización de fuentes   **/
/**  en OpenGL               **/
/******************************/

//--- Definiciones ---//
typedef TTF_Font* FONT;    // Manipulación del tipo 'fuente'

//--- Variables ---//
SDL_Surface* textSurface;  // Surface del texto
GLuint       textTexture;  // Textura del texto
int          screenWidth;  // Ancho de la resolución
int          screenHeight; // Alto de la resolución
/*_______*/


//---   Funciones ---//

/*** Función: Inicializa el sistema de fuentes ***/
// size: tamaño de la letra
// width, height: Resolución
void InitFont( int width, int height  )
{
    // Guardo los datos necesitados
    screenWidth  = width;
    screenHeight = height;
    // Creo la textura
    glGenTextures( 1, &textTexture );
    // Inicializo la interfaz TTF
    TTF_Init();
    
}

/*** Función: Libera todos los recursos asociados con la fuente ***/
void FreeFont( void )
{
    glDeleteTextures( 1, &textTexture );
    TTF_Quit();
}

/*** Función: Carga una fuente de un archivo ***/
TTF_Font* OpenFont( char* name, int size )
{
    return TTF_OpenFont( name, size );
}

/*** Función: Cierra una fuente cargada previamente ***/
void CloseFont( TTF_Font* font )
{
    TTF_CloseFont( font );
}

/*** Función: Pongo un estilo para la letra ***/
// style: TTF_STYLE_NORMAL, TTF_STYLE_BOLD, TTF_STYLE_ITALIC, TTF_STYLE_UNDERLINE, TTF_STYLE_STRIKETHROUGH
void SetFontStyle( TTF_Font* font, int style )
{
    TTF_SetFontStyle( font, style );
}

/*** Función: Pixeles recomendados para una linea nueva ***/
int GetFontLineSkip( TTF_Font* font )
{
    return TTF_FontLineSkip( font );
}

/*** Función: Pixeles totales del tamaño horizontal del texto ***/
int GetTextWidth( TTF_Font* font, char* text )
{
    int width;
    TTF_SizeUTF8( font, text, &width, NULL );
    return width;
}

/*** Función: Dibuja el texto "text" ***/
// posX, posY: posición relativa o absoluta
// (  0,   0): Esquina izquierda superior
// (100, 100): Esquina derecha inferior
// color     : Color del texto
// relative  : TRUE = ubicación relativa, FALSE = ubicación absoluta
void RenderText( char*     text,
		 TTF_Font* font,
		 GLuint    posX,
		 GLuint    posY,
		 COLOR*    color,
		 GLboolean relative )
{
    /* Posición del texto */
    if( relative )
      {
	posX = (GLuint)(posX*screenWidth/100.0f);
	posY = (GLuint)(posY*screenHeight/100.0f);
      }
    /* Color del texto */
    SDL_Color fontColor = { (Uint8)color->r*255,
			    (Uint8)color->g*255, 
			    (Uint8)color->b*255 };
    /* Surface del texto */
    textSurface = TTF_RenderUTF8_Blended( font, text, fontColor );

    /* Guardo los atributos */
    glPushAttrib( GL_ENABLE_BIT );
    /* Deshabilito la luz y habilito transparencia */
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );

    /* Creo la textura del texto */
    glBindTexture( GL_TEXTURE_2D, textTexture );
    gluBuild2DMipmaps( GL_TEXTURE_2D, textSurface->format->BytesPerPixel,
		       textSurface->w, textSurface->h,
		       GL_BGRA, GL_UNSIGNED_BYTE, textSurface->pixels );

    /* Proyección y Traslación */
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D( 0, screenWidth, screenHeight, 0 );
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    
    /* Dibujo en un cuadro */
    glBegin( GL_QUADS );
    glTexCoord2i( 0, 0 );
    glVertex2i( posX, posY );
    glTexCoord2i( 1, 0 );
    glVertex2i( posX + textSurface->w, posY );
    glTexCoord2i( 1, 1 );
    glVertex2i( posX + textSurface->w, posY + textSurface->h );
    glTexCoord2i( 0, 1 );
    glVertex2i( posX, posY + textSurface->h );
    glEnd();

    /* Proyección y Traslación */
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
   
    /* Restauro los atributos */
    glPopAttrib();

    /* Libero la superficie */
    SDL_FreeSurface( textSurface );
}

/*_______*/
