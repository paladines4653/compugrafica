/******************************/
/**        ----------        **/
/**         camera.c         **/
/**        ----------        **/
/**  Creación y manipulación **/
/**  de una cámara virtual   **/
/******************************/

//---   Estructuras   ---//

/*** Estructura de dato: CAMERA ***/
typedef struct camera
{
    VECTOR    pos;   // Posición de la cámara
    VECTOR    right; // Vector "derecha"
    VECTOR    up;    // Vector "arriba"
    VECTOR    look;  // Vector "dirección"
    // Estados de movimiento
    GLboolean walk  , walkinv;
    GLboolean strafe, strafeinv;
    GLboolean fly   , flyinv;
    GLboolean pitch , pitchinv, maxpitch;
    GLboolean yaw   , yawinv;
    GLboolean roll  , rollinv, maxroll;
}CAMERA;

/*_______*/


//---   Funciones ---//

/*** Función: Movimiento lateral ***/
void Strafe( CAMERA* cam, GLfloat units, GLboolean lockY )
{
    VECTOR dir = MulVector( cam->right, units );
    if( lockY )
	dir.y  = 0.0f;
    cam->pos   = SumVector( cam->pos, dir );
}

/*** Función: Movimiento altitudinal ***/
void Fly( CAMERA* cam, GLfloat units, GLboolean upFly )
{
    if( upFly )
    {
	VECTOR dir = { 0.0f, 1.0f, 0.0f };
	dir        = MulVector( dir, units );
	cam->pos   = SumVector( cam->pos, dir );
    }
    else
    {
	VECTOR dir = MulVector( cam->up, units );
	cam->pos   = SumVector( cam->pos, dir );
    }
}

/*** Función: Movimiento frontal ***/
void Walk( CAMERA* cam, GLfloat units, GLboolean lockY )
{
    VECTOR dir = MulVector( cam->look, units );
    if( lockY )
	dir.y  = 0.0f;
    cam->pos   = SumVector( cam->pos, dir );
}

/*** Función: Rotación en el eje "derecha" ***/
void Pitch( CAMERA* cam, GLfloat units, GLboolean lockPitch )
{
    GLfloat m[16];

    glPushMatrix(); // Guardo la matriz
    
    if( lockPitch )
    {
	static GLfloat angle = 0.0f;
	if( angle + units > cam->maxpitch || angle + units < -(cam->maxpitch) )
	    units =  copysign( cam->maxpitch, units ) - angle;
	angle += units;
    }

    glLoadIdentity();
    glRotatef( units, cam->right.x, cam->right.y, cam->right.z );
    glGetFloatv( GL_MODELVIEW_MATRIX, m );
    cam->up   = TransformCoordFromMatrix( cam->up  , m );
    cam->look = TransformCoordFromMatrix( cam->look, m );

    glPopMatrix(); // Restauro la matriz
}

/*** Función: Rotación en el eje "arriba" ***/
void Yaw( CAMERA* cam, GLfloat units, GLboolean upYaw )
{
    GLfloat m[16];

    glPushMatrix(); // Guardo la matriz
    
    glLoadIdentity();
    if( upYaw )
	glRotatef( units, 0.0f, 1.0f, 0.0f );
    else
	glRotatef( units, cam->up.x, cam->up.y, cam->up.z );
    glGetFloatv( GL_MODELVIEW_MATRIX, m );
    cam->right = TransformCoordFromMatrix( cam->right, m );
    cam->look  = TransformCoordFromMatrix( cam->look , m );

    glPopMatrix(); // Restauro la matriz
}

/*** Función: Rotación en el eje "dirección" ***/
void Roll( CAMERA* cam, GLfloat units, GLboolean lockRoll )
{
    GLfloat m[16];

    glPushMatrix(); // Guardo la matriz

    if( lockRoll )
    {
	static GLfloat angle = 0.0f;
	if( angle + units > cam->maxroll || angle + units < -(cam->maxroll) )
	    units =  copysign( cam->maxroll, units ) - angle;
	angle += units;
    }
    
    glLoadIdentity();
    glRotatef( units, cam->look.x, cam->look.y, cam->look.z );
    glGetFloatv( GL_MODELVIEW_MATRIX, m );
    cam->right = TransformCoordFromMatrix( cam->right, m );
    cam->up    = TransformCoordFromMatrix( cam->up   , m );

    glPopMatrix(); // Restauro la matriz
}

/*_________*/
