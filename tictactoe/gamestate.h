#ifndef __GAME_STATE_H__
#define __GAME_STATE_H__

enum GameState {
	MENU,
	SETTINGS,
	PLAY,
	PAUSE,
  PLAYER_WIN,
  AI_WIN,
  STALEMATE
};

typedef enum GameState eGameState;

#endif