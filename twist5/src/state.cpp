#include "state.h"

bool State::onHandleMessage(ComponentPtr src, int msg)
{
	switch (msg)
	{
	case MSG_UNFOCUS:
		awake = false;
		setVisible(false);
		return true;
	}

	return false;
}

void State::onFocus() {
	awake = true;
	setVisible(true);
}
