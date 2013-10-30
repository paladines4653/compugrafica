/*****************************/
/**       -----------       **/
/**       collision.c       **/
/**       -----------       **/
/** Creación de volumenes y **/
/** detección de colisiones **/
/*****************************/

//--- Estructuras ---//

/*** Estructura de Dato: OBJECT ***/
typedef struct volumes
{
  ELLIPSOID   ellipsoid;    // Bounding Ellipsoid
  SPHERE      sphere;       // Bounding Sphere
  BOX         box;          // Bounding Box
  GLfloat     matrix[16];   // World Matrix
} VOLUMES;

//--- Funciones ---//

/*** Función: Actualiza la matriz de transformación de los objetos ***/
void BoundingVolumes( MODEL*    model  ,  // Modelo a sacar los volúmenes
		      VOLUMES*  volumes,  // Salida de los datos de los volúmenes
		      GLfloat*  matrix ,  // Transformación(NULL usa la opengl)
		      GLboolean verbose ) // Imprime los volumenes calculados
{
  /* Matriz */
  if( matrix == NULL )
    glGetFloatv( GL_TRANSPOSE_MODELVIEW_MATRIX, volumes->matrix );
  else
    memcpy( volumes->matrix, matrix, sizeof(GLfloat) * 16 );

  /* Volumenes */
  VECTOR  min    = {  INFINITY,  INFINITY,  INFINITY };
  VECTOR  max    = { -INFINITY, -INFINITY, -INFINITY };

  // Caja
  volumes->box.min = min;
  volumes->box.max = max;
  unsigned int i;
  for( i = 0; i < model->vertexCount; i++ )
    {
      VECTOR vertex = TransformCoordFromMatrix( model->vertexBuffer[i],
						volumes->matrix );
      
      volumes->box.min.x = MINVALUE(vertex.x,volumes->box.min.x);
      volumes->box.min.y = MINVALUE(vertex.y,volumes->box.min.y);
      volumes->box.min.z = MINVALUE(vertex.z,volumes->box.min.z);
      volumes->box.max.x = MAXVALUE(vertex.x,volumes->box.max.x);
      volumes->box.max.y = MAXVALUE(vertex.y,volumes->box.max.y);
      volumes->box.max.z = MAXVALUE(vertex.z,volumes->box.max.z);
    }

  // Esfera y elipse
  GLfloat eradius         = 0.0f;
  volumes->sphere.radius  = 0.0f;
  VECTOR dist   = VectorDistance( volumes->box.min, volumes->box.max );
  VECTOR center = SumVector( MulVector( ResVector( volumes->box.max,
						   volumes->box.min ), 0.5f ),
			     volumes->box.min );
  VECTOR  ecenter = { center.x / dist.x, center.y / dist.y, center.z / dist.z };
  volumes->sphere.center    = center;
  volumes->ellipsoid.center = center;

  for( i = 0; i < model->vertexCount; i++ )
    {
      // Esfera
      GLfloat radius;
      VECTOR vertex = TransformCoordFromMatrix( model->vertexBuffer[i],
						volumes->matrix );

      radius = Norm2Vector( ResVector( vertex, center ) );
      volumes->sphere.radius = MAXVALUE( volumes->sphere.radius, radius );

      // Elipse
      vertex.x /= dist.x; vertex.y /= dist.y; vertex.z /= dist.z;

      radius = Norm2Vector( ResVector( vertex, ecenter ) );
      eradius = MAXVALUE( eradius, radius );
    }
  volumes->sphere.radius  = sqrt( volumes->sphere.radius );
  volumes->ellipsoid.axes = MulVector( dist, sqrt(eradius) );

  if( verbose )
    {
      fprintf( stderr, "-Bounding Volumes-\n" );
      PrintVector( volumes->box.min         , "\tBox Minimum      " );
      PrintVector( volumes->box.max         , "\tBox Maximum      " );
      PrintVector( volumes->sphere.center   , "\tSphere Center    " );
      fprintf( stderr, "\tSphere Radius    : %.2f\n", volumes->sphere.radius );
      PrintVector( volumes->ellipsoid.center, "\tEllipsoid Center " );
      PrintVector( volumes->ellipsoid.axes  , "\tEllipsoid Axes   " );
    }
}

/*** Función: Crea los volúmenes de una cámara con los ejes de la elipse  ***/
void CreateCameraVolume( CAMERA camera, VECTOR axes, VOLUMES* volumes )
{
  volumes->ellipsoid.center = camera.pos;
  volumes->ellipsoid.axes   = axes;
  volumes->sphere.center    = camera.pos;
  volumes->sphere.radius    = MAXVALUE( axes.x, MAXVALUE( axes.y, axes.z ) );
  volumes->box.min          = ResVector( camera.pos, axes );
  volumes->box.max          = SumVector( camera.pos, axes );
  GLfloat matrix[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
			 0.0f, 1.0f, 0.0f, 0.0f,
			 0.0f, 0.0f, 1.0f, 0.0f,
			 0.0f, 0.0f, 0.0f, 1.0f };
  memcpy( volumes->matrix, matrix, sizeof(GLfloat) * 16 );
}

/*** Función: Actuliza los volúmenes si el objeto se desea desplazar ***/
void UpdateVolumes( VOLUMES* volumes, VECTOR displacement )
{
  // Update volume positions
  volumes->box.min          = SumVector( volumes->box.min,
					 displacement );
  volumes->box.max          = SumVector( volumes->box.max,
					 displacement );
  volumes->sphere.center    = SumVector( volumes->sphere.center,
					 displacement );
  volumes->ellipsoid.center = SumVector( volumes->ellipsoid.center,
					 displacement );

  // Update world matrix
  glPushMatrix();
  glLoadIdentity();
  glTranslatef( displacement.x, displacement.y, displacement.z );
  glMultTransposeMatrixf( volumes->matrix );
  glGetFloatv( GL_TRANSPOSE_MODELVIEW_MATRIX, volumes->matrix );
  glPopMatrix();
}

/*** Función: Detecta si un objeto está solapado dentro de una esfera ***/
GLboolean InsideSphere( VOLUMES* obj1, VOLUMES* obj2, GLfloat tolerance )
{
  return 
    Norm2Vector( ResVector( obj1->sphere.center, obj2->sphere.center ) ) 
    <=
    pow( obj1->sphere.radius + obj2->sphere.radius + tolerance, 2.0f );
}

/*** Función: Verifica si existe una colisión de esferas entre 2 objetos ***/
GLboolean SphereCollision( VOLUMES* obj1        , // Objeto a colisionar
			   VOLUMES* obj2        , // Objeto a verificar
			   VECTOR   displacement, // Desplazamiento destino
			   GLfloat* outTime     , // Tiempo de colisión
			   VECTOR*  outPos      ) // Punto de colisión
{
  outPos->x = NAN;
  outPos->y = NAN;
  outPos->z = NAN;
  *outTime  = NAN;

  VECTOR  c1  = obj1->sphere.center;
  VECTOR  c2  = obj2->sphere.center;
  GLfloat r1  = obj1->sphere.radius;
  GLfloat r2  = obj2->sphere.radius;
  GLfloat a   = ( pow( displacement.x, 2.0f ) + 
		  pow( displacement.y, 2.0f ) + 
		  pow( displacement.z, 2.0f ) );
  GLfloat b   = 2.0f * ( displacement.x * ( c1.x - c2.x ) + 
			 displacement.y * ( c1.y - c2.y ) + 
			 displacement.z * ( c1.z - c2.z ) );
  GLfloat c = ( pow( c1.x - c2.x, 2.0f ) + 
		pow( c1.y - c2.y, 2.0f ) +  
		pow( c1.z - c2.z, 2.0f ) - 
		( r1 + r2 ) * ( r1 + r2 ) );

  if( SolveQuadratic( a, b, c, &r1, &r2 ) )
    {
      if( INTERVAL( r1 ) )
	{ 
	  *outTime = r1;
	  VECTOR center =
	    SumVector( obj1->sphere.center, MulVector( displacement, r1 ) );
	  *outPos       =  
	    SumVector( center, 
		       MulVector( NormalizeVector(ResVector( obj2->sphere.center,
							     center ) ),
				  obj1->sphere.radius ) );
	  return GL_TRUE;
	}
      if( INTERVAL( r2 ) )
	{
	  *outTime = r2;
	  VECTOR center =
	    SumVector( obj1->sphere.center, MulVector( displacement, r2 ) );
	  *outPos       =  
	    SumVector( center, 
		       MulVector( NormalizeVector(ResVector( obj2->sphere.center,
							center ) ),
				  obj1->sphere.radius ) );
	  return GL_TRUE;
	}
    }
  return GL_FALSE;
}

/*** Función: Calcula y detecta si hay colisión contra un triángulo ***/
GLboolean CollisionDetectionTri( TRIANGLE eTri   , // Triangulo a verificar
				 VECTOR   ePos   , // Posición del objeto
				 VECTOR   eDisp  , // Desplazamiento del objeto
				 GLfloat* outTime, // Tiempo de colisión
				 VECTOR*  outPos ) // Posición de colisión
{
  VECTOR  pos        = { NAN, NAN, NAN };
  GLfloat time       = HUGE_VALF;
  GLfloat t0         = NAN;
  GLfloat t1         = NAN;
  GLboolean embedded = GL_FALSE;

  /* Plano del triángulo */
  VECTOR normal = 
    NormalizeVector( CrossProduct( ResVector( eTri.p2, eTri.p0),
				   ResVector( eTri.p1, eTri.p0) ) );
  PLANE plane = { normal, -1.0f * DotProduct( normal, eTri.p0 ) };

  /* Cálculos */
  GLfloat pDistance    = DotProduct( plane.n, ePos ) + plane.d;
  GLfloat dotNormalVel = DotProduct( plane.n, eDisp );

  /* Tiempos de colisión */
  if( dotNormalVel != 0.0f )
    {
      GLfloat ti = (  1.0f - pDistance ) / dotNormalVel;
      GLfloat tf = ( -1.0f - pDistance ) / dotNormalVel;

      // Organizo los valores
      t0 = MINVALUE( ti, tf );
      t1 = MAXVALUE( ti, tf );

      if( t0 > 1.0f || t1 < 0.0f )
	return GL_FALSE;
      
      if( !INTERVAL( t0 ) )
	t0 = INFINITY;
      if( !INTERVAL( t1 ) )
	t1 = INFINITY;
    }
  /* Recorrido paralelo al plano */
  else
    {
      /* Nunca colisiona */
      if( fabs( pDistance ) >= 1.0f )
	return GL_FALSE;
      /* Siempre colisiona */
      else
	{
	  embedded = GL_TRUE;
	  t0 = 0.0f;
	  t1 = 1.0f;
	}
    }

  /* Detección de colisión dentro del triángulo */
  time = MINVALUE( t0, t1 );
  if( !embedded )
    {
      VECTOR collisionPoint = 
	ResVector( SumVector( ePos, MulVector( eDisp, time ) ), plane.n );
      if( PointInsideTriangle( eTri, collisionPoint ) )
	{
	  *outTime = t0;
	  *outPos  = collisionPoint;
	  return GL_TRUE;
	}
    }

  /* Detección de colisión con vértice */
  time      = HUGE_VALF;
  GLfloat a = DotProduct( eDisp, eDisp );
  GLfloat b = NAN;
  GLfloat c = NAN;
  // Vértice 0
  b = 2.0f * DotProduct( eDisp, ResVector( ePos, eTri.p0 ) );
  c = Norm2Vector( ResVector( eTri.p0, ePos ) ) - 1.0f;
  if( SolveQuadratic( a, b, c, &t0, &t1 ) )
    {
      if( !INTERVAL( t0 ) )
	t0 = INFINITY;
      if( !INTERVAL( t1 ) )
	t1 = INFINITY;
      if( time > MINVALUE( t0, t1 ) )
	{
	  time = MINVALUE( t0, t1 );
	  pos  = eTri.p0;
	}
    }
  // Vértice 1
  b = 2.0f * DotProduct( eDisp, ResVector( ePos, eTri.p1 ) );
  c = Norm2Vector( ResVector( eTri.p1, ePos ) ) - 1.0f;
  if( SolveQuadratic( a, b, c, &t0, &t1 ) )
    {
      if( !INTERVAL( t0 ) )
	t0 = INFINITY;
      if( !INTERVAL( t1 ) )
	t1 = INFINITY;
      if( time > MINVALUE( t0, t1 ) )
	{
	  time = MINVALUE( t0, t1 );
	  pos  = eTri.p1;
	}
    }
  // Vértice 2
  b = 2.0f * DotProduct( eDisp, ResVector( ePos, eTri.p2 ) );
  c = Norm2Vector( ResVector( eTri.p2, ePos ) ) - 1.0f;
  if( SolveQuadratic( a, b, c, &t0, &t1 ) )
    {
      if( !INTERVAL( t0 ) )
	t0 = INFINITY;
      if( !INTERVAL( t1 ) )
	t1 = INFINITY;
      if( time > MINVALUE( t0, t1 ) )
	{
	  time = MINVALUE( t0, t1 );
	  pos  = eTri.p2;
	}
    }
  
  /* Detección de colisión con extremos */
  // Extremo p0----p1
  VECTOR  edge  = ResVector( eTri.p1, eTri.p0 );
  VECTOR  dist  = ResVector( eTri.p0, ePos );
  GLfloat edge2 = Norm2Vector( edge );
  a = edge2 * -1.0f * Norm2Vector( eDisp ) + 
    pow( DotProduct( edge, eDisp ), 2.0f );
  b = edge2 * 2.0f * DotProduct( eDisp, dist ) - 
    2.0f * DotProduct( edge, eDisp ) * DotProduct( edge, dist );
  c = edge2 * ( 1.0f - Norm2Vector( dist ) ) + 
    pow( DotProduct( edge, dist ), 2.0f );
  if( SolveQuadratic( a, b, c, &t0, &t1 ) )
    {
      if( !INTERVAL( t0 ) )
	t0 = INFINITY;
      if( !INTERVAL( t1 ) )
	t1 = INFINITY;
      if( time > MINVALUE( t0, t1 ) )
	{
	  GLfloat frac = ( MINVALUE( t0, t1 ) * DotProduct( edge, eDisp ) - 
			   DotProduct( edge, dist ) ) / edge2;
	  if( INTERVAL( frac ) )
	    {
	      time = MINVALUE( t0, t1 );
	      pos  = SumVector( eTri.p0, MulVector( edge, frac ) );
	    }
	}
    }
  // Extremo p1----p2
  edge  = ResVector( eTri.p2, eTri.p1 );
  dist  = ResVector( eTri.p1, ePos );
  edge2 = Norm2Vector( edge );
  a = edge2 * -1.0f * Norm2Vector( eDisp ) + 
    pow( DotProduct( edge, eDisp ), 2.0f );
  b = edge2 * 2.0f * DotProduct( eDisp, dist ) - 
    2.0f * DotProduct( edge, eDisp ) * DotProduct( edge, dist );
  c = edge2 * ( 1.0f - Norm2Vector( dist ) ) + 
    pow( DotProduct( edge, dist ), 2.0f );
  if( SolveQuadratic( a, b, c, &t0, &t1 ) )
    {
      if( !INTERVAL( t0 ) )
	t0 = INFINITY;
      if( !INTERVAL( t1 ) )
	t1 = INFINITY;
      if( time > MINVALUE( t0, t1 ) )
	{
	  GLfloat frac = ( MINVALUE( t0, t1 ) * DotProduct( edge, eDisp ) - 
			   DotProduct( edge, dist ) ) / edge2;
	  if( INTERVAL( frac ) )
	    {
	      time = MINVALUE( t0, t1 );
	      pos  = SumVector( eTri.p1, MulVector( edge, frac ) );
	    }
	}
    }
  // Extremo p2----p0
  edge  = ResVector( eTri.p0, eTri.p2 );
  dist  = ResVector( eTri.p2, ePos );
  edge2 = Norm2Vector( edge );
  a = edge2 * -1.0f * Norm2Vector( eDisp ) + 
    pow( DotProduct( edge, eDisp ), 2.0f );
  b = edge2 * 2.0f * DotProduct( eDisp, dist ) - 
    2.0f * DotProduct( edge, eDisp ) * DotProduct( edge, dist );
  c = edge2 * ( 1.0f - Norm2Vector( dist ) ) + 
    pow( DotProduct( edge, dist ), 2.0f );
  if( SolveQuadratic( a, b, c, &t0, &t1 ) )
    {
      if( !INTERVAL( t0 ) )
	t0 = INFINITY;
      if( !INTERVAL( t1 ) )
	t1 = INFINITY;
      if( time > MINVALUE( t0, t1 ) )
	{
	  GLfloat frac = ( MINVALUE( t0, t1 ) * DotProduct( edge, eDisp ) - 
			   DotProduct( edge, dist ) ) / edge2;
	  if( INTERVAL( frac ) )
	    {
	      time = MINVALUE( t0, t1 );
	      pos  = SumVector( eTri.p2, MulVector( edge, frac ) );
	    }
	}
    }

  /* Decisión si se encontró colisión o no */
  if( INTERVAL( time ) )
    {
      *outTime = time;
      *outPos  = pos;
      return GL_TRUE;
    }
  
  /* No encontró una colisión */
  return GL_FALSE;
}

/*** Función: Calcula y detecta la colisión de un objeto con otro ***/
GLboolean CollisionDetectionTerr( TERRAIN* terrain , // Terreno a verificar
				  VOLUMES* cObj    , // Geometría a colisionar
				  VECTOR   disp    , // Desplazamiento del objeto
				  GLfloat* outTime , // Tiempo de colisión
				  VECTOR*  outPos  ) // Posición de colisión
{
  GLfloat minTime = INFINITY;
  unsigned int i;
  unsigned int nIndices = (terrain->vertsPerRow - 1) * 
    (terrain->vertsPerCol - 1) * 2 * 3;
  /* Todos los triángulos del terreno */
  for( i = 0; i < nIndices; i += 3 )
    {
      GLfloat time  = INFINITY;
      VECTOR  pos   = { NAN, NAN, NAN };
      VECTOR  ePos  = DivVector( cObj->ellipsoid.center, cObj->ellipsoid.axes );
      VECTOR  eDisp = DivVector( disp, cObj->ellipsoid.axes );
      VECTOR p0 = { terrain->vertexBuffer[terrain->indexBuffer[i+0]].p.x,
		    terrain->vertexBuffer[terrain->indexBuffer[i+0]].p.y, 
		    terrain->vertexBuffer[terrain->indexBuffer[i+0]].p.z };
      VECTOR p1 = { terrain->vertexBuffer[terrain->indexBuffer[i+1]].p.x, 
		    terrain->vertexBuffer[terrain->indexBuffer[i+1]].p.y, 
		    terrain->vertexBuffer[terrain->indexBuffer[i+1]].p.z };
      VECTOR p2 = { terrain->vertexBuffer[terrain->indexBuffer[i+2]].p.x, 
		    terrain->vertexBuffer[terrain->indexBuffer[i+2]].p.y, 
		    terrain->vertexBuffer[terrain->indexBuffer[i+2]].p.z };
      TRIANGLE eTri  = 
	{ DivVector( p0, cObj->ellipsoid.axes ),
	  DivVector( p1, cObj->ellipsoid.axes ),
	  DivVector( p2, cObj->ellipsoid.axes ) };
		       
      /* Detecto la colisión con un triángulo */
      if( CollisionDetectionTri( eTri, ePos, eDisp, &time, &pos ) )
	if( minTime > time )
	  {
	    /* Actualizo los datos en caso de colisión más temprana */
	    minTime = time;
	    *outPos = pos;
	  }
    }
  if( INTERVAL( minTime ) )
    {
      *outTime   = minTime;
      outPos->x *= cObj->ellipsoid.axes.x;
      outPos->y *= cObj->ellipsoid.axes.y;
      outPos->z *= cObj->ellipsoid.axes.z;
      return GL_TRUE;
    }
  return GL_FALSE;
}

/*** Función: Calcula y detecta la colisión de un objeto con otro ***/
GLboolean CollisionDetectionObj( MODEL*   model  , // Modelo a verificar 
				 VOLUMES* mObj   , // Geometría a verificar
				 VOLUMES* cObj   , // Geometría a colisionar
				 VECTOR   disp   , // Desplazamiento del objeto
				 GLfloat* outTime, // Tiempo de colisión
				 VECTOR*  outPos ) // Posición de colisión
{
  GLfloat minTime = INFINITY;
  unsigned int i;
  /* Todos los triángulos de un objeto */
  for( i = 0; i < model->indexCount; i += 3 )
    {
      GLfloat  time  = INFINITY;
      VECTOR   pos   = { NAN, NAN, NAN };
      VECTOR   ePos  = DivVector( cObj->ellipsoid.center, cObj->ellipsoid.axes );
      VECTOR   eDisp = DivVector( disp, cObj->ellipsoid.axes );
      TRIANGLE eTri  = 
	{ DivVector( TransformCoordFromMatrix( model->vertexBuffer[model->indexBuffer[i+0]], mObj->matrix ), cObj->ellipsoid.axes ),
	  DivVector( TransformCoordFromMatrix( model->vertexBuffer[model->indexBuffer[i+1]], mObj->matrix ), cObj->ellipsoid.axes ),
	  DivVector( TransformCoordFromMatrix( model->vertexBuffer[model->indexBuffer[i+2]], mObj->matrix ), cObj->ellipsoid.axes ) };
		       
      /* Detecto la colisión con un triángulo */
      if( CollisionDetectionTri( eTri, ePos, eDisp, &time, &pos ) )
	if( minTime > time )
	  {
	    /* Actualizo los datos en caso de colisión más temprana */
	    minTime = time;
	    *outPos = pos;
	  }
    }
  if( INTERVAL( minTime ) )
    {
      *outTime   = minTime;
      outPos->x *= cObj->ellipsoid.axes.x;
      outPos->y *= cObj->ellipsoid.axes.y;
      outPos->z *= cObj->ellipsoid.axes.z;
      return GL_TRUE;
    }
  return GL_FALSE;
}

/*** Función: Calcula y detecta la colisión del desplazamiento de un objeto ***/
GLboolean CollisionDetection( TERRAIN* terrain     , // Terreno del mundo
			      MODEL*   models[]    , // Modelos del mundo
			      VOLUMES* volumes[]   , // Volúmenes del mundo
			      GLuint   nObjects    , // No. de objetos del mundo
			      GLuint   nObj        , // Objeto el cual colisionar
			      VECTOR   displacement, // Desplazamiento esperado
			      GLuint*  outObj      , // Objeto colisionado
			      GLfloat* outTime     , // Tiempo de colisión
			      VECTOR*  outPos      ) // Posición de la colisión
{
  /* Variables sin valor aún */
  outPos->x = NAN;
  outPos->y = NAN;
  outPos->z = NAN;
  *outObj   = nObj;
  *outTime  = INFINITY;
  /* Variables */
  GLboolean collided = GL_FALSE;
  GLfloat   time     = INFINITY;
  VECTOR    pos      = { NAN, NAN, NAN };
  /* Terreno */
  if( terrain != NULL && CollisionDetectionTerr( terrain, volumes[nObj],
						 displacement, &time, &pos ) )
    {
      if( time < *outTime )
	{
	  collided = GL_TRUE;
	  *outTime = time;
	  *outPos  = pos;
	  // Si colisiona con el terreno, el objeto es él mismo.
	  *outObj  = nObj;
	}
    }
  
  /* Todos los objetos */
  unsigned int i;
  for( i = 0; i < nObjects; i++ )
    /* Objeto Ith*/
    if( i != nObj && InsideSphere( volumes[nObj], volumes[i], 1.0f ) )
      {
	/* Detecto la colisión */
	if( CollisionDetectionObj( models[i], volumes[i], volumes[nObj],
				   displacement, &time, &pos ) )
	  if( time < *outTime )
	    {
	      /* Actualizo los datos en caso de colisión más temprana */
	      collided = GL_TRUE;
	      *outTime = time;
	      *outPos  = pos;
	      *outObj  = i;
	    }
      }
  return collided;
}

/*** Función: Crea un nuevo desplazamiento en caso de colisionar ***/
VECTOR CollisionResponse( VOLUMES* volumes     ,  // Volúmenes NO acutalizados
			  VECTOR   collisionPos,  // Posición de la colisión
			  VECTOR   displacement ) // Desplazamiento original
{
  VECTOR eCollisionPos    = DivVector( collisionPos, volumes->ellipsoid.axes );
  VECTOR eDisplacement    = DivVector( displacement, volumes->ellipsoid.axes );
  VECTOR eEllipsoidCenter =
    DivVector( volumes->ellipsoid.center, volumes->ellipsoid.axes );
  VECTOR eDestination     = SumVector( eEllipsoidCenter, eDisplacement );
  VECTOR slidingNormal    = 
    NormalizeVector( ResVector( eEllipsoidCenter, eCollisionPos ) );
  
  PLANE slidingPlane = { slidingNormal, -1.0f * DotProduct( slidingNormal,
							    eCollisionPos ) };
  VECTOR newDisplacement = 
    ResVector( ResVector( eDestination,
			  MulVector( slidingPlane.n, 
				     DotProduct( slidingPlane.n, eDestination ) +
				     slidingPlane.d ) ),
	       eCollisionPos );

  newDisplacement.x *= volumes->ellipsoid.axes.x;
  newDisplacement.y *= volumes->ellipsoid.axes.y;
  newDisplacement.z *= volumes->ellipsoid.axes.z;
  return newDisplacement;
}

/*** Función: Detección de colisiones y respuesta en una cámara ***/
void CollisionAndResponse( GLuint   nIterations ,  // No. de iteraciones en col.
			   GLfloat  epsilon     ,  // Exactitud de la respuesta
			   CAMERA*  camera      ,  // Cámara a colisionar
			   TERRAIN* terrain     ,  // Terreno del mundo
			   MODEL*   models[]    ,  // Modelos del mundo
			   VOLUMES* volumes[]   ,  // Volúmenes del mundo
			   GLuint   nObjects    ,  // No. de objetos del mundo
			   GLuint   nObj        ,  // Objeto el cual colisionar
			   VECTOR   displacement ) // Desplazamiento esperado
{
  GLuint  iteration, outObj;
  GLfloat dispNorm, outTime;
  VECTOR  outPos;
  
  /* Iteraciones de colisión */
  /* Se realiza la colisión hasta que el número de iteraciones se cumpla
     o el nuevo desplazamiento sea muy pequeño */
  for( iteration = 0, dispNorm = NormVector( displacement );
       iteration < nIterations && dispNorm > epsilon;
       iteration++, dispNorm = NormVector( displacement ) )
    {
      if( CollisionDetection( terrain, models, volumes,
			      nObjects, nObj, displacement,
			      &outObj, &outTime, &outPos ) )
	{
	  // Desplazamiento sin colisionar(un poco antes de la colisión)
	  VECTOR newDisp = MulVector( displacement, outTime * ( 1.0f - epsilon ) );
	  VECTOR nonDisp = MulVector( displacement, outTime * epsilon );
	  // Respuesta
	  displacement = CollisionResponse( volumes[nObj], 
					    ResVector( outPos, nonDisp ), 
					    displacement );
	  // Acutalizo volúmenes
	  UpdateVolumes( volumes[nObj], newDisp );
	  // Actualizo cámara
	  camera->pos = SumVector( camera->pos, newDisp );
	}
      else
	{
	  // Actualizo volúmenes
	  UpdateVolumes( volumes[nObj], displacement );
	  // Actualizo cámara
	  camera->pos = SumVector( camera->pos, displacement );
	  // Salgo
	  break;
	}
    }
}

/*** Función: Crea una lista de ejecución para dibujar el "bounding box" ***/
GLuint RenderBoundingBox( VECTOR* boxMin, VECTOR* boxMax, MATERIAL* boxMaterial )
{
  GLfloat x  = boxMax->x - boxMin->x;
  GLfloat y  = boxMax->y - boxMin->y;

  /* Puntos */
  POINT p[] = { 
    /* Cara trasera */
    {boxMin->x    , boxMin->y    , boxMin->z}, // p0
    {boxMin->x    , boxMin->y + y, boxMin->z}, // p1
    {boxMin->x + x, boxMin->y + y, boxMin->z}, // p2
    {boxMin->x + x, boxMin->y    , boxMin->z}, // p3
    /* Cara delantera */
    {boxMax->x - x, boxMax->y - y, boxMax->z}, // p4
    {boxMax->x - x, boxMax->y    , boxMax->z}, // p5
    {boxMax->x    , boxMax->y    , boxMax->z}, // p6
    {boxMax->x    , boxMax->y - y, boxMax->z}  // p7
  };

  /* Normales */
  VECTOR n[] = {
    { 0.0f,  0.0f, -1.0f}, // n0 - Cara trasera
    { 0.0f,  0.0f,  1.0f}, // n1 - Cara delantera
    { 0.0f, -1.0f,  0.0f}, // n2 - Cara inferior
    { 0.0f,  1.0f,  0.0f}, // n3 - Cara superior
    {-1.0f,  0.0f,  0.0f}, // n4 - Cara izquierda
    { 1.0f,  0.0f,  0.0f}  // n5 - Cara derecha
  };

  /* Textura */
  TEXCOORD t[] = {
    {0.0f, 0.0f}, // t0
    {1.0f, 0.0f}, // t1
    {0.0f, 1.0f}, // t2
    {1.0f, 1.0f}  // t3
  };

  /* Creo el buffer de vértices */
  NORMAL_TEX_VERTEX v[] = {
    /* Cara Trasera*/
    {p[3], n[0], t[2]}, {p[2], n[0], t[0]}, {p[1], n[0], t[1]},
    {p[3], n[0], t[2]}, {p[1], n[0], t[1]}, {p[0], n[0], t[3]},
    /* Cara Delantera */
    {p[4], n[1], t[2]}, {p[5], n[1], t[0]}, {p[6], n[1], t[1]},
    {p[4], n[1], t[2]}, {p[6], n[1], t[1]}, {p[7], n[1], t[3]},
    /* Cara Inferior */
    {p[7], n[2], t[2]}, {p[3], n[2], t[0]}, {p[0], n[2], t[1]},
    {p[7], n[2], t[2]}, {p[0], n[2], t[1]}, {p[4], n[2], t[3]},
    /* Cara Superior */
    {p[5], n[3], t[2]}, {p[1], n[3], t[0]}, {p[2], n[3], t[1]},
    {p[5], n[3], t[2]}, {p[2], n[3], t[1]}, {p[6], n[3], t[3]},
    /* Cara Izquierda */
    {p[0], n[4], t[2]}, {p[1], n[4], t[0]}, {p[5], n[4], t[1]},
    {p[0], n[4], t[2]}, {p[5], n[4], t[1]}, {p[4], n[4], t[3]},
    /* Cara Derecha */
    {p[7], n[5], t[2]}, {p[6], n[5], t[0]}, {p[2], n[5], t[1]},
    {p[7], n[5], t[2]}, {p[2], n[5], t[1]}, {p[3], n[5], t[3]},
  };

  /* Ato los buffers */
  glVertexPointer( 3, GL_FLOAT, sizeof(NORMAL_TEX_VERTEX), &(v[0].p) );
  glNormalPointer( GL_FLOAT, sizeof(NORMAL_TEX_VERTEX), &(v[0].n) );
  glTexCoordPointer( 2 , GL_FLOAT, sizeof(NORMAL_TEX_VERTEX), &(v[0].t) );

  /* Creo la lista y dibujo */
  GLuint boxList = glGenLists( 1 );
  glNewList( boxList, GL_COMPILE );
  glPushAttrib( GL_ENABLE_BIT );
  glDisable( GL_TEXTURE_2D );
  glEnable( GL_ALPHA_TEST );
  glEnable( GL_BLEND );
  SetMaterial( boxMaterial );
  glDrawArrays( GL_TRIANGLES, 0, sizeof(v)/sizeof(NORMAL_TEX_VERTEX) );
  glPopAttrib();
  glEndList();

  return boxList;
}

/*________*/
