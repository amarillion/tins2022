#pragma once

#include "data.h"

#ifdef USE_ALSPC
struct ALSPC_DATA;
struct ALSPC_PLAYER;
#endif

struct ALLEGRO_SAMPLE;
struct ALLEGRO_CONFIG;
struct ALLEGRO_VOICE;
struct ALLEGRO_AUDIO_STREAM;
struct ALLEGRO_MIXER;

class Resources;

/**
 * Global sound manager
 */
class Sound
{
protected:
	void getSoundFromConfig(ALLEGRO_CONFIG *config);
	void initSound();
#ifdef USE_ALSPC
	void pollMusic();
#endif
private:
	bool inited;
	void propertyChanged ();
	bool soundInstalled;

	ALLEGRO_VOICE *voice;
	ALLEGRO_AUDIO_STREAM *currentMusic;
	ALLEGRO_MIXER *mixer;
#ifdef USE_ALSPC
    bool stereo;
    bool hifi;
    ALSPC_DATA *currentMusic;
    ALSPC_PLAYER *alspc_player;
#endif
	void updateMusicVolume();
	
public:
	Sound();
	bool isSoundInstalled() { return soundInstalled; }
	void setSoundInstalled(bool value) { soundInstalled = value; }
	void playSample (ALLEGRO_SAMPLE *s);
	
	bool isMusicOn() { return musicVolume.get() > 0; }
	bool isSoundOn() { return soundVolume.get() > 0; }

	RangeModel<float> soundVolume {0.5, 0.0, 1.0};
	RangeModel<float> musicVolume {0.5, 0.0, 1.0};

	void doneSound();
	void playMusic (ALLEGRO_AUDIO_STREAM *duh, float volume = 1.0f);
	void stopMusic ();
	
#ifdef USE_ALSPC
	void playMusic (ALSPC_DATA *alspc_data);
	void stopMusic ();
    bool isStereoOn() { return stereo; }
	void setStereoOn (bool value);
    bool isHifiOn() { return hifi; }
	void setHifiOn (bool value);
#endif
};
