#include "button.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "color.h"
#include <memory>
#include "wrappers.h"
#include <sstream>

using namespace std;

static const int BUTTON_MARGIN = 8; //TODO: add to skin

double Button::getPreferredWidth()
{
	return BUTTON_MARGIN + al_get_text_width(sfont, text.c_str());
}

void Button::doDraw (const GraphicsContext &gc)
{
	Bitmap dst = Bitmap(gc.buffer);

	// get the button state
	int state = isEnabled() ? (isSelected() ? 1 : (hasFocus() ? 3 : 0)) : 2;

	int x1 = getx() + gc.xofst;
	int y1 = gety() + gc.yofst;
	int x2 = x1 + w;
	int y2 = y1 + h;

	int displacement = TestFlag(D_PRESSED) ? GetSkin()->buttonDisplacement : 0;

    // copied from MASkinG
	if (border) {
		border.TiledBlit(dst, 0, state, x1, y1, w, h, 1, 4);
	}

	if (icon) {
		//TODO: state vs icon...
		al_draw_bitmap_region(icon, 0, state * h, w, h, 0, 0, 0);
	}

//	al_draw_filled_rectangle(x1, y1, x2, y2, enabled ? BLACK : GREY);

    al_draw_text (sfont, (isDisabled() ? LIGHT_GREY: DARK_GREY),
        (x2 + x1) / 2 + displacement, (y2 + y1 - al_get_font_line_height(sfont)) / 2 + displacement, ALLEGRO_ALIGN_CENTRE,
		text.c_str());

    if (hasFocus())
    {
    	//TODO
    	// replace with border function
     //   dotted_rect (gc.buffer, x, y, x + w - 1, y + h - 1, BLACK);
    	al_draw_rectangle (x1 + 3, y1 + 3, x2 - 3, y2 - 3, GREEN, 1.0);
    }
}

/**
guibutton message handler
*/
void Button::handleEvent(ALLEGRO_EVENT &event)
{
	switch (event.type)
	{
		case ALLEGRO_EVENT_TWIST_MOUSE_LEAVE_WIDGET:
		{
			if (!isToggleButton())
			{
				if (isSelected()) Deselect();
			}
		}
			break;
		case ALLEGRO_EVENT_TOUCH_BEGIN:
			doAction();
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			if (!isEnabled()) break;
			if (isToggleButton())
			{
				Toggle();
			}
			else
			{
				Select();
			}
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
			if (isSelected())
			{
				doAction();
			}
			if (!isToggleButton())
			{
				Deselect();  //TODO - also when mouse exits... // guarantee exit event
			}
			break;
		case ALLEGRO_EVENT_KEY_CHAR:
		{
			if (!isEnabled()) break;
			switch (event.keyboard.keycode)
			{
				case ALLEGRO_KEY_SPACE: case ALLEGRO_KEY_ENTER:
					if (isToggleButton())
					{
						Toggle();
					}
					doAction();
					break;
			}
			break;
		}
    }
}

void Button::doAction()
{
	if (actionFunc) { actionFunc(); }
	else pushMsg(action);
}

void Button::handleMessage(ComponentPtr src, int msg)
{
	// for component, unfocus means that "awake" and "visible" are set to false. -> This should be the meaning for Layer...
	// for Button, focus means that component is highlighted.
	switch (msg)
	{
	case MSG_FOCUS:
		setFocus(true);
		break;
	case MSG_UNFOCUS:
		setFocus(false);
		break;
	}
}

void Button::parseAndSetText(std::string value)
{
	stringstream ss;
	char accelerator;

	int state = 0;

	for (unsigned int i = 0; i < value.size(); ++i)
	{
		char x = value[i];
		if (state == 1)
		{
			//TODO: insert underline text style. Use styled text model...
			accelerator = x;
			ss << x;
			state = 0;
		}
		else
		{
			if (x == '&')
			{
				state = 1;
			}
			else
			{
				ss << x;
			}
		}
	}
	SetText(ss.str().c_str());

	//TODO: set accelerator key.
}

ComponentBuilder<Button> Button::build(int action, std::string text, int accelerator_key, ALLEGRO_BITMAP* icon)
{
	auto result = make_shared<Button>(action, text, icon);
	result->setAccelerator(accelerator_key);
	return ComponentBuilder<Button>(result);
}

ComponentBuilder<Button> Button::build(ActionFunc actionFunc, std::string text, int accelerator_key, ALLEGRO_BITMAP* icon)
{
	auto result = make_shared<Button>(0, text, icon);
	result->setActionFunc(actionFunc);
	result->setAccelerator(accelerator_key);
	return ComponentBuilder<Button>(result);
}

void Button::MakeExit() { /* TODO */ } // make this button exit the dialog
