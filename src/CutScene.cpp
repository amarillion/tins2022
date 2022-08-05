/*
 * CutScene.cpp
 *
 *  Created on: 5 Aug 2012
 *      Author: martijn
 */

#include "CutScene.h"
#include "color.h"
#include <allegro5/allegro.h>
#include "anim.h"
#include <memory>
#include "componentbuilder.h"
#include "DrawStrategy.h"
#include "mainloop.h"

using namespace std;

CutScene::CutScene(int exitCode) : buttonSkip(ALLEGRO_KEY_ESCAPE), exitCode(exitCode)
{
}

void CutScene::addFrame (ComponentPtr comp)
{
	sequence.push_back (comp);
	comp->setFont(sfont);
}

ContainerPtr CutScene::newPage ()
{
	ContainerPtr scene = make_shared<Container>();
	addFrame(scene);
	scene->add(ClearScreen::build(BLACK).get());
	return scene;
}


void CutScene::onFocus()
{
	current = sequence.begin();
	(*current)->doLayout(getx(), gety(), getw(), geth());
}

void CutScene::draw(const GraphicsContext &gc)
{
	if (current != sequence.end())
		(*current)->draw(gc);
}

void CutScene::update()
{
	Component::update();

	if (current != sequence.end())
	{
		int result = 0;

		(*current)->update();
		while ((*current)->hasMsg())
		{
			// any value at all
			result = (*current)->popMsg();
		}

		if (result != 0)
		{
			current++;
			if (current == sequence.end())
			{
				pushMsg(exitCode);
			}
			else
			{
				(*current)->doLayout(getx(), gety(), getw(), geth());
			}
		}

	}

	if (buttonSkip.justPressed())
	{
		pushMsg(exitCode);
		return; // skip animation
	}

	return;
}

