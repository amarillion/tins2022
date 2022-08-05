#include "script.h"
#include "engine.h"
#include "color.h"
#include <fstream>
#include "leveldata.h"
#include <iostream>
#include <assert.h>
#include <allegro5/allegro_font.h>
#include "util.h"
#include "mainloop.h"
#include "resources.h"

using namespace std;

Script::Script(Engine &e) : scriptmap(), ip(), parent(e), lines()
{
	state = SCRIPT_STOP; // call prepare() to start
}

void Script::prepare (int _level)
{
	level = _level;
	if (level == LEVEL_GAMEOVER)
	{
		currentScript = scriptmap["gameover"];
	}
	else if (level == LEVEL_WON)
	{
		currentScript = scriptmap["won"];
	}
	else
	{
		char buffer[100];
		snprintf (buffer, sizeof (buffer), "level%i", level + 1);
		currentScript = scriptmap[buffer];
	}
	ip = currentScript.begin();
	
	state = SCRIPT_PAUSE;
	
	tPause = 0;
	pause_time = 30;
	lines.clear();
	textx = 20;
	texty = 20;
	textw = MAIN_WIDTH;
	texth = MAIN_HEIGHT;
	imgx = 0;
	imgy = 0;
	img = NULL;
}

void Script::runScript ()
{
	if (ip == currentScript.end()) return;
	switch (ip->cmd)
	{
		case CMD_SAY:
			lines.push_back (ip->args);
			break;
		case CMD_WAIT:
			state = SCRIPT_PAUSE;
			pause_time = ip->arg1;
			break;
		case CMD_PAUSE:
			state = SCRIPT_WAIT_KEYPRESS;
			break;
		case CMD_IMG_CENTER:
			img = parent.getResources()->getBitmap(ip->args);
			imgx = (MAIN_WIDTH - al_get_bitmap_width(img)) / 2;
			imgy = (MAIN_HEIGHT - al_get_bitmap_height(img)) / 2;
			break;
		case CMD_IMG:
			img = parent.getResources()->getBitmap(ip->args);
			imgx = ip->arg1;
			imgy = ip->arg2;
			break;
	}
	ip++;
}

void Script::draw(const GraphicsContext &gc)
{	
	al_set_target_bitmap(gc.buffer);
	al_clear_to_color (BLACK);
	
	vector<string>::iterator i;
	
	int x = textx;
	int y = texty;
	int lineh = al_get_font_line_height(scriptfont);
	
	for (i = lines.begin(); i != lines.end(); ++i)
	{
		al_draw_text (scriptfont, WHITE, x, y, ALLEGRO_ALIGN_LEFT, i->c_str());
		y += lineh;
	}
	
	if (img)
	{
		al_draw_bitmap (img, imgx, imgy, 0);
	}
}

void Script::handleEvent (ALLEGRO_EVENT &event)
{
	if (event.type != ALLEGRO_EVENT_KEY_CHAR) return;

	switch (state)
	{
		case SCRIPT_WAIT_KEYPRESS:
			{
				lines.clear(); // clear current text box
				state = SCRIPT_RUN;
			}
			break;
		default:
			// nothing
			break;
	}
}

void Script::update()
{
	switch (state)
	{
		case SCRIPT_RUN:
			while (state == SCRIPT_RUN)
			{
				if (ip == currentScript.end())
				{
					state = SCRIPT_STOP;
					if (level == LEVEL_WON || level == LEVEL_GAMEOVER)
					{
						pushMsg(Engine::E_MAINMENU);
					}
					else
					{
						pushMsg(Engine::E_NEXTLEVEL);
					}
				}
				runScript();
			}
			break;
		case SCRIPT_WAIT_KEYPRESS:
			// nothing, let handleEvent handle this
			break;
		case SCRIPT_PAUSE:
			tPause++;
			if (tPause > pause_time)
			{
				tPause = 0;
				pause_time = 0;
				state = SCRIPT_RUN;
			}
			break;
		case SCRIPT_STOP:
			// do nothing...
			break;
	}
}

void Script::readFromFile(const char * filename)
{
	char errMsg[100];
	bool error = false;
	
	vector<Instruction> script;
	
	ifstream infile (filename, ios::in);
	if (!infile)
	{
		snprintf (errMsg, sizeof(errMsg), "Could not open file '%s'", filename);
		error = true;
	}
	
	char temp [1024];
	char line [1024];
	
	string section = "main"; // default section
	
	Instruction i;
	while (!error && !infile.eof())
	{
		infile.getline (line, sizeof (line));
		
		if (line[0] == '[')
		{
			// start of section
			char *start = strchr(line, '[') + 1; 
			char *end = strchr(line, ']');
			
			assert (start);
			assert (end);
			
			int len = end - start;
			strncpy(temp, start, len);
			temp[len] = '\0';
			 
			scriptmap.insert (pair<string, vector<Instruction> >(section, script));
			script.clear();
			section = string (temp);
		}
		else if (strncmp (line, "SAY", 3) == 0)
		{
			char *start = strchr(line, ' ') + 1; 
			char *end = strchr(line, '\0');
			
			assert (start);
			assert (end);
			
			int len = end - start + 1;
			strncpy(temp, start, len);
			
			i.cmd = CMD_SAY;
			i.args = string (temp);
			script.push_back(i);
		}
		else if (strncmp (line, "CENTER", 6) == 0)
		{
			char *start = strchr(line, ' ') + 1; 
			char *end = strchr(line, '\0');
			
			assert (start);
			assert (end);
			
			int len = end - start + 1;
			strncpy(temp, start, len);
			
			i.cmd = CMD_IMG_CENTER;
			i.args = string (temp);
			script.push_back(i);
		}
		else if (strncmp (line, "IMAGE", 5) == 0)
		{
			int code = sscanf (line, "IMAGE %i %i %s", &i.arg1, &i.arg2, temp);
			assert (code == 3);
			i.args = string (temp);
			i.cmd = CMD_IMG;
			script.push_back(i);
		}
		else if (strncmp (line, "PAUSE", 5) == 0)
		{
			i.cmd = CMD_PAUSE;
			script.push_back(i);
		}
		else if (strcmp (line, "") == 0)
		{
			// ignore emtpy line
		}
		else
		{
			error = true;
			snprintf (errMsg, sizeof (errMsg), "Could not parse line '%s'", line);
		}
	}
	
	if (!error)
	{
		scriptmap.insert (pair<string, vector<Instruction> >(section, script));
		script.clear();
	}
	
	if (error)
	{
		allegro_message ("%s", errMsg);
		assert (false);
	}
/*	
// print script
	for (map<string, vector<Instruction> >::iterator i = scriptmap.begin(); i != scriptmap.end(); ++i)
	{
		cout << i->first << endl;
		for (vector<Instruction>::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			cout << "  " << j->cmd << " " << j->args << endl;
		}
	}
*/
}

void Script::init()
{
	scriptfont = parent.getResources()->getFont("rursus_-_Rursus_Compact_Mono")->get(16);
	readFromFile ("data/scripts.txt");
}
