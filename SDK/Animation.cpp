#include "Animation.h"
#include <iostream>

Animation::Animation(const string& xmlDoc) :
	texture(new Texture()) {
	
	TiXmlDocument doc(xmlDoc.c_str());
	doc.LoadFile();
	if (doc.FirstChildElement("default") != nullptr) {
		string textPath = "Textures/" + string(doc.FirstChildElement("default")->Attribute("src"));
		texture->loadFromFile(textPath);
		rectMap["default"].push_back(IntRect(Vector2i(0, 0), Vector2i(texture->getSize())));
		uvRect = IntRect(Vector2i(0, 0), Vector2i(texture->getSize()));
		return;
	}
	if (doc.FirstChildElement("frames") != nullptr) {
		string textPath = "Textures/" + string(doc.FirstChildElement("frames")->Attribute("src"));
		texture->loadFromFile(textPath);
		
		int x = atoi(doc.FirstChildElement("frames")->Attribute("x")), 
			y = atoi(doc.FirstChildElement("frames")->Attribute("y"));
		for (int i = 0; i < x; i++) {
			for (int j = 0; j < y; j++) {
				rectMap["default"].push_back(
					IntRect(
						texture->getSize().x / x * i, 
						texture->getSize().y / y * j,
						texture->getSize().x/x, 
						texture->getSize().y / y)
				);
			}
		}
		uvRect = rectMap["default"][0];
		return;
	}

	
	TiXmlElement* sprites = doc.FirstChildElement("sprites");
	for (TiXmlElement* anim = sprites->FirstChildElement("animation"); anim != nullptr; anim = anim->NextSiblingElement()) {
		switchMap[anim->Attribute("title")] = atoi(anim->Attribute("delay")) / 1000.;
		if (switchMap[anim->Attribute("title")] <= 0.01) 
			switchMap[anim->Attribute("title")] = 0;

		for (TiXmlElement* cut = anim->FirstChildElement("cut"); cut != nullptr; cut = cut->NextSiblingElement()) {
			int width = atoi(cut->Attribute("w")),
				height = atoi(cut->Attribute("h")), 
				x = atoi(cut->Attribute("x")), 
				y= atoi(cut->Attribute("y"));
			rectMap[anim->Attribute("title")].push_back(IntRect(x, y, width, height));
		}
	}

	{
		string textPath = "Textures/" + string(sprites->Attribute("image"));
		texture->loadFromFile(textPath);
	}

	try {
		if (rectMap.empty()) 
			throw out_of_range("��������(�����������): ��� ������� ������ ������ ������");
		uvRect = (*rectMap.begin()).second[0];
		currAnim = (*rectMap.begin()).first;
	}
	catch (out_of_range err) {
		cerr << err.what();
	}
}

bool Animation::Update(float deltaTime, bool mirrored, string animName) {
	bool success = false;

	if (animName == "") 
		animName = currAnim;
	if(setAnim(animName)) return true;

	totalTime += deltaTime;

	if (totalTime >= switchMap[currAnim]) {
		totalTime -= switchMap[currAnim];
		currFrame = currFrame == rectMap[currAnim].size() - 1 ? 0 : currFrame + 1;
		uvRect = rectMap[currAnim][currFrame];
		success = true;
	}

	
	if (mirrored) {
		uvRect.left += abs(uvRect.width);
		uvRect.width = -abs(uvRect.width);
	}
	
	_mirrored = mirrored;

	return success;
}

int Animation::getCurrFrame() const noexcept {
	return currFrame;
}

const Texture* Animation::getTexture() const noexcept {
	return texture;
}

const string& Animation::getCurrAnim() const noexcept {
	return currAnim;
}

bool Animation::setAnim(const string& animName) {
	if (animName != currAnim) {
		currAnim = animName;
		currFrame = 0;
		totalTime = 0;
		uvRect = rectMap[currAnim][0];
		return true;
	}
	return false;
}