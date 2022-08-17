#include "crossfade.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include "resources.h"
#include <assert.h>
#include "util.h"
#include <iostream>
#include "mainloop.h"
#include "setting.h"

using namespace std;

void CrossFadeAudio::playDualMusic (ALLEGRO_AUDIO_STREAM *above, 
	ALLEGRO_AUDIO_STREAM *under, float _pan)
{
	if (!isInstalled()) return;
	if (!(isSoundOn() && isMusicOn())) return;
	
	stopMusic();
	
	if (!al_attach_audio_stream_to_mixer(above, mixer)) {
	allegro_message("al_attach_audio_stream_to_mixer failed.\n"); //TODO: log error.
	}
	if (!al_attach_audio_stream_to_mixer(under, mixer)) {
	allegro_message("al_attach_audio_stream_to_mixer failed.\n"); //TODO: log error.
	}
	musicAbove = above;
	musicUnder = under;
	pan = _pan;
	al_set_audio_stream_gain(musicAbove, pan * musicVolume.get());
	al_set_audio_stream_gain(musicUnder, (1.0 - pan) * musicVolume.get());
}

void CrossFadeAudio::updateMusicVolume() {
	if (!isInstalled()) return;

	if (musicAbove && musicUnder) {
		al_set_audio_stream_gain(musicAbove, pan * musicVolume.get());
		al_set_audio_stream_gain(musicUnder, (1.0 - pan) * musicVolume.get());
	}
}

void CrossFadeAudio::setPan(float _pan) {
	pan = _pan;
	updateMusicVolume();
}

void CrossFadeAudio::stopMusic () {
	if (musicAbove) {
		al_detach_audio_stream(musicAbove);
		musicAbove = nullptr;
	}
	if (musicUnder) {
		al_detach_audio_stream(musicUnder);
		musicUnder = nullptr;
	}
}
