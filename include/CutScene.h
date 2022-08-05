/*
 * CutScene.h
 *
 *  Created on: 5 Aug 2012
 *      Author: martijn
 */

#ifndef CUTSCENE_H_
#define CUTSCENE_H_

#include "component.h"
#include <vector>
#include "container.h"
#include "input.h"

class CutScene : public Component {
private:
	Input buttonSkip;
	int exitCode;
	std::vector<ComponentPtr> sequence;
	std::vector<ComponentPtr>::iterator current;
	void addFrame (ComponentPtr comp);
public:
	ContainerPtr newPage ();
	CutScene(int exitCode);
	virtual void onFocus() override;
	virtual void draw(const GraphicsContext &gc) override;
	virtual void update() override;
};

#endif /* CUTSCENE_H_ */
