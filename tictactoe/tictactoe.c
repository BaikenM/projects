/**
 * @file tictactoe.c
 * @author Baiken Meirambekov (baikenmkv@gmail.com)
 * @brief A basic tictactoe implementation.
 * @version 0.2
 * @date 2022-06-17
 * 
 * @copyright Copyright (c) 2022
 * 
 * RULES: player moves first, AI chooses an option randomly
 * 
 * NEW: 	- Fixed EOF (CTRL-D)
 * 			- Added code documentation
 *			- Refurbished code for better readability and reusability 		
 * 
 * NOTES: Upcoming features:
 * 			- Difficulty (Easy, mideium, hard)
 * 			- Board size (any given, minimum 3x3)
 * 			- First move (player, AI, random)
 * 			- Board draw size (custom UI)
 * 			- Simulate AI response time (1-2 secs), loadscreen/loadbar
 * 			- Better (prettier) CLI output
 * 			- Tip which cells are empty
 * 			- Choose custom marks
 * 			- Player vs Player mode
 * 
 * TODO: 	- In-game rules
 *			- Add game statistics to playGame()
 *			- Optimize line detection
 *			- Optimize AI - record occupied cells, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "gamestate.h"
#include "nextmove.h"

#define MAX_BOARD_DIM 7

typedef char*** pBoard_t;

/**
 * @brief Struct to store information relevant to the game.
 */
typedef struct {
	eGameState m_CurrentState;
	eNextMove  m_NextMove;
	size_t 	   m_Dim;
	char	 **m_Board;
	int  	   m_MovesLeft;
} GameSession;

/**
 * @brief Check whether the given number is within a certain range.
 * 
 * @param number Target
 * @param begin	 Range begin
 * @param end	 Range end
 *
 * @return 1, if number falls within the range
 */
int isInRange(char number, int begin, int end) {
	return ((number >= begin + 48) && (number <= end + 48));
}

/**
 * @brief Deallocate resources before exiting.
 * 
 * @param status Exit status.
 */
void exitGame(GameSession *TheGame, int status) {
	for (int i = 0; i < MAX_BOARD_DIM; i++) {
		free((TheGame->m_Board)[i]);
	}
	free(TheGame->m_Board);

	if (status)
		exit(EXIT_SUCCESS);
	else
		exit(EXIT_FAILURE);
}

/**
 * @brief Get Player's input and validate. The input is expected to be a single
 * 		  digit character inside the range. Upon failing to get the valid input
 * 		  prompt Player to try again.
 * 
 * @param rangeBegin Minimum expected digit
 * @param rangeEnd   Maximum expected digit
 * @return -1, if EOF is encountered
 * 		     0-9, if input is valid  
 */
int getPlayerInput(int rangeBegin, int rangeEnd, GameSession *TheGame) {
	char number;
	char lineEnd;

	while (1) {
		number = getchar();
		
		/* Abort game */
		if (number == EOF) {
			exitGame(TheGame, 0);
		}

		/* Force a single char to be entered as input.
		The number must be a digit that falls within range.
		The char following the number must be a newline character */
		if (!isdigit(number) || 
		!isInRange(number, rangeBegin, rangeEnd) ||
		(lineEnd = getchar()) != '\n') {
			printf("Please, enter a valid option [%d-%d]\n", rangeBegin, rangeEnd);
			
			/* Skip every char from stdin buffer */
			while ((lineEnd = getchar()) != '\n' && lineEnd != EOF);
			continue;
		}
		return (int)number - 48;
	}
}

/**
 * @brief Reset the game, nullifying all board cells 
 *        and defaulting the game state
 */
void resetGame(GameSession *TheGame) {
	for (int i = 0; i < TheGame->m_Dim; i++)
		for (int j = 0; j < TheGame->m_Dim; j++)
			(TheGame->m_Board)[i][j] = 0;

	TheGame->m_CurrentState = PLAY;
	TheGame->m_MovesLeft = TheGame->m_Dim * TheGame->m_Dim;
	TheGame->m_NextMove = PLAYER_MOVE; 
}

/**
 * @brief Draw Main Menu screen, allowing Player to start the game,
 * 		  change settings or exit program.
 */
void drawMenuScreen(GameSession *TheGame) {
	int    playerSelect;	

	printf("Welcome to TictacToe!\n");
	printf("1) Play\n");	
	printf("2) Settings\n");	
	printf("3) Exit\n");	

	playerSelect = getPlayerInput(1, 3, TheGame);

	switch (playerSelect) {
		case 1:
			TheGame->m_CurrentState = PLAY;
			break;
		case 2:
			TheGame->m_CurrentState = SETTINGS;
			break;
		case 3:
			TheGame->m_CurrentState = EXIT;
			break;
		default:
			printf("Unexpected input. State: %d\n", TheGame->m_CurrentState);
			break;
	}
}

/**
 * @brief TODO
 * 
 */
void drawSettingsScreen(GameSession *TheGame) {
	printf("Settings are not yet implemented. Returning to main menu...\n");
	TheGame->m_CurrentState = MENU;
}

/**
 * @brief Print out the board in its current state. TODO
 * 
 */
void drawBoard(pBoard_t board, size_t dim) {
	for (int i = 0; i < dim; i++) {
		for(int k = 0; k < dim; k++) {
			printf("+-");
		}
		printf("+\n");

		for (int j = 0; j < dim; j++) {
			printf("|");
			if ((*board)[i][j]) {
				printf("%c", (*board)[i][j]);
			} else {
				printf(" ");
			}
		}
		printf("|\n");
	}
	
	for(int k = 0; k < dim; k++) {
		printf("+-");
	}
	printf("+\n");
}

/**
 * @brief Based on input, get the cell position on board.
 * 		  If the cell is empty, mark it. 
 * 
 * @param pos Player input, used to calculate the desired cell position
 *
 * @return 1, if cell was empty
 */
int checkCell(GameSession *TheGame, int pos, char playerSymbol) {
	int x, y;

	pos--;
	x = pos / TheGame->m_Dim;
	y = pos % TheGame->m_Dim;

	if ((TheGame->m_Board)[x][y])
		return 0;
	
	(TheGame->m_Board)[x][y] = playerSymbol;
	return 1; 
}

/**
 * @brief Pause the game. Prompt user for following instructions.
 */
void processPauseState(GameSession *TheGame) {
	int    playerSelect;	
	
	printf("Game paused.\n");
	printf("1) Continue\n");	
	printf("2) Restart\n");	
	printf("3) Menu\n");	
	
	playerSelect = getPlayerInput(1, 3, TheGame);

	switch (playerSelect)
	{
	case 1:
		printf("Game resumed.\n");
		drawBoard(&TheGame->m_Board, TheGame->m_Dim);
		break;
	case 2:
		TheGame->m_CurrentState = RESTART;
		break;
	case 3:
		TheGame->m_CurrentState = MENU;
		break;
	}
}

/**
 * @brief Pause the game or keep prompting Player to choose
 * 		  an empty cell to mark, based on Player input.
 */
void getPlayerMove(GameSession *TheGame) {
	int    playerSelect;	

	while (TheGame->m_CurrentState == PLAY) {
		printf("Your turn!\n");
		printf("Pick a cell [1-9] or Pause [0].\n");

		playerSelect = getPlayerInput(0, 9, TheGame);

		/* Player paused the game.
		Skip rest of the loop to get Player input again. */
		if (!playerSelect){
			processPauseState(TheGame);
			continue;
		}
		
		/* If cell was empty, mark it, otherwise prompt Player again. */
		if (checkCell(TheGame, playerSelect, 'X' )) {
			break;
		}
		printf("Please, pick an empty cell.\n");
	}
}

/**
 * @brief Simple AI behaviour simulation.
 */
void getAImove(GameSession *TheGame) {
	time_t t;
	int AIchoice;

	srand((unsigned) time(&t));

	do {
		AIchoice = rand() % (TheGame->m_Dim * TheGame->m_Dim);
	} while (!checkCell(TheGame, AIchoice, 'O'));
}

/**
 * @brief Search for complete (comprised of a single symbol) lines in the board.
 * 
 * @param board Pointer to 2D board.
 * @param dim Dimensions of given board.
 *
 * @return 1, if line is detected
 */
int detectedLine(pBoard_t board, size_t dim) {
	int foundLine;

	for (int i = 0; i < dim; i++) {
		foundLine = 1;
		for (int j = 0; j < dim - 1; j++) {
			/* if does not contain same symbol in adjacent cell on same line 
			or the cell does not contain neither X nor O */
			if ((*board)[i][j] != (*board)[i][j + 1] || !(*board)[i][j]) {
				foundLine = 0;
				break;
			}  
		} 
		if (foundLine) return 1;
	} /* Rows */

	for (int i = 0; i < dim; i++) {
		foundLine = 1;
		for (int j = 0; j < dim - 1; j++) {
			if ((*board)[j][i] != (*board)[j + 1][i] || !(*board)[j][i]) {
				foundLine = 0;
				break;
			}
		}
		if (foundLine) return 1;
	} /* Columns */

	foundLine = 1;
	for (int i = 0; i < dim - 1; i++) {
		if ((*board)[i][i] != (*board)[i + 1][i + 1] || !(*board)[i][i]) {
			foundLine = 0;
			break;
		}
	} /* first diagonal */
	if (foundLine) return 1;

	foundLine = 1;
	for (int i = 0; i < dim - 1; i++) {
		if ((*board)[i][dim - 1 - i] != (*board)[i + 1][dim - 2 - i] || !(*board)[i][dim - 1 - i]) {
			foundLine = 0;
			break;
		}
	} /* second diagonal */
	if (foundLine) return 1;
	return 0;
}

/**
 * @brief Process turn by taking input from Player or simulating AI decision.
 * 		  Detect a win/stalemate condition, in which case finish the game.
 */
void processTurn(GameSession *TheGame) {
	switch (TheGame->m_NextMove) {
	case PLAYER_MOVE:
		TheGame->m_NextMove = AI_MOVE;
		getPlayerMove(TheGame);
		break;

	case AI_MOVE:
		TheGame->m_NextMove = PLAYER_MOVE;
		getAImove(TheGame);
		break;
	}

	/* Restart game */
	if (TheGame->m_CurrentState != PLAY)
		return;

	/* If a complete line is detected, change game state and abort */
	if (detectedLine(&TheGame->m_Board, TheGame->m_Dim)) {
		if (TheGame->m_NextMove == AI_MOVE)
			TheGame->m_CurrentState = PLAYER_WIN;
		else
			TheGame->m_CurrentState = AI_WIN;
		return;
	}

	/* Set stalemate if there is no winner and there are no empty cells left */
	if (--TheGame->m_MovesLeft == 0)
		TheGame->m_CurrentState = STALEMATE;
}

/**
 * @brief Conclude the game printing the result of the game.
 * 		  Restart or go back to main menu.
 */
void drawGameoverScreen(GameSession *TheGame) {
	switch (TheGame->m_CurrentState) {
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
			printf("Unexpected game state\n");
			break;
	}

	printf("1) Play again\n");	
	printf("2) Menu\n");	

	if (getPlayerInput(1, 2, TheGame) == 1) 
		TheGame->m_CurrentState = PLAY;
	else
		TheGame->m_CurrentState = MENU;
}

/**
 * @brief Check whether the game is over
 * 
 * @return 1, if gameover
 */
int isGameOver(eGameState state) {
	return ((state == PLAYER_WIN) || (state == AI_WIN) || (state == STALEMATE));
}

/**
 * @brief Set the game. Process Player and AI turns and draw the board each turn.
 * 		  Draw the "Game Over" screen upon the end of the game.
 * 
 * @param TheGame An instance of a running game
 */
void playGame(GameSession *TheGame) {
	TheGame->m_NextMove = PLAYER_MOVE;
	resetGame(TheGame);
	drawBoard(&TheGame->m_Board, TheGame->m_Dim);

	printf("Game starts.\n");

	while (TheGame->m_CurrentState == PLAY) {
		processTurn(TheGame);
		
		if (TheGame->m_CurrentState == PLAY)
			drawBoard(&TheGame->m_Board, TheGame->m_Dim);
	}

	/* Draw the screen, if Player did not restart the game */
	if (isGameOver(TheGame->m_CurrentState))
		drawGameoverScreen(TheGame);
}

/**
 * @brief Update the game based on its state
 * 
 */
void updateGame(GameSession *TheGame) {
	switch(TheGame->m_CurrentState) {
		case MENU:
			drawMenuScreen(TheGame);
			break;
		case SETTINGS:
			drawSettingsScreen(TheGame);
			break;
		case PLAY:
			playGame(TheGame);
			break;
		case RESTART:
			resetGame(TheGame);
			playGame(TheGame);
			break;
		default:
			printf("Unexpected game state.\n");
			break;
	}
}

/**
 * @brief Set default game settings
 * 
 * @return 1, if game was successfully initialized
 */
int initGame(GameSession *TheGame) {
	/* TODO: Multiple malloc calls is deemed inefficient, consider pointer to VLA.
	A somewhat sloppy approach to memory management. */
	TheGame->m_CurrentState = MENU;
	TheGame->m_NextMove     = PLAYER_MOVE;
	TheGame->m_Dim  	    = 3;
	TheGame->m_MovesLeft	= TheGame->m_Dim * TheGame->m_Dim;
	TheGame->m_Board 	    = malloc(MAX_BOARD_DIM * sizeof(char*));
	
	/* Check if malloc failed */
	if (!TheGame->m_Board) {
		return 0;
	}

	/* Allocate enough memory for 7x7 board. Board dimensions can be set up later. */
	for (int i = 0; i < MAX_BOARD_DIM; i++) {
		TheGame->m_Board[i] = malloc(MAX_BOARD_DIM * sizeof(char));
		
		/* Check if malloc failed */
		if (!TheGame->m_Board[i]) {
			for (int j = 0; j < i; j++) {
				free(TheGame->m_Board[j]);
			}
			free(TheGame->m_Board);
			return 0;
		}
	}
	return 1;
}

int main(int argv, char **argc) {
	GameSession TheGame;
	if (!initGame(&TheGame)) {
		printf("Could not allocate enough memory.\n");
		return 1;
	}

	/* Game state loop */
	while (TheGame.m_CurrentState != EXIT) {
		updateGame(&TheGame);	
	}
	exitGame(&TheGame, 1);
	return 0;
}