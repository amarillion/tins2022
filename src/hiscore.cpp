#include "hiscore.h"
#include "util.h"
#include <algorithm>
#include <list>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#include "../include/engine.h"
#include "color.h"
#include "mainloop.h"

using namespace std;

struct Record
{
	string name;
	int score;
};

const char *defaultNames[10] = {
		"Betelgeuse",
		"Copernicus",
		"Tycho Brahe",
		"Adonis",
		"Kepler",
		"Buzz Aldrin",
		"Werner von Braun",
		"Riga",
		"Alpha Centauri",
		"Orion"
};

class HiScoreImpl : public HiScore
{
private:

	vector<Record> records;
	const char *configfilename = "hiscore.cfg";
	bool blinkOn = false;
	int blinkTimer = 0;


public:


	HiScoreImpl() : records(10) {}
	virtual ~HiScoreImpl() {}

	virtual void init() override
	{
		ALLEGRO_CONFIG *config;
		config = al_load_config_file(configfilename);
		if (config == NULL) config = al_create_config();

		for (int i = 0; i < 10; ++i)
		{
			char buffer[1024];
			snprintf (buffer, sizeof(buffer), "score_%i", i);
			records[i].score = get_config_int(config, "hiscores", buffer, 10 * (10 - i));

			snprintf (buffer, sizeof(buffer), "name_%i", i);
			records[i].name = get_config_string(config, "hiscores", buffer, defaultNames[i]);
		}

		sort (records.begin(), records.end(), [](const Record &a, const Record &b){ return a.score > b.score; });

		al_destroy_config(config);
	}

	virtual bool isHiscore (int score) override
	{
		return (score > records[9].score);
	}

    virtual void draw(const GraphicsContext &gc) override
    {
    	al_clear_to_color(BLUE);

    	for (int i = 0; i < 10; ++i)
    	{

    		int xco = 10;
    		int yco = (10 * i) + 60;

    		al_draw_text (sfont, WHITE,
					xco, yco, ALLEGRO_ALIGN_LEFT, records[i].name.c_str());

    		xco += al_get_text_width(sfont, records[i].name.c_str());

    		if (editing && editPos == i)
        	{
    			if (blinkOn)
    			{
    	    		al_draw_text (sfont, GREEN,
    						xco, yco, ALLEGRO_ALIGN_LEFT, "_");
    			}
        	}

    		al_draw_textf (sfont, CYAN,
    				200, yco, ALLEGRO_ALIGN_LEFT, "... %5i000", records[i].score);

    	}

    }

    virtual void update() override
    {
    	if (editing)
    	{
    		// blink cursor...
    		if (blinkTimer++ > 10) { blinkTimer = 0; blinkOn = !blinkOn; }
    	}
    }

private:
    void saveToFile()
    {
		ALLEGRO_CONFIG *config;
		config = al_create_config();

		for (int i = 0; i < 10; ++i)
		{
			char buffer[1024];
			snprintf (buffer, sizeof(buffer), "score_%i", i);
			set_config_int (config, "hiscores", buffer, records[i].score);

			snprintf (buffer, sizeof(buffer), "name_%i", i);
			al_set_config_value (config, "hiscores", buffer, records[i].name.c_str());
		}
		al_save_config_file(configfilename, config);
    }

public:
    virtual void handleEvent (ALLEGRO_EVENT &event) override
    {
		if (event.type == ALLEGRO_EVENT_KEY_CHAR)
		{
			if (editing)
			{
				string &name = records[editPos].name;
				switch (event.keyboard.keycode)
				{
				case ALLEGRO_KEY_ESCAPE:
				case ALLEGRO_KEY_ENTER:
					editing = false;
					saveToFile();
					pushMsg(Engine::MENU_MAIN);
					break;
				case ALLEGRO_KEY_DELETE:
				case ALLEGRO_KEY_BACKSPACE:
				{
					if (name.length() > 0)
					{
						records[editPos].name = name.substr(0, name.length()-1);
					}
				}
				break;
				default:
					if (name.length() < 20)
					{
						if (event.keyboard.unichar >= 32 &&
							event.keyboard.unichar < 127)
						{
							records[editPos].name += (char)event.keyboard.unichar;
						}
					}
					break;
				}

			}
			else
			{
				pushMsg(Engine::MENU_MAIN);
			}
		}
    }

    bool editing = true;
    int editPos = -1;

    virtual void add(int newScore) override
    {
    	for (int i = 0; i < 10; ++i)
    	{
    		if (records[i].score <= newScore)
    		{
    			editPos = i;
    	    	editing = true;
    			break;
    		}
    	}

    	if (editPos >= 0)
    	{
			// shift remaining ones down.
			for (int i = 8; i >= editPos; i--)
			{
				records[i+1] = records[i];
			}

			records[editPos].score = newScore;
			records[editPos].name = "";
    	}
    }

};

shared_ptr<HiScore> HiScore::newInstance()
{
	shared_ptr<HiScore> result = make_shared<HiScoreImpl>();
	return result;
}

