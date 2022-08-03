#include "depthshader.h"
#include <allegro5/allegro.h>
#include <stdio.h>

DepthShader::DepthShader()
{
	shader = al_create_shader(ALLEGRO_SHADER_AUTO);
	assert(shader);

	static const char *discarding_glsl_pixel_source =
	   "#ifdef GL_ES\n"
	   "precision lowp float;\n"
	   "#endif\n"
	   "uniform sampler2D " ALLEGRO_SHADER_VAR_TEX ";\n"
	   "uniform bool " ALLEGRO_SHADER_VAR_USE_TEX ";\n"
	   "varying vec4 varying_color;\n"
	   "varying vec2 varying_texcoord;\n"
	   "void main()\n"
	   "{\n"
	   "  if (" ALLEGRO_SHADER_VAR_USE_TEX ") {\n"
	   "    vec4 p = texture2D(" ALLEGRO_SHADER_VAR_TEX ", varying_texcoord);\n"
	   "    if (p.a == 0.0) {\n"
	   "        discard;\n"
	   "    }\n"
	   "    gl_FragColor = varying_color * p;\n"
	   "  }\n"
	   "  else\n"
	   "    gl_FragColor = varying_color;\n"
	   "}\n";

	bool ok;

	ok = al_attach_shader_source(shader, ALLEGRO_PIXEL_SHADER, discarding_glsl_pixel_source);
	//TODO: assert with message format...
	if (!ok) printf ("al_attach_shader_source failed: %s\n", al_get_shader_log(shader));
	assert(ok);

	ok = al_attach_shader_source(shader, ALLEGRO_VERTEX_SHADER,
			al_get_default_shader_source(ALLEGRO_SHADER_GLSL, ALLEGRO_VERTEX_SHADER));

	//TODO: assert with message format...
	if (!ok) printf ("al_attach_shader_source failed: %s\n", al_get_shader_log(shader));
	assert(ok);

	ok = al_build_shader(shader);
	//TODO: assert with message...
	if (!ok) printf ("al_build_shader failed: %s\n", al_get_shader_log(shader));
	assert(ok);
}

void DepthShader::enable() {
	al_use_shader(shader);
}

void DepthShader::disable() {
	al_use_shader(NULL);
}
