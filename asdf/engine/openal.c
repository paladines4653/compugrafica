/*****************************/
/**       ----------        **/
/**        openal.c         **/
/**       ----------        **/
/**  Rutinas necesarias pa  **/
/**  ra inicializar openAL  **/
/*****************************/

//---   Definiciones  ---//

// 16 buffers, 64Kbs, 1Mb total
#define NUM_BUFFERS 16
#define BUFFER_SIZE 64*1024

/*______*/


//---   Estructuras   ---//

/*** Estructura de dato: SOUND ***/
typedef struct sound
{
    ALuint  audioBuffer;
    ALuint  audioSource;
    ALfloat audioPosition[3];
    ALfloat audioVelocity[3];
} SOUND;

/*** Estructura de dato: MUSIC ***/
typedef struct music
{
    OggVorbis_File oggFile;
    ALuint         musicBuffers[NUM_BUFFERS];
    ALuint         musicSource;
    ALenum         musicFormat;
    long           musicFreq;
    ALboolean      loop;
} MUSIC;

/*_______*/


//---   Funciones ---//

/*** Función: Inicializa el sistema de sonido ***/
// userDevice = dispositivo de sonido a abrir(NULL=default)
int InitOpenAL( char* userDevice )
{
    // Dispositivo de sonido por defecto
    const ALCchar* defaultDevice;
    // Dispositivo abierto
    ALCdevice*     device;
    // Contexto
    ALCcontext*    context;

    // Averiguo el dispositivo de sonido por defecto
    defaultDevice = alcGetString( NULL, ALC_DEFAULT_DEVICE_SPECIFIER );
    
    // Abro el dispositivo de sonido
    if( userDevice != NULL )
	device = alcOpenDevice( userDevice );
    else
	device = alcOpenDevice( defaultDevice );
    // Comprobación
    if( device == NULL )
    {
	fprintf( stderr, "Error abriendo dispositivo de sonido: %s\n", userDevice ? userDevice : defaultDevice );
	return 0;
    }

    // Creación del contexto
    context = alcCreateContext( device, NULL );
    if( context == NULL )
    {
	fprintf( stderr, "Error creando contexto de sonido en: %s\n", userDevice ? userDevice : defaultDevice );
	return 0;
    }

    // Inicializo el sonido
    if( alcMakeContextCurrent( context ) == AL_FALSE )
    {
	fprintf( stderr, "Error inicializando sonido en: %s\n", userDevice ? userDevice : defaultDevice );
	return 0;
    }

    // Modelo de atenuación de los sonidos
    alDistanceModel( AL_INVERSE_DISTANCE_CLAMPED );

    return 1;
}

/*** Función: Libero el sistema de sonido ***/
void FreeOpenAL()
{
    ALCcontext* context;
    ALCdevice*  device;

    // Cierro OpenAL
    context = alcGetCurrentContext();          // Contexto Actual
    device  = alcGetContextsDevice( context ); // Dispositivo del contexto actual
    alcMakeContextCurrent( NULL );             // Desato el contexto
    alcDestroyContext( context );              // Libero el contexto
    alcCloseDevice( device );                  // Cierro el dispositivo
}

/*** Función: Caracterísitcas del auditor ***/
void SetListener( ALfloat* pos, ALfloat* look, ALfloat* up, ALfloat* vel )
{
    ALfloat orientation[] = { look[0], look[1], look[2],
			      up[0]  , up[1]  , up[2] };

    // Posición, velocidad y orientación
    alListenerfv( AL_POSITION, pos );
    alListenerfv( AL_VELOCITY, vel );
    alListenerfv( AL_ORIENTATION, orientation );
}

/*** Función: Carga un sonido para ser reproducido(solo WAV) ***/
void CreateSound( SOUND* sound, char* soundFile )
{
    // SDL
    SDL_AudioSpec spec;
    Uint32        length;
    Uint8*        buffer;

    // Cargo el Sonido
    SDL_LoadWAV( soundFile, &spec, &buffer, &length );
    
    // Encuentro el formato
    ALenum format;
    switch( spec.format )
    {
	case AUDIO_U8: case AUDIO_S8:
	    if( spec.channels == 1 )
		format = AL_FORMAT_MONO8;
	    else if( spec.channels == 2 )
		format = AL_FORMAT_STEREO8;
	    break;

	case AUDIO_U16: case AUDIO_S16:
	    if( spec.channels == 1 )
		format = AL_FORMAT_MONO16;
	    else if( spec.channels == 2 )
		format = AL_FORMAT_STEREO16;
	    break;
    }

    // Creo el buffer
    alGenBuffers( 1, &(sound->audioBuffer) );

    // Cargo el sonido en la tarjeta de sonido	    
    alBufferData( sound->audioBuffer, format, (ALvoid*)buffer, (ALsizei)length, (ALsizei)spec.freq );

    // Libero el sonido de memoria
    SDL_FreeWAV( buffer );

    // Creo la fuente del sonido
    alGenSources( 1, &(sound->audioSource) );
    // Propiedades del sonido
    alSourcei( sound->audioSource, AL_BUFFER , sound->audioBuffer );
    alSourcef( sound->audioSource, AL_PITCH  , 1.0f );
    alSourcef( sound->audioSource, AL_ROLLOFF_FACTOR, 0.05f ); //<--- A mano la atenuación

    sound->audioPosition[0] = 0.0f; sound->audioPosition[1] = 0.0f; sound->audioPosition[2] = 0.0f;
    sound->audioVelocity[0] = 0.0f; sound->audioVelocity[1] = 0.0f; sound->audioVelocity[1] = 0.0f;
}

/*** Función: Libera un sonido ***/
void FreeSound( SOUND* sound )
{
    alDeleteBuffers( 1, &(sound->audioBuffer) );
    alDeleteSources( 1, &(sound->audioSource) );
}

/*** Función: Propiedades del sonido ***/
void SetSoundProperties( SOUND* sound )
{
    alSourcefv( sound->audioSource, AL_POSITION, sound->audioPosition );
    alSourcefv( sound->audioSource, AL_VELOCITY, sound->audioVelocity );
    alSourcei( sound->audioSource , AL_SOURCE_RELATIVE, AL_FALSE );
}

/*** Función: Ejecuta un sonido ***/
// volume = volumen relativo(1.0=normal)
// loop   = TRUE: Se repite, FALSE: No se repite
void PlaySound( SOUND* sound, float volume, ALboolean loop )
{
    // Volumen, loop
    alSourcei( sound->audioSource , AL_LOOPING, loop );
    alSourcef( sound->audioSource , AL_GAIN, volume );   

    // Play
    alSourcePlay( sound->audioSource );
}

/*** Función: Pausa un sonido ***/
void PauseSound( SOUND* sound )
{
    alSourcePause( sound->audioSource );
}

/*** Función: Para un sonido ***/
void StopSound( SOUND* sound )
{
    alSourceStop( sound->audioSource );
}

/*** Función: Carga una canción para ser reproducida(solo OGG) ***/
void CreateMusic( MUSIC* music, char* musicFile )
{
    vorbis_info* info;
    FILE* file;

    // Abro el archivo ogg
    file = fopen( musicFile, "rb" );
    ov_open( file, &(music->oggFile), NULL, 0 );
    info = ov_info( &(music->oggFile), -1 );

    // Encuentro el formato y la frecuencia
    music->musicFreq = info->rate;
    switch( info->channels )
    {
	case 1:
	    music->musicFormat = AL_FORMAT_MONO16;
	    break;
	case 2:
	    music->musicFormat = AL_FORMAT_STEREO16;
	    break;
    }

    // Genero los buffers
    alGenBuffers( NUM_BUFFERS, music->musicBuffers );
    alGenSources( 1, &(music->musicSource ) );

    // Propiedades
    alSource3f( music->musicSource, AL_POSITION, 0.0f, 0.0f, 0.0f );
    alSource3f( music->musicSource, AL_VELOCITY, 0.0f, 0.0f, 0.0f );
    alSource3f( music->musicSource, AL_DIRECTION, 0.0f, 0.0f, 0.0f );
    alSourcef( music->musicSource , AL_PITCH  , 1.0f );
    alSourcef( music->musicSource , AL_ROLLOFF_FACTOR, 0.0f );
    alSourcei( music->musicSource , AL_SOURCE_RELATIVE, AL_TRUE );

    // Cargos los primeros buffers
    int nBuffer = 0;
    while( nBuffer != NUM_BUFFERS )
    {
	int bytesRead, bitStream;
	char buffer[BUFFER_SIZE];
	// Leo el archivo OGG
	bytesRead = ov_read( &(music->oggFile),
			     buffer,      // Buffer[OUT]
			     BUFFER_SIZE, // Tamaño del buffer a leer
			     0,           // (0=Little Endian, 1=Big Endian)
			     2,           // (1=8bits, 2=16bits)
			     1,           // (0=unsigned, 1=signed)
			     &bitStream );
	// Pongo el buffer en la tarjeta de sonido
	alBufferData( music->musicBuffers[nBuffer], music->musicFormat, (ALvoid*)buffer, (ALsizei)bytesRead, (ALsizei)music->musicFreq );
	nBuffer++;
    }
    
    // Queue Buffers
    alSourceQueueBuffers( music->musicSource, NUM_BUFFERS, music->musicBuffers );
}

/*** Función: Libera la música ***/
void FreeMusic( MUSIC* music )
{
    // Verifico que el sonido esté parado
    ALint state;
    alGetSourcei( music->musicSource, AL_SOURCE_STATE, &state );
    if( state == AL_PLAYING )
	alSourceStop( music->musicSource );
	
    // Descargo los buffers y libero recursos
    alSourceUnqueueBuffers( music->musicSource, NUM_BUFFERS, music->musicBuffers );
    alDeleteBuffers( NUM_BUFFERS, music->musicBuffers );

    // Libero el archivo ogg
    ov_clear( &(music->oggFile) );
}

/*** Funcion: Ejecuta la música ***/
void PlayMusic( MUSIC* music, float volume, ALfloat loop )
{
    // Propiedades
    music->loop = loop;
    alSourcef( music->musicSource, AL_GAIN, volume );
    alSourcef( music->musicSource, AL_MIN_GAIN, volume );
    alSourcef( music->musicSource, AL_MAX_GAIN, volume );

    // Play
    alSourcePlay( music->musicSource );
}

/*** Función: Pausa la música ***/
void PauseMusic( MUSIC* music )
{
    alSourcePause( music->musicSource );
}

/*** Funcion: Para la música ***/
void StopMusic( MUSIC* music )
{
    alSourceStop( music->musicSource );
}

/*** Función: Streaming de la música ***/
void StreamMusic( MUSIC* music )
{
    int   nBuffers  = 0;
    int   bytesRead = 1;
    ALint state;

    // Obtengo cuantos buffers se usaron
    alGetSourcei( music->musicSource, AL_BUFFERS_PROCESSED, &nBuffers );

    // Los re-uso
    int    bitStream;
    ALuint musicBuffer; // Used buffer
    char   buffer[BUFFER_SIZE];
    while( nBuffers != 0 )
    {
	// Leo el archivo OGG
	bytesRead = ov_read( &(music->oggFile),
			     buffer,      // Buffer[OUT]
			     BUFFER_SIZE, // Tamaño del buffer a leer
			     0,           // (0=Little Endian, 1=Big Endian)
			     2,           // (1=8bits, 2=16bits)
			     1,           // (0=unsigned, 1=signed)
			     &bitStream );
	
	// Quito el buffer y pongo el nuevo
	alSourceUnqueueBuffers( music->musicSource, 1, &musicBuffer );
	alBufferData( musicBuffer, music->musicFormat, (ALvoid*)buffer, (ALsizei)bytesRead, (ALsizei)music->musicFreq );
	alSourceQueueBuffers( music->musicSource, 1, &musicBuffer );
	nBuffers--;
    }

    // Verifico que el sonido siga reproduciendose
    alGetSourcei( music->musicSource, AL_SOURCE_STATE, &state );
    if( state != AL_PLAYING )
	alSourcePlay( music->musicSource );

    // Verifico si la música acabó
    if( bytesRead == 0 )
	if( music->loop )
	    ov_raw_seek( &(music->oggFile), 0 );
}
