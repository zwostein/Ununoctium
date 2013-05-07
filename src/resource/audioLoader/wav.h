#ifndef RESOURCE_AUDIOLOADER_WAV_INCLUDED
#define RESOURCE_AUDIOLOADER_WAV_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


#include <AL/al.h>


int audioLoader_WAV( const char * filename, ALuint * buffer, ALsizei * frequency, ALenum * format );


#ifdef __cplusplus
}
#endif

#endif
