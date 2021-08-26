#pragma once
#include "SDK/Animation.h"
#include "SDK/Player.h"
#include "SDK/Object.h"
#include <vector>

#include "SDK/Level.h"
#include "SDK/Button.h"
#include <iostream>
#include <SFML/Audio.hpp>
#include "SDK/ConfigManager.h"
#include <Windows.h>
#include "SDK/Menu.h"
#include "SDK/SmartSprite.h"

using namespace std;
using namespace sf;

class World
{
private:
	View view;
	bool viewCentered = true;
	MusicPlayer musicPlayer;

	ContextSettings cs;
	RenderWindow wnd;

	Level level;
	int currLevel = 0;

	Player player;
	// ����� �������� ������� � ������ �������������
	map<string, map<string, function<void()>>> useMaps;
	map<string, shared_ptr<SmartSprite>> sprites;

	enum class MenuState : int {
		MAIN, INGAME, LEVELS, WIN, NO_MENU
	};
	map<string, Menu> menus;		
	MenuState menuState = MenuState::MAIN;


	inline void initLevel() {
		if (useMaps.count(level.getName())) {
			level.applyUseMap(useMaps[level.getName()]);
		}
	}
	inline void initUseMaps() {
		useMaps["hub"] =
		{
		{
		"hub_levels", [this]() mutable {
			menuState = MenuState::LEVELS;
			view.setCenter(Vector2f(wnd.getSize() / 2u));
			menus["levels"].setPosition(Vector2f(75, 225));
			sprites["levels_bg"]->setPosition(view.getCenter());
		}
		}
		};

		for (int i = 0; i < 3; i++)
			useMaps["lvl1"].emplace(std::make_pair("wrong_door" + to_string(i + 1), 
				[this]() mutable {
					player.respawn(level.spawn);
				}));
		
		for (int i = 0; i < 3; i++) 
			useMaps["lvl" + to_string(i+1)].emplace(make_pair(string("door_lvl"),
				[this]() mutable {
					menuState = MenuState::WIN;
				}));
		
		useMaps["lvl1"].emplace(make_pair(string("door_opener"), [this]() {
			((InteractiveDoor*)level.getIntObject("block_door").get())->Update();
			} ));
	}
	inline void initMenus() {
		menus["main"] = Menu(Vector2f(wnd.getSize().x / 2, wnd.getSize().y / 2 + 100), true);;
		menus["main"].load("Models/menuButtons.xml", "Models/Menu/mainMenu.xml");
		menus["main"].applyUseMap({
	{
	"start",
	[this]() mutable {
		menuState = MenuState::NO_MENU;
		if (level.getName() != "hub") {
			level = ConfigManager::loadLevel("hub");
			initLevel();
			player.respawn(level.spawn);
		}
		musicPlayer.setMusic("hub");
		musicPlayer.play();
	}
	},
	{
	"exit",
	[this]()mutable {
		musicPlayer.stop();
		wnd.close();
		exit(0);
	}
	}
			});

		menus["ingame"] = Menu(Vector2f(wnd.getSize().x / 2, wnd.getSize().y / 2), true);
		menus["ingame"].load("Models/menuButtons.xml", "Models/Menu/ingameMenu.xml");
		menus["ingame"].applyUseMap({
			{
			"continue",
			[this]() mutable {
				menuState = MenuState::NO_MENU;
				if (musicPlayer.isInterrupted()) musicPlayer.play();
			}
			},
			{
			"menu",
			[this]() mutable {
				musicPlayer.stop();
				view.setCenter(Vector2f(wnd.getSize() / 2u));
				wnd.setView(view);
				menuState = MenuState::MAIN;
			}
			}
			});

		menus["levels"] = Menu(Vector2f(75, 225), false);
		menus["levels"].load("Models/levelButtons.xml", "Models/Menu/levelMenu.xml");
		for (int i = 0; i < 8; i++) {
			menus["levels"].applyUseMap({
				{
					string("lvl") + to_string(i+1),
					[this, i]() mutable {
						level = ConfigManager::loadLevel(string("lvl") + to_string(i+1));
						initLevel();
						player.respawn(level.spawn.x, level.spawn.y);
						menuState = MenuState::NO_MENU;
						musicPlayer.setMusic("lvl" + to_string(i + 1));
						musicPlayer.play();
				}
				}
				});
		}

		menus["win"] = Menu(Vector2f(wnd.getSize().x / 2, wnd.getSize().y / 2), true);
		menus["win"].load("Models/menuButtons.xml", "Models/Menu/winMenu.xml");
		menus["win"].applyUseMap({
			{
			"menu",
			[this]() mutable {
				menuState = MenuState::NO_MENU;
				level = ConfigManager::loadLevel("hub");
				initLevel();
				player.respawn(level.spawn);
			}
			}
			});

	}
	inline void initSprites() {
		sprites["menu_bg"] = shared_ptr<SmartSprite>(new SmartSprite("Textures/menu_bg.png"));
		sprites["menu_bg"]->setPosition(Vector2f(wnd.getSize() / 2u));

		sprites["ingame_menu_bg"] = shared_ptr<SmartSprite>(new SmartSprite("Textures/ingame_menu_bg.png"));
		sprites["ingame_menu_bg"]->setPosition(Vector2f(wnd.getSize() / 2u));

		sprites["hub_bg"] = shared_ptr<SmartSprite>(new SmartSprite("Textures/hub_bg.jpg"));
		sprites["lvl1_bg"] = shared_ptr<SmartSprite>(new SmartSprite("Textures/lvl1_bg.jpg"));;
		sprites["lvl2_bg"] = shared_ptr<SmartSprite>(new SmartSprite("Textures/lvl2_bg.jpg"));;
		sprites["lvl3_bg"] = sprites["lvl1_bg"];

		sprites["unfocused_greyscale"] = shared_ptr<SmartSprite>(new SmartSprite());
		sprites["levels_bg"] = shared_ptr<SmartSprite>(new SmartSprite("Textures/levels_bg.png"));
		sprites["win_bg"] = shared_ptr<SmartSprite>(new SmartSprite("Textures/win_bg.png"));
	}

public:
	inline World(const Vector2f& wndSize) :
		wnd(RenderWindow(sf::VideoMode(wndSize.x, wndSize.y), L"�� � �����!", Style::Titlebar | Style::Close , cs)),
		view(View(Vector2f(wndSize.x, wndSize.y), wndSize)),
		level(ConfigManager::loadLevel()),
		player(new Animation("Models/rat.xml"), Vector2f(191 / 2, 191 / 2), 650, 210,1),
		musicPlayer("cfg/music.xml")
	{
		cs.antialiasingLevel = 8;
		musicPlayer.setVolume(10);
		player.respawn(
			level.spawn.x, 
			level.spawn.y
		);
		initSprites();
		initUseMaps();
		initMenus();
		initLevel();
	}

	inline void start() {
		view.setCenter(Vector2f(wnd.getSize() / 2u));

		Event ev;
		Font pixelFont;
		pixelFont.loadFromFile("Fonts/kongtext.ttf");
		Font hintFont;
		hintFont.loadFromFile("Fonts/forward.ttf");

		Clock clock;
		float deltaTime = 0;

		bool greyscaled = false;;

		Texture unfocused;
		unfocused.create(wnd.getSize().x, wnd.getSize().y);
		musicPlayer.setMusic("main_menu");
		musicPlayer.play();
		
		while (wnd.isOpen()) {
			while (!wnd.hasFocus()) {
				if (!greyscaled) {
					unfocused.update(wnd);
					sprites["unfocused_greyscale"]->setTexture(unfocused);
					sprites["unfocused_greyscale"]->setPosition(
						Vector2f(view.getCenter().x - wnd.getSize().x / 2, view.getCenter().y - wnd.getSize().y / 2)
					);
					wnd.draw(*sprites["unfocused_greyscale"], &Achievement::greyscale);
					wnd.display();
					greyscaled = true;
				}
				if (wnd.pollEvent(ev))
					if (ev.type == Event::GainedFocus) {
						greyscaled = false;
						break;
					}
			}
			wnd.clear();				
			sprites[level.getName() + "_bg"]->setPosition(view.getCenter());
			wnd.draw(*sprites[level.getName() + "_bg"]);

			if (menuState == MenuState::NO_MENU) {
				
				deltaTime = clock.restart().asSeconds();
				if (deltaTime > 1. / 60.) 
					deltaTime = 1. / 60.;
			
				player.Update(deltaTime);
				level.checkCollision(player, deltaTime);

				level.Update(player);
				level.Update(deltaTime);
			}

			level.Draw(wnd, &player);
			level.drawHint(wnd, player, pixelFont);

			wnd.setView(view);

			switch (menuState) {
			case MenuState::NO_MENU: {
				musicPlayer.setMusic(level.getName());
				musicPlayer.play();

				view.setCenter(Vector2f(player.getPos().x - 0.01, player.getPos().y));

				if (view.getCenter().x - wnd.getSize().x / 2 < 0)
					view.move(-(view.getCenter().x - wnd.getSize().x / 2) - 0.01, 0);
				else if (view.getCenter().x + wnd.getSize().x / 2 > level.getSize().x)
					view.move((level.getSize().x - (view.getCenter().x + wnd.getSize().x / 2)) - 0.01, 0);
				if (view.getCenter().y - wnd.getSize().y / 2 < 0)
					view.move(0, -(view.getCenter().y - wnd.getSize().y / 2));
				else if (view.getCenter().y + wnd.getSize().y / 2 > level.getSize().y)
					view.move(0, (level.getSize().y - (view.getCenter().y + wnd.getSize().y / 2)));


				if (Keyboard::isKeyPressed(Keyboard::Dash))
					musicPlayer.setVolume(musicPlayer.getVolume() - 1);
				if (Keyboard::isKeyPressed((Keyboard::Key)55))
					musicPlayer.setVolume(musicPlayer.getVolume() + 1);

				while (wnd.pollEvent(ev)) {
					switch (ev.type) {
					case Event::Closed:
						wnd.close(); return;
						break;
					case Event::Resized:
						view.setSize(Vector2f(wnd.getSize()));//VIEW_HEIGHT * (float(wnd.getSize().x) / float(wnd.getSize().y)), VIEW_HEIGHT);
						break;
					case Event::KeyReleased: {
						switch (ev.key.code) {
						case Keyboard::Escape:
							musicPlayer.interrupt();
							menuState = MenuState::INGAME;
							sprites["ingame_menu_bg"]->setPosition(view.getCenter());
							menus["ingame"].setPosition(Vector2f(view.getCenter().x, view.getCenter().y - 100));
							continue;
							break;
						case Keyboard::F11:
							Level::debug = !Level::debug;
							break;
						case Keyboard::X:
							level.reload();
							initLevel();
							break;
						case Keyboard::M:
							musicPlayer.setVolume(musicPlayer.muted() ? 100 : 0);
							break;
						}
					}
										   break;
					case Event::MouseButtonReleased:
						//if (ev.mouseButton.button == Mouse::Button::Left) 
						//	musicPlayer.CheckClick(ev, wnd, view);
						break;
					}
					level.checkInteraction(ev, player);
				}

			}
								   break;
			case MenuState::MAIN: {
				musicPlayer.setMusic("main_menu");
				musicPlayer.play();

				view.setCenter(Vector2f(wnd.getSize() / 2u));
				wnd.draw(*sprites["menu_bg"]);
				menus["main"].drawButtons(wnd);
				while (wnd.pollEvent(ev)) {
					menus["main"].CheckClick(ev, wnd, view);
					if (ev.type == Event::Closed) {
						wnd.close();
						exit(0);
					}
				}
			}
								break;
			case MenuState::INGAME: {
				wnd.draw(*sprites["ingame_menu_bg"]);
				menus["ingame"].drawButtons(wnd);
				if (wnd.pollEvent(ev)) {
					if (ev.type == Event::KeyReleased)
						if (ev.key.code == (Keyboard::Escape)) {
							menuState = MenuState::NO_MENU;
							if (musicPlayer.isInterrupted()) musicPlayer.play();
							continue;
						}
					menus["ingame"].CheckClick(ev, wnd, view);
				}
			}
				break;
			case MenuState::WIN: {
				sprites["win_bg"]->setPosition(view.getCenter());
				wnd.draw(*sprites["win_bg"]);
				menus["win"].setPosition(view.getCenter());
				menus["win"].drawButtons(wnd);
				if (wnd.pollEvent(ev)) 
					menus["win"].CheckClick(ev, wnd, view);
			}
								  break;

			case MenuState::LEVELS: {
				Text levelNumber("", pixelFont, 80);
				levelNumber.setFillColor(Color::Black);
				levelNumber.setOrigin(80 / 6 - 5, 80 /3);

				wnd.draw(*sprites["levels_bg"]);
				menus["levels"].drawButtons(wnd);
				for (int i = 0; i < 8; i++) {
					levelNumber.setString(to_string(i+1));
					levelNumber.setPosition(menus["levels"].getButtonPos("lvl" + to_string(i+1)) + Vector2f(75, 75));
					wnd.draw(levelNumber);
				}
				while (wnd.pollEvent(ev)) {
					menus["levels"].CheckClick(ev, wnd, view);
					if (ev.type == Event::Closed) {
						wnd.close();
						exit(0);
					}
				}
			}
				break;
			}


			wnd.display();
		}
	}
};

