/**
 * @file tictactoe.c
 * @author Baiken Meirambekov (baikenmkv@gmail.com)
 * @brief A basic tictactoe implementation.
 * @version 0.1
 * @date 2022-06-16
 * 
 * @copyright Copyright (c) 2022
 * 
 * RULES: player moves first, AI chooses an option randomly
 * NOTES: needs additional features for game improvement such as
 * 					- Difficulty (Easy, mideium, hard)
 * 					- Board size (any given, minimum 3x3)
 * 					- First move (player, AI, random)
 * 					- Board draw size
 * 					- Fix EOF
 * 					- Simulate AI response time (1-2 secs), loadscreen/loadbar
 * 					- Better (prettier) CLI output
 * 					- Tip which cells are empty
 * 					- Code documentation
 * 					- Parse condition in getPlayerMove() and overall polish of code
 * 					- Rules of the game
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

#include "gamestate.h"
#include "nextmove.h"

eGameState gCurrentState;
eNextMove  gNextMove;
const int  BOARDSIZE = 3;

void resetBoard(char (*board)[BOARDSIZE][BOARDSIZE]) {
	for (int i = 0; i < BOARDSIZE; i++) {
		for (int j = 0; j < BOARDSIZE; j++) {
			(*board)[i][j] = 0;
		}
	}
}

/** Main Menu, get player input to set up the game
*
**/
void drawMenuScreen(bool* isRunning, char (*board)[BOARDSIZE][BOARDSIZE]) {
	int    playerSelect;	
	char*  buf = NULL;
	size_t len;

	printf("Welcome to TictacToe!\n");
	printf("1) Play\n");	
	printf("2) Settings\n");	
	printf("3) Exit\n");	

	clearerr(stdin);
	while (getline(&buf, &len, stdin) && !feof(stdin)) {
		if (sscanf(buf, "%d", &playerSelect) != 1 ||
		    //sscanf(buf, "%c", &rem) != 1 || rem != '\n' ||
		   (playerSelect != 1 &&
		    playerSelect != 2 &&
		    playerSelect != 3 )) {

			printf("Please, choose correct option [1-3].\n");
		} else break;
	}
	free(buf);

	switch (playerSelect) {
		case 1:
			gCurrentState = PLAY;
			break;
		case 2:
			gCurrentState = SETTINGS;
			break;
		case 3:
			*isRunning = false;
			break;
		default:
			printf("Unexpected input. State: %d\n", gCurrentState);
			break;
	}
}

void drawSettingsScreen() {
	printf("Settings are not yet implemented. Returning to main menu...\n");
	gCurrentState = MENU;
}

void drawBoard(char board[BOARDSIZE][BOARDSIZE]) {
	for (int i = 0; i < BOARDSIZE; i++) {
		for(int k = 0; k < BOARDSIZE; k++) {
			printf("+-");
		}
		printf("+\n");

		for (int j = 0; j < BOARDSIZE; j++) {
			printf("|");
			if (board[i][j]) {
				printf("%c", board[i][j]);
			} else {
				printf(" ");
			}
		}
		printf("|\n");
	}
	
	for(int k = 0; k < BOARDSIZE; k++) {
		printf("+-");
	}
	printf("+\n");
}

bool checkCell(int pos, char (*board)[BOARDSIZE][BOARDSIZE], char playerSymbol) {
	if(!pos) return true;
	
	int x, y;

	pos--;
	x = pos / 3;
	y = pos % 3;

	if ((*board)[x][y]) {
		return false;
	} // Cell is not empty
	(*board)[x][y] = playerSymbol;
	return true; 
}

bool processPauseState(char (*board)[BOARDSIZE][BOARDSIZE]) {
	int    playerSelect;	
	char*  buf = NULL;
	size_t len;
	
	printf("Game paused.\n");
	printf("1) Continue\n");	
	printf("2) Restart\n");	
	printf("3) Menu\n");	
	
	while (getline(&buf, &len, stdin) && !feof(stdin)) {
		if (sscanf(buf, "%d", &playerSelect) != 1 ||
		    //sscanf(buf, "%c", &rem) != 1 || rem != '\n' ||
		   (playerSelect != 1 &&
		    playerSelect != 2 &&
				playerSelect != 3 )) {

			printf("Please, choose a correct option [1-3].\n");
		} else break;
	}
	free(buf);

	if (playerSelect == 1) {
		printf("Game resumed.\n");
		gCurrentState = PLAY;
		gNextMove = PLAYER_MOVE;
		return true;
	} // CONTINUE (1)
	else {
		if (playerSelect == 2) {
			gCurrentState = PLAY;
		} // RESTART (2)
		else {
			gCurrentState = MENU;
		} // MAIN MENU (3)
	}	
	return false;
}

bool getPlayerMove(char (*board)[BOARDSIZE][BOARDSIZE], int* movesLeft) {
	int    playerSelect;	
	char*  buf = NULL;
	size_t len;
	bool   cellEmpty;
	int 	 inputFail;

	printf("Your turn!\n");
	printf("Pick a cell [1-9] or Pause [0].\n");

	while (getline(&buf, &len, stdin) && !feof(stdin)) {
		if ((inputFail = sscanf(buf, "%d", &playerSelect)) != 1 ||
			//sscanf(buf, "%c", &rem) != 1 || rem != '\n' ||
			playerSelect < 0 || playerSelect > 9 || 
			!(cellEmpty = checkCell(playerSelect, board, 'X' ))) {

				if (!inputFail || playerSelect < 0 || playerSelect > 9) {
					printf("Please, choose correct option [0-9].\n");
					continue;
				}
				if (!cellEmpty) {
					printf("Please, choose an empty cell.\n");
					continue;
				}
		} else break;
	}
	free(buf);
	// printf("Here\n");

	if (!playerSelect) {
		++*movesLeft;
		if (!processPauseState(board)) {
			return false;
		}
	} // Player chose to pause the game (0) and restart/go back to main menu
	return true;
}

void getAImove(char (*board)[BOARDSIZE][BOARDSIZE]) {
	time_t t;
	int AIchoice;

	srand((unsigned) time(&t));

	do {
		AIchoice = rand() % 8 + 1;
	} while (!checkCell(AIchoice, board, 'O'));
}

bool detectedLine(char board[BOARDSIZE][BOARDSIZE]) {
	bool foundLine;

	for (int i = 0; i < BOARDSIZE; i++) {
		foundLine = true;
		for (int j = 0; j < BOARDSIZE - 1; j++) {
			if (board[i][j] != board[i][j + 1] || !board[i][j]) {
				foundLine = false;
				break;
			} // doesn't contain same symbol as adjacent cell on same line, then check if cell contains X or O 
		} 
		if (foundLine) return true;
	} // rows

	for (int i = 0; i < BOARDSIZE; i++) {
		foundLine = true;
		for (int j = 0; j < BOARDSIZE - 1; j++) {
			if (board[j][i] != board[j + 1][i] || !board[j][i]) {
				foundLine = false;
				break;
			}
		}
		if (foundLine) return true;
	} // columns

	foundLine = true;
	for (int i = 0; i < BOARDSIZE - 1; i++) {
		if (board[i][i] != board[i + 1][i + 1] || !board[i][i]) {
			foundLine = false;
			break;
		}
	} // first diagonal
	if (foundLine) return true;

	foundLine = true;
	for (int i = 0; i < BOARDSIZE - 1; i++) {
		if (board[i][BOARDSIZE - 1 - i] != board[i + 1][BOARDSIZE - 2 - i] || !board[i][BOARDSIZE - 1 - i]) {
			foundLine = false;
			break;
		}
	} // second diagonal
	if (foundLine) return true;
	return false;
}

bool processTurn(char (*board)[BOARDSIZE][BOARDSIZE], int* movesLeft) {
	if(gNextMove == PLAYER_MOVE) {
		gNextMove = AI_MOVE;
		if (!getPlayerMove(board, movesLeft)) {
			return false;
		} // restart or back to main menu

		if (detectedLine(*board)) {
			gCurrentState = PLAYER_WIN;
		} // found a line with same symbols
	} // process player's move
	else {
		gNextMove = PLAYER_MOVE;
		getAImove(board);
		if (detectedLine(*board)) {
			gCurrentState = AI_WIN;
		}
	} // process AI's move

	if (!--*movesLeft && gCurrentState == PLAY) {
		gCurrentState = STALEMATE;
	} // set stalemate, if no more moves are left and no one won this turn
	return true;
}

void drawGameoverScreen() {
	int    playerSelect;	
	char*  buf = NULL;
	size_t len;

	switch (gCurrentState) {
		case PLAYER_WIN:
			printf("You win!\n");
			break;
		case AI_WIN:
			printf("You lose!\n");
			break;
		case STALEMATE:
			printf("It's a stalemate!\n");
			break;
		default:
			break;
	}

	printf("1) Play again\n");	
	printf("2) Menu\n");	

	while (getline(&buf, &len, stdin) && !feof(stdin)) {

		if (sscanf(buf, "%d", &playerSelect) != 1 ||
		    //sscanf(buf, "%c", &rem) != 1 || rem != '\n' ||
		   (playerSelect != 1 &&
		    playerSelect != 2 )) {

			printf("Please, choose a correct option [1-2].\n");
		} else break;
	}
	free(buf);

	if (playerSelect == 1) {
		gCurrentState = PLAY;
	} else {
		gCurrentState = MENU;
	}
}

void playGame(bool* playerWin, char (*board)[BOARDSIZE][BOARDSIZE]) {
	int movesLeft = 9; // given board size is 3x3
	gNextMove			= PLAYER_MOVE;

	printf("Game starts.\n");

	resetBoard(board);
	drawBoard(*board);

	while (gCurrentState == PLAY) {
		if (!processTurn(board, &movesLeft)) {
			return;
		} // Player chose to restart or go back to main menu, ending this game's loop
		if (gCurrentState == PLAYER_WIN) {
			break;
		} // Player built a line this turn
		if(movesLeft) {
			processTurn(board, &movesLeft);
		}
		drawBoard(*board);
	} // game loop
	drawGameoverScreen();
}

void updateGame(bool* gameIsRunning, char (*board)[BOARDSIZE][BOARDSIZE]) {
	bool playerWin;
	
	switch(gCurrentState) {
		case MENU:
			drawMenuScreen(gameIsRunning, board);
			break;
		case SETTINGS:
			drawSettingsScreen();
			break;
		case PLAY:
			playGame(&playerWin, board);
			break;
		
		default:
			printf("Unexpected game state.\n");
			break;
	}
}

int main(int argv, char **argc) {
	char board[BOARDSIZE][BOARDSIZE];
	bool gameIsRunning = true;
	gCurrentState 		 = MENU;

	while (gameIsRunning) {
		updateGame(&gameIsRunning, &board);	
	} // game state loop
	return 0;
}