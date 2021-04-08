#include "SDL.h"
#include "SDL_ttf.h"
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <random>
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
using namespace std;
#undef main
enum GameState {
	MAIN_MENU,
	WIN,
	GAME,
	LOAD,
	EXIT,
	LOSE,
};
struct Card {
	int number;
	int type;
	SDL_Texture* texture;
};
struct Button {
	SDL_Texture* texture;
	SDL_Rect* rect;
	SDL_Texture* text;
	void (*function)(GameState&);
};
int myRandomGenerator(int j) {
	return rand() % j;
}
void renderButton(SDL_Renderer* renderer, Button btn) {
	if(btn.texture!=nullptr)
		SDL_RenderCopy(renderer, btn.texture, 0, btn.rect);
	if(btn.text!=nullptr)
		SDL_RenderCopy(renderer, btn.text, 0, btn.rect);
}
bool isWithinRect(SDL_Rect* rect, int mouseX, int mouseY) {
	if ((mouseX >= rect->x && mouseX <= (rect->x + rect->w)) && (mouseY >= rect->y && mouseY <= (rect->y + rect->h)))
		return true;
	return false;
}
void loadDeck(deque<Card> &deck,vector<string>textureNamesNumbers,vector<string>textureNamesTypes,SDL_Renderer* renderer){

	for (int cardNo = 0; cardNo < textureNamesNumbers.size(); cardNo++)
	{
		for (int cardType = 0; cardType < textureNamesTypes.size(); cardType++)
		{
			string textureName = ("Textures/Playing Cards/" + textureNamesNumbers[cardNo] + "_of_" + textureNamesTypes[cardType] + ".bmp");
			SDL_Surface* loadedSurface = SDL_LoadBMP(textureName.c_str());
			Card card;
			card.number = cardNo;
			card.type = cardType;
			card.texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
			deck.push_back(card);
			SDL_FreeSurface(loadedSurface);
		}
	}
}
void Draw(deque<Card>& deck, vector<Card>& hand, vector<string> textureNamesNumbers,vector<string> textureNamesTypes,SDL_Renderer* renderer)
{
	if (deck.empty())
		loadDeck(deck, textureNamesNumbers, textureNamesTypes, renderer);
	hand.push_back(deck.front());
	deck.pop_front();
}
void loadGame(deque<Card>& deck, Card& cardOnTable, vector<string> textureNamesNumbers, vector<string> textureNamesTypes,SDL_Renderer* renderer, vector<Card>& playerHand, vector<Card>& cpuHand) {
	
	playerHand.clear();
	deck.clear();
	loadDeck(deck,textureNamesNumbers,textureNamesTypes,renderer);
	std::random_device rd;
	std::mt19937 g(rd());
	shuffle(deck.begin(), deck.end(), g);
	cardOnTable = deck.front();
	deck.pop_front();
	for (int i = 0; i < 5; i++)
	{
		playerHand.push_back(deck.front());
		deck.pop_front();
	}
	for (int i = 0; i < 5; i++)
	{
		cpuHand.push_back(deck.front());
		deck.pop_front();
	}
}
bool placeCard(Card card, vector<Card>& hand, Card& cardOnTable)
{
	if (cardOnTable.number == card.number || cardOnTable.type == card.type)
	{
		cardOnTable = card;
		hand.erase(remove_if(hand.begin(), hand.end(), [&](Card const& c) {return c.number == card.number && c.type == card.type; }), hand.end());
		return true;
	}
	return false;
}
void cpuLogic(vector<Card>& cpuHand,Card& cardOnTable, deque<Card>& deck, vector<string> textureNamesNumbers,vector<string>textureNamesTypes, SDL_Renderer* renderer)
{
	for (int i = 0; i < cpuHand.size(); i++)
	{
		Card card = cpuHand[i];
		if (card.type == cardOnTable.type || card.number == cardOnTable.number)
		{
			placeCard(card, cpuHand, cardOnTable);
			return;
		}
	}
	Draw(deck,cpuHand,textureNamesNumbers,textureNamesTypes,renderer);
	return;
}
int main()
{
	bool gameIsRunning = true;
	GameState currentGameState = MAIN_MENU;
	SDL_Init(SDL_INIT_EVERYTHING);
	TTF_Init();
	TTF_Font* font = TTF_OpenFont("font.ttf", 20);
	SDL_Window* win = SDL_CreateWindow("Card Matcher", 100, 100, 1280, 800, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(win,0,0);
	SDL_Surface* buttonSurface = SDL_LoadBMP("Textures/buttonTexture.bmp");
	SDL_Texture* buttonTexture = SDL_CreateTextureFromSurface(renderer, buttonSurface);
	SDL_Surface* exitButtonTextSurface = TTF_RenderText_Solid(font, "Exit", SDL_Color{ 255,255,255 });
	Button exitButton{
		buttonTexture,
		new SDL_Rect{1280/2-50,400+50,100,50},
		SDL_CreateTextureFromSurface(renderer,exitButtonTextSurface),
		[](GameState& state)->void {state = GameState::EXIT; }
	};
	SDL_Surface* startButtonTextSurface = TTF_RenderText_Solid(font, "Start", SDL_Color{ 255,255,255 });
	Button startButton{
		buttonTexture,
		new SDL_Rect{1280/2-50,400-50,100,50},
		SDL_CreateTextureFromSurface(renderer,startButtonTextSurface),
		[](GameState& state)->void {state = GameState::LOAD; }
	};
	SDL_Surface* titleTextSurface = TTF_RenderText_Solid(font, "Card Matcher", SDL_Color{ 255,255,255 });
	Button titleScreenLabel{
		nullptr,
		new SDL_Rect{1280 / 2 - 200,100,400,100},
		SDL_CreateTextureFromSurface(renderer,titleTextSurface),
		nullptr
	};
	SDL_Surface* miniExitTextSurface = TTF_RenderText_Solid(font, "X", SDL_Color{ 255,255,255 });
	Button miniExitButton{
		buttonTexture,
		new SDL_Rect{1280 - 50,0,50,50},
		SDL_CreateTextureFromSurface(renderer,miniExitTextSurface),
		[](GameState& state)->void {state = GameState::MAIN_MENU; }
	};
	SDL_Surface* drawButtonTextSurface = TTF_RenderText_Solid(font, "Draw", SDL_Color{ 255,255,255 });
	Button drawButton{
		buttonTexture,
		new SDL_Rect{900,400,100,50},
		SDL_CreateTextureFromSurface(renderer,drawButtonTextSurface),
		nullptr
	}; 
	SDL_Surface* youWonTextSurface = TTF_RenderText_Solid(font, "You Won!", SDL_Color{ 255,255,255 });
	Button youWonLabel{
		nullptr,
		new SDL_Rect{400,400,300,50},
		SDL_CreateTextureFromSurface(renderer,youWonTextSurface),
		nullptr
	};
	SDL_Surface* youLostTextSurface = TTF_RenderText_Solid(font, "You Lost!", SDL_Color{ 255,255,255 });
	Button youLostLabel{
		nullptr,
		new SDL_Rect{400,400,300,50},
		SDL_CreateTextureFromSurface(renderer,youLostTextSurface),
		nullptr
	};
	vector<Button> mainMenuButtons{exitButton,startButton,titleScreenLabel};
	vector<Button> gameButtons{ miniExitButton };
	vector<string> textureNamesNumbers{"ace","2","3","4","5","6","7","8","9","10","jack","queen","king"};
	vector<string> textureNamesTypes{ "spades","clubs","diamonds","hearts" };
	deque<Card> deck{};
	vector<Card> playerHand{};
	vector<Card> cpuHand{};
	Card cardOnTable;
	while (gameIsRunning)
	{
		SDL_Event e;
		int mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);
		while (SDL_PollEvent(&e))
		{
			switch (currentGameState) {
			case MAIN_MENU:
				for (int i = 0; i < deck.size(); i++)
				{
					SDL_DestroyTexture(deck[i].texture);
				}
				for (int i = 0; i < playerHand.size(); i++)
				{
					SDL_DestroyTexture(playerHand[i].texture);
				}
				deck.clear();
				playerHand.clear();
				if (e.type == SDL_MOUSEBUTTONDOWN)
					for (Button btn : mainMenuButtons)
					{
						if (isWithinRect(btn.rect, mouseX, mouseY))
							if (btn.function != nullptr)
								btn.function(currentGameState);
					}
				break;
			case GAME:
				if (e.type == SDL_MOUSEBUTTONDOWN)
				{
					for (Button btn : gameButtons)
						if (isWithinRect(btn.rect, mouseX, mouseY))
							if (btn.function != nullptr)
								btn.function(currentGameState);
					if (isWithinRect(drawButton.rect, mouseX, mouseY))
					{
						Draw(deck, playerHand, textureNamesNumbers, textureNamesTypes, renderer);
						cpuLogic(cpuHand, cardOnTable, deck, textureNamesNumbers, textureNamesTypes, renderer);
					}
					for (int i = 0; i < playerHand.size(); i++)
					{
						SDL_Rect* destRect = new SDL_Rect{ 100 + i * 100,600,100,140 };
						if (isWithinRect(destRect, mouseX, mouseY))
						{
							if(placeCard(playerHand[i], playerHand, cardOnTable))
								cpuLogic(cpuHand, cardOnTable, deck, textureNamesNumbers, textureNamesTypes, renderer);
						}
						delete destRect;
					}
				}
				if (playerHand.empty())
					currentGameState = WIN;
				if (cpuHand.empty())
					currentGameState = LOSE;
				break;
			case WIN:
				if (e.type == SDL_MOUSEBUTTONDOWN)
				{
					if (isWithinRect(miniExitButton.rect, mouseX, mouseY))
						currentGameState = MAIN_MENU;
				}
				break;
			case LOSE:
				if (e.type == SDL_MOUSEBUTTONDOWN)
				{
					if (isWithinRect(miniExitButton.rect, mouseX, mouseY))
						currentGameState = MAIN_MENU;
				}
				break;
			}
			if (e.type == SDL_QUIT)
				currentGameState = EXIT;
		}
		if (currentGameState == LOAD)
		{
			loadGame(deck, cardOnTable,textureNamesNumbers,textureNamesTypes,renderer,playerHand,cpuHand);
			currentGameState = GAME;
		}
		SDL_RenderClear(renderer);
		switch (currentGameState) {
		case MAIN_MENU:
			for (Button btn : mainMenuButtons)
				renderButton(renderer, btn);
			break;
		case GAME:
			{

			SDL_Surface* noOfCardsSurface = TTF_RenderText_Solid(font, ("Number of cards of your opponent: "+to_string(cpuHand.size())).c_str(), SDL_Color{ 255,255,255 });
			SDL_Texture* noOfCardsTexture = SDL_CreateTextureFromSurface(renderer, noOfCardsSurface);
			SDL_FreeSurface(noOfCardsSurface);
			SDL_Rect* destRect = new SDL_Rect{ 200,100,500,50 };
			SDL_RenderCopy(renderer, noOfCardsTexture, 0, destRect);
			delete destRect;
			for (Button btn : gameButtons)
				renderButton(renderer, btn);
			SDL_Rect* cardOnTableRect = new SDL_Rect{ 1280 / 2 - 50,800 / 2 - 70,100,140 };
			SDL_RenderCopy(renderer, cardOnTable.texture, 0, cardOnTableRect);
			delete cardOnTableRect;
			for (int i =0;i<playerHand.size();i++)
			{
				SDL_Rect* destRect = new SDL_Rect{ 100 + i * 100,600,100,140 };
				SDL_RenderCopy(renderer, playerHand[i].texture, 0, destRect);
				delete destRect;
			}
			renderButton(renderer, drawButton);
			SDL_DestroyTexture(noOfCardsTexture);
			}
			break;
		case WIN:
			renderButton(renderer, miniExitButton);
			renderButton(renderer, youWonLabel);
			break;
		case LOSE:
			renderButton(renderer, miniExitButton);
			renderButton(renderer, youLostLabel);
			break;
		case EXIT:
			gameIsRunning = false;
			break;
		}
		SDL_RenderPresent(renderer);
	}
	TTF_Quit();
	SDL_Quit();
}