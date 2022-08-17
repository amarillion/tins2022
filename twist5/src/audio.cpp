#include "audio.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include "resources.h"
#include <assert.h>
#include "util.h"
#include <iostream>
#include "mainloop.h"
#include "setting.h"

using namespace std;

Audio::Audio(): soundInstalled(true) {
	currentMusic = nullptr;
	musicVolume.AddListener([=](int) {
		updateMusicVolume();
	});
}

void Audio::init() {
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
}

void Audio::getSoundFromConfig(ALLEGRO_CONFIG *config) {
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

void Audio::playSample (ALLEGRO_SAMPLE *s) {
	if (!(isSoundOn() && isInstalled())) return;
	assert (s);

	bool success = al_play_sample (s, soundVolume.get(), 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
	if (!success) {
		cout << "Could not play sample" << endl; //TODO: log error.
	}
}

void Audio::playMusic (ALLEGRO_AUDIO_STREAM *duh, float volume) {
	if (!isInstalled()) return;
	if (!(isSoundOn() && isMusicOn())) return;
	if (currentMusic) {
		al_detach_audio_stream(currentMusic);
		currentMusic = nullptr;
	}
	if (!al_attach_audio_stream_to_mixer(duh, mixer)) {
	       allegro_message("al_attach_audio_stream_to_mixer failed.\n"); //TODO: log error.
	}
	currentMusic = duh;
	al_set_audio_stream_gain(currentMusic, volume * musicVolume.get());
}

void Audio::updateMusicVolume() {
	if (!isInstalled()) return;

	if (currentMusic) {
		al_set_audio_stream_gain(currentMusic, musicVolume.get());
	}
}
		
void Audio::stopMusic () {
	if (currentMusic) {
		al_detach_audio_stream(currentMusic);
		currentMusic = nullptr;
	}
}

void Audio::done() {
	stopMusic();
}
