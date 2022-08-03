#include "messagebox.h"
#include <memory>
#include "button.h"
#include "text.h"
#include "mainloop.h"

using namespace std;

class MessageBoxImpl : public MessageBox
{
	int response = -1;

public:
	int getResponse() { return response; }
	MessageBoxImpl(const char * title, const char *message, const char *btn1, const char *btn2, const char *btn3)
	{
		setDimension(400, 120);
		setTitle(title);

		auto lbl = Text::build(BLACK, ALLEGRO_ALIGN_LEFT, message).layout(Layout::LEFT_TOP_RIGHT_BOTTOM, 10, 10, 10, 50).get();

		int buttonw = 100;
		int buttonh = 20;

		vector<const char *> btns;

		int spacer = 20;
		btns.push_back(btn1);
		if (btn2 != nullptr)
		{
			btns.push_back(btn2);
			if (btn3 != nullptr)
			{
				btns.push_back(btn3);
			}
		}

		int totalw = buttonw * btns.size() + spacer * (btns.size() - 1);

		int xco = -(totalw - buttonw) / 2;
		int yco = 15;

		getClient()->add(lbl);
		int btnIdx = 1;
		for (auto btnStr : btns)
		{
			cout << xco << endl;
			auto btn = Button::build(
					[=]{ this->response = btnIdx; pushMsg(MSG_CLOSE); },
					btnStr).layout(Layout::CENTER_BOTTOM_W_H, xco, yco, buttonw, buttonh).get();
			getClient()->add(btn);
			xco += buttonw + spacer;
			btnIdx++;
		}

	}
};

int MessageBox::showMessage(const char * title, const char *message, const char *btn1, const char *btn2, const char *btn3)
{
	auto dlg = make_shared<MessageBoxImpl>(
			title, message, btn1, btn2, btn3
	);
	dlg->Centre();
	dlg->PopupModal();

	return dlg->getResponse();
}
