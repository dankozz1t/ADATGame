#pragma once
#include <vector>
#include "TileMap.h"
#include "Object.h"
#include "Player.h"
#include "InteractiveArray.h"

using namespace std;

class Level : public InteractiveArray
{
private:
	vector<pair<int, TileMap>> tileLayers;
	vector<Object> objects;

	void insertWithPriority(vector<pair<int, TileMap>>& layers, pair<int, TileMap> tmap);
	bool _bordered = false;
public:
	using InteractiveArray::InteractiveArray;

	bool bordered() const;

	vector<Object>& getObjects();

	Level& load(const string& filename, const Vector2f& offset = Vector2f(0, 0), const RenderWindow* window = nullptr, map<string, function<void()>> useMap = {});

	void Draw(RenderWindow& wnd, Player* player = nullptr) const;
	
	void applyFuncMap(map<string, function<void()>> map) {
		for (auto& obj : interactives) {
			if (map.count(obj->getName())) {
				obj->use = map[obj->getName()];
			}
		}
	}

	void Update(Player& player) override;
};

