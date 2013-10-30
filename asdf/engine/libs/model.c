/*****************************/
/**        --------         **/
/**         model.c         **/
/**        --------         **/
/**  Carga y renderizado    **/
/**  de modelos en archivos **/
/*****************************/

//--- Definiciones ---//
#define AI_CONFIG_PP_RVC_FLAGS (aiComponent_ANIMATIONS|aiComponent_BONEWEIGHTS|aiComponent_LIGHTS|aiComponent_CAMERAS)

//--- Estructuras ---//

/*** Estructura de dato: PROPERTIES ***/
typedef struct properties
{
  GLboolean wireframe;
  GLboolean culling;
  GLboolean flat;
  GLboolean transparency;
  GLboolean blending;
  GLenum    texOp;
} PROPERTIES;

/*** Estructura de dato: MODEL ***/
typedef struct model
{
  GLuint      modelList;    // Lista de comandos opengl del model
  GLuint*     textureIDs;   // Lista de los ID's de las texturas
  MATERIAL*   materials;    // Lista de materiales
  PROPERTIES* properties;   // Material Properties
  VECTOR*     vertexBuffer; // Buffer de Vértices
  GLuint      vertexCount;  // Número de Vértices
  GLuint*     indexBuffer;  // Buffer de Índices
  GLuint      indexCount;   // Número de Índices
} MODEL;
/*__________*/


//--- Funciones ---//

/*** Función: Renderiza recursivamente un modelo y guarda su geometría ***/
void RenderModel( const struct aiScene* scene,
		  const struct aiNode*  node,
		  struct aiMatrix4x4    matrix,
		  MODEL* modelStruct, GLboolean verbose )
{
  if( verbose )
    printf( "\t\tExecuting Node '%s'\n", node->mName.data );

  /* Transformacion del nodo */
  glPushMatrix();
  glMultTransposeMatrixf( (GLfloat*)&node->mTransformation );

  struct aiMatrix4x4 transformation = matrix;
  aiMultiplyMatrix4( &transformation, &node->mTransformation );

  /* Meshes */
  unsigned int i;
  for( i = 0; i < node->mNumMeshes; i++ )
    {
      const struct aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      unsigned int matIndex = mesh->mMaterialIndex;

      if( verbose )
	printf( "\t\t\tRendering Mesh No.%d - '%s'\n", i, mesh->mName.data );     

      /* Propiedades */
      if( mesh->mNormals == NULL )
	glDisable( GL_LIGHTING );
      else
	glEnable( GL_LIGHTING );
      if( mesh->mTextureCoords == NULL )
	glDisable( GL_TEXTURE_2D );
      else
	glEnable( GL_TEXTURE_2D );
      if( modelStruct->properties[matIndex].wireframe )
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      else
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      if( modelStruct->properties[matIndex].culling )
	glEnable( GL_CULL_FACE );
      else
	glDisable( GL_CULL_FACE );
       if( modelStruct->properties[matIndex].flat )
	 glShadeModel( GL_FLAT );
      else
	glShadeModel( GL_SMOOTH );
      if( modelStruct->properties[matIndex].transparency )
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
      else
	glBlendFunc( GL_ONE, GL_ONE );
      if( modelStruct->properties[matIndex].blending )
	glEnable( GL_BLEND );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		       modelStruct->properties[matIndex].texOp );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		       modelStruct->properties[matIndex].texOp );

      /* Material */
      SetMaterial( &modelStruct->materials[matIndex] );

      /* Textura */
      glBindTexture( GL_TEXTURE_2D,
		     modelStruct->textureIDs[matIndex] );
      /* Caras */
      unsigned int j;
      for( j = 0; j < mesh->mNumFaces; j++ )
	{
	  /* J-th Face */
	  const struct aiFace* face = &mesh->mFaces[j];
	  switch( face->mNumIndices )
	    {
	    case 1 : glBegin( GL_POINTS )   ; break;
	    case 2 : glBegin( GL_LINES )    ; break;
	    case 3 : glBegin( GL_TRIANGLES ); break;
	    default: glBegin( GL_POLYGON )  ; break;
	    }

	  modelStruct->indexBuffer =
	    realloc( modelStruct->indexBuffer,
		     sizeof(GLuint) *
		     (modelStruct->indexCount + face->mNumIndices) );
	  
	  /* Índices */
	  unsigned int k;
	  for( k = 0; k < face->mNumIndices; k++ )
	    {
	      /* Geometría */
	      modelStruct->indexBuffer[modelStruct->indexCount + k] = 
		modelStruct->vertexCount + face->mIndices[k];
	      
	      /* Color */
	      if( mesh->mColors[0] != NULL )
		glColor4fv( (GLfloat*)&mesh->mColors[0][face->mIndices[k]] );
	      /* Tex Coord */
	      if( mesh->mTextureCoords[0] != NULL )
		glTexCoord2fv( (GLfloat*)&mesh->mTextureCoords[0][face->mIndices[k]] );
	      /* Normal */
	      if( mesh->mNormals != NULL )
		glNormal3fv( (GLfloat*)&mesh->mNormals[face->mIndices[k]] );
	      /* Vertex */
	      glVertex3fv( (GLfloat*)&mesh->mVertices[face->mIndices[k]] );
	    }
	  glEnd();

	  modelStruct->indexCount += face->mNumIndices;
	}
      
      if( modelStruct->properties[matIndex].blending )
	glDisable( GL_BLEND );

       /* Geometría */
      modelStruct->vertexBuffer =
	realloc( modelStruct->vertexBuffer,
		 sizeof(VECTOR) *
		 (modelStruct->vertexCount + mesh->mNumVertices) );
      for( j = 0; j < mesh->mNumVertices; j++ )
	{
	  struct aiVector3D vertex = mesh->mVertices[j];
	  aiTransformVecByMatrix4( &vertex, &transformation );
	  VECTOR v = { vertex.x, vertex.y, vertex.z };
	  modelStruct->vertexBuffer[modelStruct->vertexCount + j] = v;
	}

      modelStruct->vertexCount += mesh->mNumVertices;
    }


  /* Nodos hijos */
  for( i = 0; i < node->mNumChildren; i++ )
    RenderModel( scene, node->mChildren[i], transformation, 
		 modelStruct, verbose );

  glPopMatrix(); // Restauro Cámara
}

/*** Función: Carga texturas, propiedades y materiales ***/
void LoadMaterials( const struct aiScene* scene,
		    const char* texturePath,
		    MODEL*      modelStruct,
		    GLboolean   verbose )
{
/* Cargo las texturas y materiales */
  modelStruct->materials  = calloc( scene->mNumMaterials, sizeof(MATERIAL) );
  modelStruct->properties = calloc( scene->mNumMaterials, sizeof(PROPERTIES) );
  modelStruct->textureIDs = calloc( scene->mNumMaterials, sizeof(GLuint) );
  unsigned int i;
  for( i = 0; i < scene->mNumMaterials; i++ )
    {
      /*** Material ***/
      MATERIAL mat = { {0.2f, 0.2f, 0.2f, 1.0f},
		       {0.8f, 0.8f, 0.8f, 1.0f}, 
		       {0.0f, 0.0f, 0.0f, 1.0f},
		       {0.0f, 0.0f, 0.0f, 1.0f},
		       0.0f };
      struct aiString    materialName;
      struct aiMaterial* material;
      struct aiColor4D   ambient, diffuse, specular, emissive;
      GLfloat            shininess, strength;
      unsigned int       size, stat;

      material             = scene->mMaterials[i];
      materialName.length  = 0;
      materialName.data[0] = '\0';
      
      aiGetMaterialString( material, AI_MATKEY_NAME, &materialName );
      if( verbose )
	printf( "\tLoading Material No.%d - '%s'\n", i, materialName.data );

      
      stat = aiGetMaterialColor( material, AI_MATKEY_COLOR_AMBIENT , &ambient );
      if( stat == AI_SUCCESS )
	{
	  COLOR amb    = {ambient.r, ambient.g, ambient.b, ambient.a};
	  mat.ambient  = amb;
	}
      stat = aiGetMaterialColor( material, AI_MATKEY_COLOR_DIFFUSE , &diffuse );
      if( stat == AI_SUCCESS )
	{
	  COLOR diff   = {diffuse.r, diffuse.g, diffuse.b, diffuse.a};
	  mat.diffuse  = diff;
	}
      stat = aiGetMaterialColor( material, AI_MATKEY_COLOR_SPECULAR, &specular );
      if( stat == AI_SUCCESS )
	{
	  COLOR spec   = {specular.r, specular.g, specular.b, specular.a};
	  mat.specular = spec;
	}
      stat = aiGetMaterialColor( material, AI_MATKEY_COLOR_EMISSIVE, &emissive );
      if( stat == AI_SUCCESS )
	{
	  COLOR emm   = {emissive.r, emissive.g, emissive.b, emissive.a};
	  mat.emission = emm;
	}
      
      size = 1;
      stat = aiGetMaterialFloatArray( material, AI_MATKEY_SHININESS,
				      (float*)&shininess, &size );
      if( stat == AI_SUCCESS )
	{
	  size = 1;
	  stat = aiGetMaterialFloatArray( material, AI_MATKEY_SHININESS_STRENGTH,
					  (float*)&strength, &size );
	  if( stat == AI_SUCCESS )
	    {
	      mat.shininess = shininess * strength;
	    }
	  else
	    mat.shininess = shininess;
	}

      modelStruct->materials[i] = mat;
      /*________*/

      /*** Propiedades ***/
      int   wireframe, culling, transparency, shading;
      float opacity;
      size = 1;
      stat = aiGetMaterialIntegerArray( material, AI_MATKEY_ENABLE_WIREFRAME, 
					&wireframe, &size );
      if( stat == AI_SUCCESS )
	modelStruct->properties[i].wireframe = wireframe;
      size = 1;
      stat = aiGetMaterialIntegerArray( material, AI_MATKEY_TWOSIDED,
				      &culling, &size );
      if( stat == AI_SUCCESS )
	modelStruct->properties[i].culling = !culling;
      size = 1;
      stat = aiGetMaterialIntegerArray( material, AI_MATKEY_SHADING_MODEL,
				      &shading, &size );
      if( stat == AI_SUCCESS )
	{
	  if( shading == aiShadingMode_Flat )
	    modelStruct->properties[i].flat = GL_TRUE;
	  else
	    modelStruct->properties[i].flat = GL_FALSE;
	}
      size = 1;
      stat = aiGetMaterialIntegerArray( material, AI_MATKEY_BLEND_FUNC,
					&transparency, &size );
      if( stat == AI_SUCCESS )
	{
	  if( transparency == aiBlendMode_Default )
	    modelStruct->properties[i].transparency = GL_TRUE;
	  else
	    modelStruct->properties[i].transparency = GL_FALSE;
	}
      size = 1;
      stat = aiGetMaterialFloatArray( material, AI_MATKEY_OPACITY,
				      &opacity, &size );
      if( stat == AI_SUCCESS )
	{
	  if( opacity != 1.0f )
	    modelStruct->properties[i].blending = GL_TRUE;
	  else
	    modelStruct->properties[i].blending = GL_FALSE;
	}
      /*_________*/

      /*** Textura ***/
      int texOp;
      struct aiString textureFile, fileName;
      textureFile.length = 0; textureFile.data[0] = '\0';
      fileName.length    = 0; fileName.data[0]    = '\0';
      aiGetMaterialString( material, 
			   AI_MATKEY_TEXTURE( aiTextureType_DIFFUSE, 0 ),
			   &textureFile );
      size = 1;
      aiGetMaterialIntegerArray( material,
				 AI_MATKEY_MAPPING( aiTextureType_DIFFUSE, 0 ),
				 &texOp, &size );
      switch( texOp )
	{
	case aiTextureMapMode_Wrap:
	  modelStruct->properties[i].texOp = GL_REPEAT;
	  break;
	case aiTextureMapMode_Mirror:
	  modelStruct->properties[i].texOp = GL_MIRRORED_REPEAT;
	  break;
	case aiTextureMapMode_Clamp:
	  modelStruct->properties[i].texOp = GL_CLAMP_TO_EDGE;
	  break;
	case aiTextureMapMode_Decal:
	  modelStruct->properties[i].texOp = GL_CLAMP_TO_BORDER;
	  break;
	default:
	  modelStruct->properties[i].texOp = GL_REPEAT;
	  break;
	}

      if( textureFile.length != 0 )
	{
	  fileName.length += strlen( texturePath ) + 
	    strlen( "/" ) + textureFile.length;
	  strcat( fileName.data, texturePath );
	  strcat( fileName.data, "/" );
	  strcat( fileName.data, textureFile.data );
	  if( verbose )	  
	    printf( "\tLoading Diffuse Texture '%s'\n",
		    fileName.data );
	  if( access( fileName.data, F_OK ) == 0 )
	    modelStruct->textureIDs[i] = LoadTexture( fileName.data );
	  else
	    fprintf( stderr, "ERROR: File '%s' does not exist.\n", 
		     fileName.data );
	}
      else
	modelStruct->textureIDs[i] = 0;
      /*________*/
    }
}

/*** Función: Carga el modelo del archivo "modelFile" ***/
void LoadModel( const char* modelFile,
		const char* texturePath,
		GLboolean   verbose,
		MODEL*      modelStruct )
{
  /* Cargo el modelo */
  if( verbose )
    printf( "Loading model '%s':\n", modelFile );
  const struct aiScene* scene;
  scene = aiImportFile( modelFile,
			aiProcess_OptimizeGraph            |
			aiProcess_RemoveComponent          |
			aiProcess_CalcTangentSpace         |
			aiProcess_JoinIdenticalVertices    |
			aiProcess_FixInfacingNormals       |
			aiProcess_GenSmoothNormals         |
			aiProcess_RemoveRedundantMaterials |
			aiProcess_SortByPType              |
			aiProcess_FlipWindingOrder         );
  if( scene == NULL )
    {
      PrintError( aiGetErrorString(), GL_FALSE );
      return;
    }

  /* Contenido del modelo */
  if( verbose )
    {
      printf( "\t%d Meshes\n"   , scene->mNumMeshes );
      printf( "\t%d Materials\n", scene->mNumMaterials );
      printf( "\t%d Textures\n" , scene->mNumTextures );
    }

  /* Materiales */
  modelStruct->textureIDs = NULL;
  modelStruct->materials  = NULL;
  modelStruct->properties = NULL;
  LoadMaterials( scene, texturePath, modelStruct, verbose );

  struct aiMatrix4x4 matrix;
  aiIdentityMatrix4( &matrix );
  modelStruct->vertexBuffer = NULL;
  modelStruct->vertexCount  = 0;
  modelStruct->indexBuffer  = NULL;
  modelStruct->indexCount   = 0;

  /* Creo la lista de ejecución */
  if( verbose )
    printf( "\tCreating execution list...\n" );
  modelStruct->modelList = glGenLists( 1 );

  /*** Renderización ***/
  glPushAttrib( GL_TRANSFORM_BIT );
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glNewList( modelStruct->modelList, GL_COMPILE );
  glPushAttrib( GL_ENABLE_BIT   |
		GL_TEXTURE_BIT  |
		GL_LIGHTING_BIT |
		GL_COLOR_BUFFER_BIT );
  RenderModel( scene, scene->mRootNode, matrix, modelStruct, verbose );
  glPopAttrib();
  glEndList();
  glPopMatrix();
  glPopAttrib();
  /*_________*/  
  
   /* Libero el modelo */
  aiReleaseImport( scene );
  if( verbose )
    printf( "Done!\n");
}
  
/*** Función: Libera los recursos asociados con un modelo ***/
void FreeModel( MODEL* modelStruct )
{
  glDeleteTextures( sizeof(modelStruct->textureIDs) / sizeof(GLuint),
		    modelStruct->textureIDs );
  glDeleteLists( modelStruct->modelList, 1 );
  free( modelStruct->textureIDs );
  free( modelStruct->materials );
  free( modelStruct->vertexBuffer );
  free( modelStruct->indexBuffer );
}

/*_______*/
