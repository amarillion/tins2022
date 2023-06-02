#include "ripple.h"

#include <allegro5/allegro.h>
#include "mainloop.h"
#include "resources.h"

using namespace std;

RippleEffect::RippleEffect()
{
	assert(inited);

	shader = al_create_shader(ALLEGRO_SHADER_AUTO);
	assert(shader);

	bool ok;

	ok = al_attach_shader_source(shader, ALLEGRO_PIXEL_SHADER, fragShaderSource.c_str());
	//TODO: assert with message format...
	if (!ok) printf ("al_attach_shader_source failed: %s\n", al_get_shader_log(shader));
	assert(ok);

	ok = al_attach_shader_source(shader, ALLEGRO_VERTEX_SHADER, vertShaderSource.c_str());

	//TODO: assert with message format...
	if (!ok) printf ("al_attach_shader_source failed: %s\n", al_get_shader_log(shader));
	assert(ok);

	ok = al_build_shader(shader);
	//TODO: assert with message...
	if (!ok) printf ("al_build_shader failed: %s\n", al_get_shader_log(shader));
	assert(ok);
}

void RippleEffect::enable(float time, ALLEGRO_BITMAP *bmp) {
	al_use_shader(shader);

	al_set_shader_sampler("textureMap", bmp, 1);
	int texSize[2] = {
			al_get_bitmap_width(bmp),
			al_get_bitmap_height(bmp)
	};
//	al_set_shader_int_vector("u_texture_size", 2, texSize, 1);
	al_set_shader_float("time", time);
//	int dir[2] = { 0, 1 };
//	al_set_shader_int_vector("u_dir", 2, dir, 1);
}

void RippleEffect::disable() {
	al_use_shader(NULL);
}

void RippleEffect::init(std::shared_ptr<Resources> resources) {
	fragShaderSource = resources->getTextFile("twirl_frag");
	vertShaderSource = resources->getTextFile("twirl_vert");
	inited = true;
}

string RippleEffect::fragShaderSource;
string RippleEffect::vertShaderSource;
bool RippleEffect::inited = false;
