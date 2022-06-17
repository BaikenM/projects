#ifndef __GAME_STATE_H__
#define __GAME_STATE_H__

enum GameState {
	MENU,
	SETTINGS,
	PLAY,
	PAUSE,
	RESTART,
  PLAYER_WIN,
  AI_WIN,
  STALEMATE,
	EXIT
};

typedef enum GameState eGameState;

#endif