/****************************/
/**       ----------       **/
/**        sprite.c        **/
/**       ----------       **/
/**  Interfaz para rende-  **/
/**  rizar elementos HUD   **/
/**  en OpenGL             **/
/****************************/


//---   Funciones   ---//

/*** Función: Inicia el modo 2D ***/
// width, height: Dimensión de la pantalla
void Begin2D( GLuint width, GLuint height )
{
    /* Proyección */
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D( 0, width, height, 0 );

    /* Traslación */
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    /* Guardo los atributos(luz) */
    glPushAttrib( GL_ENABLE_BIT | GL_POLYGON_BIT );
    /* Deshabilito la luz y habilito transparencia */
    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    glFrontFace( GL_CW );
}

/*** Función: Dibuja una textura en modo HUD ***/
void Render2DTexture( GLuint tex, GLuint xi, GLuint yi, GLuint xf, GLuint yf )
// tex: Texture ID
// xi, yi: Ubicación esquina izquierda superior
// xf, yf: Ubicación esquina derecha inferior
{
    /* Ato la texutura */
    glBindTexture( GL_TEXTURE_2D, tex );
    
    /* Dibujo en un cuadro */
    glBegin( GL_QUADS );
    glTexCoord2f( 0.0f, 0.0f );
    glVertex2i( xi, yi );
    glTexCoord2f( 1.0f, 0.0f );
    glVertex2i( xf, yi );
    glTexCoord2f( 1.0f, 1.0f );
    glVertex2i( xf, yf );
    glTexCoord2f( 0.0f, 1.0f );
    glVertex2i( xi, yf );
    glEnd();
}

/*** Función: Finaliza el modo 2D ***/
void End2D( void )
{
    /* Proyección */
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();

    /* Traslación */
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    
    /* Restauro los atributos */
    glPopAttrib();
}
