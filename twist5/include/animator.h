#pragma once

#include "easing.h"
#include "updateable.h"

template<typename T>
class Animator : public Updateable {
	const EasingFuncPtr f;
	T src;
	T dest;
	std::function<void(T)> setter;
	std::function<void()> onFinished;
	int totalSteps;
	int currentStep;
public:
	Animator (
		const T src, const T dest, 
		int steps,
		std::function<void(T)> setter,
		const EasingFuncPtr f = linear
	) : f(f), src(src), dest(dest), setter(setter), onFinished(nullptr), totalSteps(steps), currentStep(0) {
	}

	Animator (
		const T src, const T dest, 
		int steps,
		std::function<void(T)> setter,
		std::function<void()> onFinished,
		const EasingFuncPtr f = linear
	) : f(f), src(src), dest(dest), setter(setter), onFinished(onFinished), totalSteps(steps), currentStep(0) {
		// make sure we start right
		setter(src);
	}


	virtual void update() override {
		if (!isAlive()) return;

		// interpolate...
		double delta = (double)currentStep / (double)totalSteps;

		// apply easing func
		double ease = f(delta);
		T newp = src + ((dest - src) * ease);
		setter(newp);

		currentStep++;

		if (currentStep > totalSteps) {
			if (onFinished) {
				onFinished();
			}
			kill();
		}
	}
};
