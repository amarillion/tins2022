#include "sound.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include "resources.h"
#include <assert.h>
#include "util.h"
#include <iostream>
#include "mainloop.h"
#include "setting.h"

#ifdef USE_ALSPC
#include <alspc.h>
#endif

using namespace std;

Sound::Sound(): inited(false), soundInstalled(true)
{
	musicVolume.AddListener([=](int) {
		updateMusicVolume();
	});
}

void Sound::initSound()
{
#ifdef USE_ALSPC
    hifi = true;
    stereo = true;
    currentMusic = NULL;
    alspc_install();
    alspc_player = NULL;
#endif

	voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
	if (!voice) {
		allegro_message("Could not create ALLEGRO_VOICE.\n");	//TODO: log error.
	}

	mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32,
			ALLEGRO_CHANNEL_CONF_2);
	if (!mixer) {
		allegro_message("Could not create ALLEGRO_MIXER.\n");	//TODO: log error.
	}

	if (!al_attach_mixer_to_voice(mixer, voice)) {
		allegro_message("al_attach_mixer_to_voice failed.\n");	//TODO: log error.
	}

	inited = true;
}

void Sound::getSoundFromConfig(ALLEGRO_CONFIG *config)
{
	musicVolume.set(
		get_config(Tag<float>{}, config, "twist", "musicVolume", musicVolume.get())
	);
	musicVolume.AddListener([=](int) {
		set_config(Tag<float>{}, config, "twist", "musicVolume", musicVolume.get());
	});

	soundVolume.set(
		get_config(Tag<float>{}, config, "twist", "soundVolume", soundVolume.get())
	);
	soundVolume.AddListener([=](int){
		set_config(Tag<float>{}, config, "twist", "soundVolume", soundVolume.get());
	});

}

void Sound::playSample (ALLEGRO_SAMPLE *s)
{
	if (!(isSoundOn() && isSoundInstalled())) return;
	assert (s);

	bool success = al_play_sample (s, soundVolume.get(), 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
	if (!success) {
		cout << "Could not play sample" << endl; //TODO: log error.
	}
}

void Sound::playMusic (ALLEGRO_AUDIO_STREAM *above, 
	ALLEGRO_AUDIO_STREAM *under, float _pan)
{
	if (!isSoundInstalled()) return;
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

void Sound::updateMusicVolume() {
	if (!isSoundInstalled()) return;

	if (musicAbove && musicUnder) {
		al_set_audio_stream_gain(musicAbove, pan * musicVolume.get());
		al_set_audio_stream_gain(musicUnder, (1.0 - pan) * musicVolume.get());
	}
}

void Sound::setPan(float _pan) {
	pan = _pan;
	updateMusicVolume();
}

void Sound::stopMusic ()
{
	if (musicAbove) {
		al_detach_audio_stream(musicAbove);
		musicAbove = nullptr;
	}
	if (musicUnder) {
		al_detach_audio_stream(musicUnder);
		musicUnder = nullptr;
	}
}

void Sound::doneSound()
{
	stopMusic();
}

