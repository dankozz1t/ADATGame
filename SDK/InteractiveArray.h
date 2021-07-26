#pragma once
#include "Object.h"
#include <vector>
#include <Player.h>
#include <functional>

using namespace std;

typedef unsigned int uint;

enum class IntObjType {
	Button, Lever
};

class InteractiveObject : public Object {
protected:
	string name;
	IntObjType type;

	Animation anim;
	uint row = 0;

	bool active = true;

	// �� ������, ���� ������ ����� �����������
	bool oneTime = false;
public:
	InteractiveObject(Texture* text, Vector2f size, Vector2f pos, string name, IntObjType type, function<void()> use = []() {}, bool oneTime = false);

	function<void()> use;

	static Keyboard::Key interactKey;

	const string& getName() const;
	const IntObjType& getType() const;

	virtual void Update() = 0;

	uint getRow() const;

	bool isActive() const;;
	bool isOneTime() const;
};

class InteractiveButton : public InteractiveObject {
public:	
	InteractiveButton(Texture* text, Vector2f size, Vector2f pos, string name, function<void()> use = []() {}, bool oneTime = false);

	bool pressed = false;

	void Update() override;
};

class InteractiveLever : public InteractiveObject {

public:
	InteractiveLever(Texture* text, Vector2f size, Vector2f pos, string name, function<void()> use = []() {}, bool oneTime = false);;

	void Update() override;;
	bool on = false;
};

class InteractiveArray
{
private:
	Text hintText;
	float hintOpacity = 0;
	float opacityOffset = 0.4;
protected:
	vector<InteractiveObject*> interactives;
public:
	InteractiveArray() {
		hintText.setString("E");
		hintText.setOrigin(Vector2f(hintText.getCharacterSize(), hintText.getCharacterSize()/2) / 2.0f);
		hintText.setColor(Color(255, 255, 255, hintOpacity));
	}
	void checkInteraction(Event& ev, Player& player);

	// �������� �� ��������� ������ ��� ������� ������ ��� �������� � ������������� �������� 
	void drawHint(Player& player, RenderWindow& wnd, Font& font);
	
	void addObject(InteractiveObject* obj);

	virtual void Update(Player& player) = 0;
};

