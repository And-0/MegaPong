#include <genesis.h>
#include <string.h>
#include "resources.h"

/*The sprites we'll need*/
Sprite* player;
Sprite* ball;

/*Variables for the player*/
int player_pos_x = 144;
const int player_pos_y = 200;
const int player_width = 32;
const int player_height = 8;
int player_vel_x = 0;

/*The edges of the play field*/
const int LEFT_EDGE = 0;
const int RIGHT_EDGE = 320;
const int TOP_EDGE = 0;
const int BOTTOM_EDGE = 240;

/*Variables to store the position and velocity of the ball*/
int ball_pos_x = 160-4;
int ball_pos_y = 100;
int ball_vel_x = 0;
int ball_vel_y = 0;
int ball_width = 8;
int ball_height = 8;

/*Track the game state*/
ball_launched = FALSE;
game_on = FALSE;

/*Score variables*/
int score = 0;
char label_score[6] = "SCORE\0";
char str_score[3] = "0";

/*Messages to be displayed*/
char msg_start[22] = "Press START to Begin!\0";
char msg_reset[37] = "Game over! Press START to Play Again.";

/*A small helper function to get the sign of an int*/
int sign(int x) {
    return (x > 0) - (x < 0);
}

void positionPlayer(){
	/*Add the player's velocity to its position*/
	player_pos_x += player_vel_x;

	/*Keep the player within the bounds of the screen*/
	if(player_pos_x < LEFT_EDGE) player_pos_x = LEFT_EDGE;
	if(player_pos_x + player_width > RIGHT_EDGE) player_pos_x = RIGHT_EDGE - player_width;

	/*Let the Sprite engine position the sprite*/
	SPR_setPosition(player,player_pos_x,player_pos_y);
}

/*Draws text in the center of the screen*/
void showText(char s[]){
	VDP_drawText(s, 20 - strlen(s)/2 ,15);
}

/*Draws the current score onto the screen*/
void updateScoreDisplay(){
	sprintf(str_score,"%d",score);
	VDP_clearText(1,2,3);
	VDP_drawText(str_score,1,2);
}

void endGame(){
	showText(msg_reset);
	game_on = FALSE;
	ball_launched = FALSE;
}

void launchBall(){
	if(ball_launched) return;

	score = 0;
	updateScoreDisplay();
	ball_pos_x = 0;
	ball_pos_y = 0;
	ball_launched = TRUE;
	ball_vel_x = 2;
	ball_vel_y = 2;

	/*Clear the text from the screen*/
	VDP_clearTextArea(0,10,40,10);
}

/*Move the ball and handle collisions*/
void moveBall(){
	if(!ball_launched) return;

	/*Add the velocity to the ball's position*/
	ball_pos_x += ball_vel_x;
	ball_pos_y += ball_vel_y;

	/*Check for collisions with the player paddle*/
	if(ball_pos_x < player_pos_x + player_width && ball_pos_x + ball_width > player_pos_x){
		if(ball_pos_y < player_pos_y + player_height && ball_pos_y + ball_height >= player_pos_y){
			/*If there is a collision, position the ball above the player
			 *and invert the velocity */
			ball_pos_y = player_pos_y - ball_height - 1;
			ball_vel_y = -ball_vel_y;

			/*Increase the score and update the display*/
			score++;
			updateScoreDisplay();

			/*Every 10 hits increase the speed of the ball*/
			if( score % 10 == 0){
				ball_vel_x += sign(ball_vel_x);
				ball_vel_y += sign(ball_vel_y);
			}
		}
	}
	
	/*Keep the ball within the screen bounds by making it bounce off;
	 *if it leaves the screen at the bottom, end the game*/
	if(ball_pos_x < LEFT_EDGE){
		ball_pos_x = LEFT_EDGE;
		ball_vel_x = -ball_vel_x;
	} else if(ball_pos_x + ball_width > RIGHT_EDGE){
		ball_pos_x = RIGHT_EDGE - ball_width;
		ball_vel_x = -ball_vel_x;
	}

	if(ball_pos_y < TOP_EDGE){
		ball_pos_y = TOP_EDGE;
		ball_vel_y = -ball_vel_y;
	} else if(ball_pos_y + ball_height > screenHeight){
		endGame();
	}

	/*Let the Sprite engine position the ball*/
	SPR_setPosition(ball,ball_pos_x,ball_pos_y);
	
}

/*The callback function that handles Joypad input*/
void myJoyHandler( u16 joy, u16 changed, u16 state)
{
	if (joy == JOY_1)
	{
		/*If a button has been pressed, and that button is the Start Button...*/
		if(state & BUTTON_START){
			/*Start the game if it's not currently going*/
			if(!game_on){
				game_on = TRUE;
				launchBall();
			}
		}
		
		/*Set player velocity if left or right are pressed;
		 *set velocity to 0 if no direction is pressed */
		else if (state & BUTTON_RIGHT)
		{
			player_vel_x = 3;
		}
		else if (state & BUTTON_LEFT)
		{
			player_vel_x = -3;
		} else{
			if( (changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT) ){
				player_vel_x = 0;
			}
		}
	}
}

/*The main function where the cool things happen!*/
int main()
{
	SYS_disableInts();

	/*Set up the controller*/
	JOY_init();
	JOY_setEventHandler( &myJoyHandler );

	/*Load and draw the background*/
	VDP_loadBMPTileData(tile.image,1,1,1,1);
	VDP_setPalette(PAL1, tile.palette->data);
	VDP_fillTileMapRect(PLAN_A,TILE_ATTR_FULL(PAL1,0,FALSE,FALSE,1),0,0,40,30);

	SYS_enableInts();

	/*Draw the texts*/
	VDP_setTextPlan(PLAN_B);/*Set the text plane to Plane B so texts are drawn above the tiles*/
	VDP_drawText(label_score,1,1);
	updateScoreDisplay();
	showText(msg_start);
	VDP_drawText("MEGAPONG",16,10);

	/*Add the sprites for the player and ball*/
	SPR_init(0,0,0);
	player = SPR_addSprite(&paddle, player_pos_x, player_pos_y, TILE_ATTR(PAL1, 0, FALSE, FALSE));
	ball = SPR_addSprite(&imgball, ball_pos_x,ball_pos_y,TILE_ATTR(PAL1,0, FALSE, FALSE));
	SPR_update();

	/*Game loop*/
	while(1)
	{
		if(game_on){
			moveBall();
			positionPlayer();
		}
		
		SPR_update();
		VDP_waitVSync();
	}

	return(0);
}







