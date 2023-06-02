#pragma once

#include <functional>
#include <memory>

// forward declarations;
class Resources;
struct ALLEGRO_SHADER;
struct ALLEGRO_BITMAP;

/**
 * Wrapper for a time-dependent shader
 * that produces a dissolve effect
 */
class RippleEffect {

private:
	ALLEGRO_SHADER *shader;
	static std::string fragShaderSource;
	static std::string vertShaderSource;
	static bool inited;
public:
	static void init(std::shared_ptr<Resources> res);
	RippleEffect();
//
//	/**
//	 * time: from start to completely dissolved, between 0.0 and 1.0
//	 * closure: drawing code to execute between enabling and disabling the shader.
//	 */
//	void withPattern(float time, std::function<void()> closure) {
//		enable(time);
//		closure();
//		disable();
//	}

	void enable(float sigma, ALLEGRO_BITMAP *bmp);
	void disable();

	ALLEGRO_SHADER *getShader() { return shader; }
};

