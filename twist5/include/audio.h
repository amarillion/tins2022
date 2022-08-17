#pragma once

#include "data.h"

struct ALLEGRO_SAMPLE;
struct ALLEGRO_CONFIG;
struct ALLEGRO_VOICE;
struct ALLEGRO_AUDIO_STREAM;
struct ALLEGRO_MIXER;

class Resources;

/**
 * Global audio manager
 */
class Audio {
private:
	void propertyChanged();
	bool soundInstalled;
	ALLEGRO_VOICE *voice;
	ALLEGRO_AUDIO_STREAM *currentMusic;
	virtual void updateMusicVolume();	
protected:
	ALLEGRO_MIXER *mixer;
public:
	void getSoundFromConfig(ALLEGRO_CONFIG *config);
	void init();
	Audio();
	virtual ~Audio() {}
	bool isInstalled() { return soundInstalled; }
	void setInstalled(bool value) { soundInstalled = value; }
	void playSample(ALLEGRO_SAMPLE *s);
	
	bool isMusicOn() { return musicVolume.get() > 0; }
	bool isSoundOn() { return soundVolume.get() > 0; }

	RangeModel<float> soundVolume {0.5, 0.0, 1.0};
	RangeModel<float> musicVolume {0.5, 0.0, 1.0};

	void done();
	void playMusic (ALLEGRO_AUDIO_STREAM *duh, float volume = 1.0f);
	virtual void stopMusic();
};
