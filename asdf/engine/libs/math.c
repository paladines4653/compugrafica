/*****************************/
/**       ----------        **/
/**         math.c          **/
/**       ----------        **/
/**  Definiciones y opera-  **/
/**  ciones algebráicas     **/
/*****************************/

//---   Definiciones   ---//

/*** Colores comunes ***/
#define WHITE   {1.0f, 1.0f, 1.0f, 1.0f}
#define BLACK   {0.0f, 0.0f, 0.0f, 1.0f}
#define RED     {1.0f, 0.0f, 0.0f, 1.0f}
#define GREEN   {0.0f, 1.0f, 0.0f, 1.0f}
#define BLUE    {0.0f, 0.0f, 1.0f, 1.0f}
#define YELLOW  {1.0f, 1.0f, 0.0f, 1.0f}
#define CYAN    {0.0f, 1.0f, 1.0f, 1.0f}
#define MAGENTA {1.0f, 0.0f, 1.0f, 1.0f}
#define GRAY    {0.5f, 0.5f, 0.5f, 1.0f}
#define PALE    {0.1f, 0.1f, 0.1f, 1.0f}

/*** Colores extra ***/
#define BEACH_SAND         {1.0f , 0.976, 0.616, 1.0f}
#define DESERT_SAND        {0.980, 0.804, 0.529, 1.0f}
#define LIGHTGREEN         {0.235, 0.722, 0.471, 1.0f}
#define PUREGREEN          {0.0f , 0.651, 0.318, 1.0f}
#define DARKGREEN          {0.0f , 0.447, 0.212, 1.0f}
#define LIGHT_YELLOW_GREEN {0.486, 0.773, 0.463, 1.0f}
#define PURE_YELLOW_GREEN  {0.224, 0.710, 0.290, 1.0f}
#define DARK_YELLOW_GREEN  {0.098, 0.482, 0.188, 1.0f}
#define LIGHTBROWN         {0.776, 0.612, 0.427, 1.0f}
#define DARKBROWN          {0.451, 0.392, 0.341, 1.0f}

#define MINVALUE(x,y) (x < y ? x : y )
#define MAXVALUE(x,y) (x > y ? x : y )
#define INTERVAL(x) (x >= 0.0f && x <= 1.0f)

/*_______*/


//---   Estructuras   ---//

/*** Estructura de Dato: COLOR ***/
typedef struct color
{
    GLfloat r; // Red
    GLfloat g; // Gree
    GLfloat b; // Blue
    GLfloat a; // Alpha
} COLOR;

/*** Estructura de Dato: TEXCOORD ***/
typedef struct texcoord
{
    GLfloat u; // Coord u
    GLfloat v; // Coord v
} TEXCOORD;

/*** Estructura de Dato: POINT ***/
typedef struct point
{
    GLfloat x; // Pos x
    GLfloat y; // Pos y
    GLfloat z; // pos z
} POINT;

/*** Estructura de Dato: VECTOR ***/
typedef struct vector
{
    GLfloat x; // Dir x 
    GLfloat y; // Dir y 
    GLfloat z; // Dir z
} VECTOR;

/*** Estructura de Dato: TRIANGLE ***/
typedef struct triangle
{
    VECTOR p0; // 1er punto
    VECTOR p1; // 2do punto
    VECTOR p2; // 3er punto
} TRIANGLE;

/*** Estructura de Dato: RAY ***/
typedef struct ray
{
    VECTOR p0; // Punto Inicial
    VECTOR u;  // Dirección
} RAY;

/*** Estructura de Dato: PLANE ***/
typedef struct plane
{
    VECTOR  n; // Normal del plano
    GLfloat d; // Distancia al plano
} PLANE;

/*** Estructura de Dato: SPHERE ***/
typedef struct sphere
{
    VECTOR  center; // Centro de la esfera
    GLfloat radius; // Radio de la esfera
} SPHERE;

/*** Estructura de Dato: ELLIPSOID ***/
typedef struct ellipsoid
{
    VECTOR center; // Centro de la ellipse
    VECTOR axes;   // Semi-axes
} ELLIPSOID;

/*** Estructura de Dato: BOX ***/
typedef struct box
{
    VECTOR min; // Extremo mínimo
    VECTOR max; // Extremo máximo
} BOX;

/*** Estructura de Dato: NORMAL_VERTEX ***/
typedef struct normal_tex_vertex
{
    POINT    p; // Ubicación
    VECTOR   n; // Normal
    TEXCOORD t; // Coord de textura
} NORMAL_TEX_VERTEX;

/*** Estructura de Dato: MATERIAL ***/
typedef struct material
{
    COLOR   ambient;   // Ambiente
    COLOR   diffuse;   // Difuso
    COLOR   specular;  // Especular
    COLOR   emission;  // Emisivo
    GLfloat shininess; // Brillo
} MATERIAL;

/*** Estructura de Dato: DIRLIGHT ***/
typedef struct dirlight
{
    COLOR   ambient;   // Color Ambiente
    COLOR   diffuse;   // Color Difuso
    COLOR   specular;  // Color Especular
    VECTOR  dir;       // Dirección
}DIRLIGHT;

/*** Estructura de Dato: POINTLIGHT ***/
typedef struct pointlight
{
    COLOR   ambient;   // Color Ambiente
    COLOR   diffuse;   // Color Difuso
    COLOR   specular;  // Color Especular
    POINT   pos;       // Posición
    GLfloat constAtt;  // Atenuación constante
    GLfloat linearAtt; // Atenuación lineal
    GLfloat quadrAtt;  // Atenuación cuadrática
}POINTLIGHT;

/*** Estructura de Dato: SPOTLIGHT ***/
typedef struct spotlight
{
    COLOR   ambient;   // Color Ambiente
    COLOR   diffuse;   // Color Difuso
    COLOR   specular;  // Color Especular
    POINT   pos;       // Posición
    VECTOR  dir;       // Dirección
    GLfloat intensity; // Concentración
    GLfloat angle;     // Ángulo
    GLfloat constAtt;  // Atenuación constante
    GLfloat linearAtt; // Atenuación lineal
    GLfloat quadrAtt;  // Atenuación cuadrática
}SPOTLIGHT;

/*________*/


//---   Funciones ---//

/*** Función: Calcular los FPS ***/
GLuint CalculateFPS( GLfloat elapsed )
{
  static GLfloat elapsedTime  = 0.0f;
  static GLuint  numberFrames = 0;
  static GLuint  FPS          = 0;

  elapsedTime += elapsed; // Tiempo se acumula
  numberFrames++;         // 1 cuadro se acumula
  if( elapsedTime > 1.0f )              // Ya pasó un segundo
  {
      FPS = numberFrames / elapsedTime; // FPS = Frames / Time
      elapsedTime = 0.0f;
      numberFrames = 0.0f;
  }

  return FPS;
}

/*** Función: devuelve un decimal aleatoria ***/
GLfloat RandomFloat( GLfloat min, GLfloat max, GLuint decimals )
{
  GLuint d = pow( 10, decimals );
  GLuint r = abs(max - min) * d;
  return min + (rand()%r / (GLfloat)d);
}

/*** Función: Resuelve una ecuación cuadrática ***/
GLboolean SolveQuadratic( GLfloat a, GLfloat b, GLfloat c,
			  GLfloat* r1, GLfloat* r2 )
{
  GLfloat det = b*b-4.0f*a*c;
  if( det < 0.0f )
    return GL_FALSE;

  GLfloat sqrtdet = sqrt(det);
  GLfloat root1 = -(b-sqrtdet)/(2.0f*a);
  GLfloat root2 = -(b+sqrtdet)/(2.0f*a);

  *r1 = MINVALUE( root1, root2 );
  *r2 = MAXVALUE( root1, root2 );

  return GL_TRUE;
}

/*--- VECTORES ---*/

/*** Función: Saca la norma de un vector ***/
GLfloat NormVector( VECTOR v )
{
  return sqrt( (v.x * v.x) + (v.y * v.y) + (v.z * v.z) );
}

/*** Función: Escalar un Vector ***/
VECTOR MulVector( VECTOR v, GLfloat k )
{
  VECTOR res = {v.x * k,
		v.y * k,
		v.z * k};
  return res;
}

/*** Función: Suma 2 vectores ***/
VECTOR SumVector( VECTOR v1, VECTOR v2 )
{
  VECTOR res = {v1.x + v2.x,
		v1.y + v2.y,
		v1.z + v2.z};
  return res;
}

/*** Función: Resta 2 vectores ***/
VECTOR ResVector( VECTOR v1, VECTOR v2 )
{
  VECTOR res = {v1.x - v2.x,
		v1.y - v2.y,
		v1.z - v2.z};
  return res;
}

/*** Función: Normaliza un vector **/
VECTOR NormalizeVector( VECTOR v )
{
  GLfloat n = NormVector( v );
  VECTOR res = {v.x / n,
		v.y / n,
		v.z / n};
  return res;
}

/*** Función: Producto punto ***/
float DotProduct( VECTOR v1, VECTOR v2 )
{
  return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

/*** Función: Producto cruz ***/
VECTOR CrossProduct( VECTOR v1, VECTOR v2 )
{
  VECTOR res;
  
  res.x = (v1.y * v2.z) - (v1.z * v2.y);
  res.y = (v1.z * v2.x) - (v1.x * v2.z);
  res.z = (v1.x * v2.y) - (v1.y * v2.x);
  
  return res;
}

/*** Función: Divisón de vectores ***/
VECTOR DivVector( VECTOR v1, VECTOR v2 )
{
  VECTOR res;
  
  res.x = v1.x / v2.x;
  res.y = v1.y / v2.y;
  res.z = v1.z / v2.z;
  
  return res;
}

/*** Función: Distancia Vector ***/
VECTOR VectorDistance( VECTOR v1, VECTOR v2 )
{
  VECTOR res;
  
  res.x = fabs( v1.x - v2.x );
  res.y = fabs( v1.y - v2.y );
  res.z = fabs( v1.z - v2.z );

  return res;
}

/*** Función: Norma cuadrada ***/
GLfloat Norm2Vector( VECTOR v )
{
  return (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
}

/*** Función: Verifica si un punto está dentro de un triángulo ***/
GLboolean PointInsideTriangle( TRIANGLE triangle, VECTOR point )
{
  VECTOR  v0   = ResVector( triangle.p1, triangle.p0 );
  VECTOR  v1   = ResVector( triangle.p2, triangle.p0 );
  VECTOR  v2   = ResVector( point      , triangle.p0 );
  
  GLfloat v0v0 = DotProduct( v0, v0 );
  GLfloat v1v1 = DotProduct( v1, v1 );
  GLfloat v0v1 = DotProduct( v0, v1 );
  GLfloat v0v2 = DotProduct( v0, v2 );
  GLfloat v1v2 = DotProduct( v1, v2 );
  
  GLfloat den  = ( v0v0 * v1v1 - v0v1 * v0v1 );
  GLfloat u    = ( v1v1 * v0v2 - v0v1 * v1v2 ) / den;
  GLfloat v    = ( v0v0 * v1v2 - v0v1 * v0v2 ) / den;

  return INTERVAL( u ) && INTERVAL( v ) && INTERVAL( u + v );
}

/*** Función: Transforma un punto en base a una matriz ***/
VECTOR TransformCoordFromMatrix( VECTOR v, GLfloat* M )
{
  VECTOR res;
  
  res.x = (v.x * M[0] + v.y * M[1] + v.z * M[ 2] + M[ 3]);
  res.y = (v.x * M[4] + v.y * M[5] + v.z * M[ 6] + M[ 7]);
  res.z = (v.x * M[8] + v.y * M[9] + v.z * M[10] + M[11]);
  
  return res;
}

/*** Función: Transforma un punto en base a la matriz del espacio visual  ***/
VECTOR TransformCoord( VECTOR v )
{
  GLfloat M[16];
  VECTOR res;
  
  glGetFloatv( GL_MODELVIEW_MATRIX, M );
  
  res.x = (v.x * M[0] + v.y * M[4] + v.z * M[ 8] + M[12]);
  res.y = (v.x * M[1] + v.y * M[5] + v.z * M[ 9] + M[13]);
  res.z = (v.x * M[2] + v.y * M[6] + v.z * M[10] + M[14]);
  
  return res;
}

/*** Función: Imprime un vector(stderr) ***/
void PrintVector( VECTOR v, char* name )
{
  fprintf( stderr, "%s: %.2f,%.2f,%.2f\n", name, v.x, v.y, v.z );
}

/*** Función: Imprime un error o una advertencia ***/
void PrintError( const char* msg, GLboolean warning )
{
  if( warning )
    fprintf( stderr, "WARNING: %s\n", msg );
  else
    fprintf( stderr, "ERROR: %s\n", msg );
}

/*---------------*/

/*** Función: Verifica si un punto está dentro de una caja ***/
GLboolean PointInsideBox( BOX b, VECTOR p )
{
  return( ( p.x >= b.min.x && p.x <= b.max.x ) &&
	  ( p.y >= b.min.y && p.y <= b.max.y ) &&
	  ( p.z >= b.min.z && p.z <= b.max.z ) );
}

/*--- Picking ---*/

/*** Función: Devuelve el rayo creado desde un punto x,y en la ventana **/
RAY PickingRay( GLdouble x, GLdouble y )
{
  RAY      res;
  GLdouble nearx, neary, nearz;
  GLdouble farx , fary , farz;
  GLfloat  depth;
  
  GLdouble model[16];
  GLdouble proj[16];
  GLint    view[4];

  /* Obtengo las matrices */
  glGetDoublev( GL_MODELVIEW_MATRIX, model );
  glGetDoublev( GL_PROJECTION_MATRIX, proj );
  glGetIntegerv( GL_VIEWPORT, view );

  /* Valor de produndidad del pixel xy */
  glReadPixels( x, view[3] - y,
		1, 1,
		GL_DEPTH_COMPONENT,
		GL_FLOAT, &depth );

  /* Punto en el plano cercano */
  gluUnProject( x, view[3] - y, 0,
		model, proj, view,
		&nearx, &neary, &nearz );

  /* Punto en el plano lejano */
  gluUnProject( x, view[3] - y, (GLdouble)depth,
		model, proj, view,
		&farx, &fary, &farz );

  /* Calculo el rayo */
  VECTOR near = {(GLfloat)nearx, (GLfloat)neary, (GLfloat)nearz };
  VECTOR far  = {(GLfloat)farx , (GLfloat)fary , (GLfloat)farz  };

  res.p0 = near;
  res.u  = NormalizeVector( SumVector( far, MulVector( near, -1.0f ) ) );

  return res;
}

/*** Función: Dice si un rayo intercepta una esfera ***/
GLboolean TestRaySphere( RAY ray, SPHERE sphere )
{
  VECTOR  v = SumVector( ray.p0, MulVector( sphere.center, -1.0f ) );
  GLfloat b = 2.0f * DotProduct( ray.u, v );
  GLfloat c = DotProduct( v, v ) - sphere.radius * sphere.radius;

  // Discriminante
  GLfloat discriminant = (b * b) - (4.0f * c);
  if( discriminant < 0.0f )
    // Respuesta imaginaria( no intersecta )
    return GL_FALSE;

  discriminant = sqrtf( discriminant );
  
  // 2 respuestas
  GLfloat s0 = (-b + discriminant) / 2.0f;
  GLfloat s1 = (-b - discriminant) / 2.0f;

  // Verifico que alguna respuesta sea válida
  if( s0 >= 0.0f || s1 >= 0.0f )
    return GL_TRUE;
  else
    return GL_FALSE;
}

/*---------------*/

/*** Función: Escalar un color ***/
void MulColor( const COLOR* color, GLfloat k, COLOR* out )
{
  out->r = color->r * k;
  out->g = color->g * k;
  out->b = color->b * k;
}

/*** Función: Pone un material **/
void SetMaterial( const MATERIAL* mtrl )
{
  // Ambiente
  glMaterialfv( GL_FRONT, GL_AMBIENT  , (GLfloat*)&mtrl->ambient   );
  // Difusa
  glMaterialfv( GL_FRONT, GL_DIFFUSE  , (GLfloat*)&mtrl->diffuse   );
  // Especular
  glMaterialfv( GL_FRONT, GL_SPECULAR , (GLfloat*)&mtrl->specular  );
  // Emisión
  glMaterialfv( GL_FRONT, GL_EMISSION , (GLfloat*)&mtrl->emission  );
  // Brillo
  glMaterialf ( GL_FRONT, GL_SHININESS, mtrl->shininess );
}

/*--- LUCES ---*/

/*** Función: Pone una luz direccional ***/
void SetDirLight( GLenum lightID, const DIRLIGHT* dirlight )
{
  // Ambiente
  glLightfv( lightID, GL_AMBIENT , (GLfloat*)&dirlight->ambient );
  // Difuso
  glLightfv( lightID, GL_DIFFUSE , (GLfloat*)&dirlight->diffuse );
  // Especular
  glLightfv( lightID, GL_SPECULAR, (GLfloat*)&dirlight->specular );
  // Dirección
  GLfloat dir[] = {dirlight->dir.x,
		   dirlight->dir.y,
		   dirlight->dir.z,
		   0.0f};
  glLightfv( lightID, GL_POSITION, dir );
}

/*** Función: Pone una luz puntual ***/
void SetPointLight( GLenum lightID, const POINTLIGHT* pointlight )
{
  // Ambiente
  glLightfv( lightID, GL_AMBIENT , (GLfloat*)&pointlight->ambient );
  // Difuso
  glLightfv( lightID, GL_DIFFUSE , (GLfloat*)&pointlight->diffuse );
  // Especular
  glLightfv( lightID, GL_SPECULAR, (GLfloat*)&pointlight->specular );
  // Posición
  GLfloat pos[] = {pointlight->pos.x,
		   pointlight->pos.y,
		   pointlight->pos.z,
		   1.0f};
  glLightfv( lightID, GL_POSITION, pos );
  // Atenuación constante
  glLightf( lightID, GL_CONSTANT_ATTENUATION, pointlight->constAtt );
  // Atenuación lineal
  glLightf( lightID, GL_LINEAR_ATTENUATION, pointlight->linearAtt );
  // Atenuación cuadrática
  glLightf( lightID, GL_QUADRATIC_ATTENUATION, pointlight->quadrAtt );
}

/*** Función: Pone una luz spotlight ***/
void SetSpotLight( GLenum lightID, const SPOTLIGHT* spotlight )
{
  // Ambiente
  glLightfv( lightID, GL_AMBIENT , (GLfloat*)&spotlight->ambient );
  // Difuso
  glLightfv( lightID, GL_DIFFUSE , (GLfloat*)&spotlight->diffuse );
  // Especular
  glLightfv( lightID, GL_SPECULAR, (GLfloat*)&spotlight->specular );
  // Posición
  GLfloat pos[] = {spotlight->pos.x,
		   spotlight->pos.y,
		   spotlight->pos.z,
		   1.0f};
  glLightfv( lightID, GL_POSITION, pos );
  // Dirección
  GLfloat dir[] = {spotlight->dir.x,
		   spotlight->dir.y,
		   spotlight->dir.z,
		   0.0f};
  glLightfv( lightID, GL_SPOT_DIRECTION, dir );
  // Intensidad
  glLightf( lightID, GL_SPOT_EXPONENT, spotlight->intensity );
  // Ángulo
  glLightf( lightID, GL_SPOT_CUTOFF, spotlight->angle / 2.0f );
  // Atenuación constante
  glLightf( lightID, GL_CONSTANT_ATTENUATION, spotlight->constAtt );
  // Atenuación lineal
  glLightf( lightID, GL_LINEAR_ATTENUATION, spotlight->linearAtt );
  // Atenuación cuadrática
  glLightf( lightID, GL_QUADRATIC_ATTENUATION, spotlight->quadrAtt );
}

/*-------------*/

/*** Función: Carga la matriz de proyección de sombra de una "spotlight" ***/
// LightDir    : Posición de la luz o dirección a la luz
// planeNormal : Normal al plano donde se proyectará la sombra
// p0          : Punto pertenciente al plano donde se proyectará la sombra
// Dir         : True->(Directional Light), False->(Point Light)
void LoadShadowMatrix( GLfloat* lightDir, VECTOR* planeNormal, VECTOR* p0, GLboolean dir )
{
  GLfloat planeD   = - (p0->x)*(planeNormal->x) - (p0->y)*(planeNormal->y) - (p0->z)*(planeNormal->z);
  GLfloat plane[4] = {planeNormal->x, planeNormal->y, planeNormal->z, planeD};
  GLfloat light[4] = {lightDir[0], lightDir[1], lightDir[2], dir ? 0.0f : 1.0f};
  GLfloat dot      = plane[0] * light[0] + plane[1] * light[1] +
    plane[2] * light[2] + plane[3] * light[3];
  
  GLfloat shadowMatrix[16];
  shadowMatrix[0]  = dot  - light[0] * plane[0];
  shadowMatrix[4]  = 0.0f - light[0] * plane[1];
  shadowMatrix[8]  = 0.0f - light[0] * plane[2];
  shadowMatrix[12] = 0.0f - light[0] * plane[3];
  
  shadowMatrix[1]  = 0.0f - light[1] * plane[0];
  shadowMatrix[5]  = dot  - light[1] * plane[1];
  shadowMatrix[9]  = 0.0f - light[1] * plane[2];
  shadowMatrix[13] = 0.0f - light[1] * plane[3];
  
  shadowMatrix[2]  = 0.0f - light[2] * plane[0];
  shadowMatrix[6]  = 0.0f - light[2] * plane[1];
  shadowMatrix[10] = dot  - light[2] * plane[2];
  shadowMatrix[14] = 0.0f - light[2] * plane[3];
  
  shadowMatrix[3] = 0.0f - light[3] * plane[0];
  shadowMatrix[7] = 0.0f - light[3] * plane[1];
  shadowMatrix[11] = 0.0f - light[3] * plane[2];
  shadowMatrix[15] = dot  - light[3] * plane[3];
  
  glMultMatrixf( shadowMatrix );
}

/*** Función: Carga la tetura del archivo "textureFile" ***/
GLuint LoadTexture( char* textureFile )
{
  /* Creo la textura y el ID de la textura */
  SDL_Surface* texture;           // Textura
  GLenum format;                  // Formato
  GLuint textureID;               // ID
  glGenTextures( 1, &textureID ); // Creo el ID

  /* Cargo la textura */
  texture = IMG_Load( textureFile );

  /* Identifico el formato de la textura */
  // Luminance
  if( texture->format->BytesPerPixel == 1 )
      format = GL_LUMINANCE;
  // 3 componentes
  if( texture->format->BytesPerPixel == 3 )
    {
      // RGB
      if( texture->format->Rmask < texture->format->Gmask &&
	  texture->format->Gmask < texture->format->Bmask )
	format = GL_RGB;
      // BGR
      if( texture->format->Bmask < texture->format->Gmask &&
	  texture->format->Gmask < texture->format->Rmask )
	format = GL_BGR;
    }
  // 4 componentes
  if (texture->format->BytesPerPixel == 4 )
    {
      // RGBA
      if( texture->format->Rmask < texture->format->Gmask &&
	  texture->format->Gmask < texture->format->Bmask )
	format = GL_RGBA;
      // BGRA
      if( texture->format->Bmask < texture->format->Gmask &&
	  texture->format->Gmask < texture->format->Rmask )
	format = GL_BGRA;
    }
  /*________*/
  
  /* Textura */
  glBindTexture( GL_TEXTURE_2D, textureID );
  // Pongo la textura en memoria
  SDL_LockSurface( texture );
  gluBuild2DMipmaps( GL_TEXTURE_2D,                  // Siempre GL_TEXTURE_2D
		     texture->format->BytesPerPixel, // Número de colores de la textura
		     texture->w,                     // Largo
		     texture->h,                     // Ancho
		     format,                         // Formato de imágen
		     GL_UNSIGNED_BYTE,               // Los datos de la textura están en bytes
		     texture->pixels );              // Puntero al contenido de la texura
  SDL_UnlockSurface( texture );
  // Libero la textura cargada
  SDL_FreeSurface( texture );

  // Devuelvo el ID de la textura
  return textureID;
}

/*** Función: Crea un screenshot del estado acutal del framebuffer en "fileName" ***/
void CreateScreenshot( char* appName )
{
  /* Nombre de Archivo */
  char   fileName[100];
  time_t actualTime;
  time( &actualTime );
  sprintf( fileName, "%s  %s.bmp", ctime( &actualTime ), appName );
  fileName[24] = ' '; fileName[25] = '-';

  /* Superficies */
  SDL_Surface* surface;
  SDL_Surface* screen;
  
  /* Leo la superficie actual */
  screen = SDL_GetVideoSurface();
  
  /* Creo y pongo los atributos en la superficie destino */
  surface = SDL_CreateRGBSurface( SDL_SWSURFACE,
				  screen->w,
				  screen->h,
				  screen->format->BitsPerPixel,
				  screen->format->Rmask,
				  screen->format->Gmask,
				  screen->format->Bmask,
				  screen->format->Amask );
  
  /* Creo la memoria para los pixeles */
  GLuint x = screen->w * screen->h * screen->format->BytesPerPixel;
  GLubyte pixels[x];
  
  /* Obtengo los datos del framebuffer */
  glReadPixels( 0, 0, screen->w, screen->h, GL_BGRA, GL_UNSIGNED_BYTE, pixels );
  
  /* La invierto(para ajustar al formato BMP) */
  SDL_LockSurface( surface );
  int h;
  for( h = 0; h < screen->h; h++ )
    memcpy( &((GLubyte*)surface->pixels)[h * screen->w * screen->format->BytesPerPixel],
	    &pixels[(screen->h - h - 1) * screen->w * screen->format->BytesPerPixel],
	    screen->w * screen->format->BytesPerPixel * sizeof(GLubyte) );
  SDL_UnlockSurface( surface );
  
  /* La grabo y libero las superficies */
  SDL_SaveBMP( surface, fileName );
  SDL_FreeSurface( screen );
  SDL_FreeSurface( surface );
}

/*________*/
