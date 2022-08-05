#ifndef SCRIPT_H
#define SCRIPT_H

#include <vector>
#include <string>
#include <map>
#include "state.h"

class Engine;

enum Command 
{
	CMD_SAY, // add line "args" in textbox
	CMD_PAUSE, // wait for keypress, then clear
	CMD_WAIT, // wait arg1 msec
	CMD_IMG_CENTER, // get image "args" from resources and display it at center
	CMD_IMG, // get image "args" from resources and display it at arg1, arg2
};

class Instruction
{
public:
	Command cmd;
	int arg1, arg2;
	std::string args;
};

const int LINEH = 10; // line distance

class Script : public State
{
private:
	std::map <std::string, std::vector<Instruction> > scriptmap;
	enum ScriptState { SCRIPT_RUN, SCRIPT_PAUSE, SCRIPT_WAIT_KEYPRESS, SCRIPT_STOP };
	std::vector<Instruction> currentScript;
	std::vector<Instruction>::iterator ip;
	ScriptState state;
	int tPause;
	int pause_time;
	int level;
	Engine &parent;
	void runScript();
	virtual void handleEvent (ALLEGRO_EVENT &event) override;
	
	// objects that are manipulated
	std::vector <std::string> lines;
	int textx, texty, textw, texth;
	ALLEGRO_BITMAP *img;
	int imgx, imgy;
	
	ALLEGRO_FONT *scriptfont;
public:
	Script (Engine &e);
	virtual ~Script() {}
	void init();
	void prepare (int level);
	virtual void update() override;
	virtual void draw(const GraphicsContext &gc) override;
	
	void readFromFile (const char *filename);
};

#endif
