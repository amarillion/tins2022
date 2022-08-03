#pragma once

#include <functional>

// forward declaration;
struct ALLEGRO_SHADER;

/**
 * Wrapper for a simple shader that renders sprites properly icm depth buffer.
 */
class DepthShader {

private:
	ALLEGRO_SHADER *shader;

public:
	DepthShader();

	void enable();
	void disable();

	ALLEGRO_SHADER *getShader() { return shader; }
};
