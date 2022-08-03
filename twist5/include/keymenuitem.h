#pragma once

#include "menubase.h"
#include "input.h"

class KeyMenuItem : public MenuItem
{
	private:
		std::string btnName;
		const char* btnConfigName;
		Input &btn;
		bool waitForKey;
		ALLEGRO_CONFIG *config;
	public:
		KeyMenuItem (std::string _btnName, const char* _btnConfigName, Input & _btn, ALLEGRO_CONFIG *_config) :
			btnName (_btnName),
			btnConfigName(_btnConfigName), btn(_btn), waitForKey (false), config(_config) {}
		virtual void handleEvent(ALLEGRO_EVENT &event) override;
		virtual std::string getText() override;
		virtual std::string getHint() override;
};
