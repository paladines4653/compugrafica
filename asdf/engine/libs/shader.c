/****************************/
/**       ----------       **/
/**        shader.c        **/
/**       ----------       **/
/**  Interfaz para crear   **/
/**  shaders en GLSL       **/
/****************************/


//---   Funciones   ---//

/*** Función: Crea y retorna un shader a partir de un archivo ***/
// type = GL_VERTEX_SHADER ó GL_FRAGMENT_SHADER
GLuint CreateShader( const char* shaderSource, GLenum type )
{
    /* Lee todo el archivos en una cadena */
    char* source = NULL;
    int   length = 0;
    FILE* file   = fopen( shaderSource, "r" );
    fseek( file, 0, SEEK_END );
    length = ftell( file );
    rewind( file );
    source = malloc( sizeof(char) * (length + 1) );
    fread( source, sizeof(char), length, file );
    source[length] = '\0';
    fclose( file );

    /* Crea el shader */
    GLuint shader;
    shader = glCreateShader( type );
    glShaderSource( shader, 1, (const GLchar**)&source, NULL );
    free( source );
    
    /* Compila el shader */
    GLint status;
    glCompileShader( shader );
    glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
    if( status != GL_TRUE )
    {
	GLint   errorLength = 0;
	GLchar* errorString = NULL;
	glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &errorLength );
	errorString = malloc( sizeof(GLchar) * errorLength );
	glGetShaderInfoLog( shader, errorLength, &errorLength, errorString );
	fprintf( stderr, "Error compiling Shader '%s':\n%s\n", shaderSource, errorString );
	free( errorString );
	return 0;
    }

    return shader;
}
    
/*** Función: Crea un programa en base a un shader listo para ejecutar ***/ 
// nShaders = Número de shaders a incorporar en el programa
GLuint CreateProgram( int nShaders, ... )
{
    GLuint  program;
    GLint   status;
    va_list vl;
    program = glCreateProgram();

    /* Incorporar los shaders */
    va_start( vl, nShaders );
    while( nShaders != 0 )
    {
	glAttachShader( program, va_arg( vl, GLuint ) );
	nShaders--;
    }		
    va_end( vl );
    
    
    /* Linkear programa */
    glLinkProgram( program );
    glGetProgramiv( program, GL_LINK_STATUS, &status );
    if( status != GL_TRUE )
    {
	GLint errorLength = 0;
	GLchar* errorString = NULL;
	glGetProgramiv( program, GL_INFO_LOG_LENGTH, &errorLength );
	errorString = malloc( sizeof(GLchar) * errorLength );
	glGetProgramInfoLog( program, errorLength, &errorLength, errorString );
	fprintf( stderr, "Error linking Program:\n%s\n", errorString );
	free( errorString );
	return 0;
    }
    
    return program;
}
