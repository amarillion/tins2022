#pragma once

#include "audio.h"

/**
 * Replacement sound module that supports cross-fade
 */
class CrossFadeAudio : public Audio {
private:
	ALLEGRO_AUDIO_STREAM *musicAbove;
	ALLEGRO_AUDIO_STREAM *musicUnder;
	float pan = 1.0;
public:
	void setPan(float _pan);	
	void playDualMusic (ALLEGRO_AUDIO_STREAM *above, 
		ALLEGRO_AUDIO_STREAM *under, float volume = 1.0f);

	virtual void updateMusicVolume() override;
	virtual void stopMusic() override;
};
