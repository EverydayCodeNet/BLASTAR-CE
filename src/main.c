 /*
 *--------------------------------------
 * Program Name: BLASTAR
 * Author: Everyday Code
 * License:
 * Description: Remake of Elon Musk's 1984 game Blastar for the TI-84 Plus CE Calculator.
 *--------------------------------------
*/

#include <math.h>
#include <tice.h>
#include <keypadc.h>
#include <graphx.h>
#include <fileioc.h>

// converted graphics files
#include "gfx/gfx.h"


kb_key_t key;

typedef struct {
    uint8_t ships;
    int score;
    int x,y;

    //is bullet active
    bool fired;
    int bulletX,bulletY;
    
    bool endgame;
} player_t;
player_t player;

typedef struct {
    int x, y;
    bool fired;

    //status beam - send alert
    int bulletX, bulletY;

    //right to left by default
    bool dir;

    //distance to player
    int distance;
} enemy_t;
enemy_t enemy; 

void WhiText(void) {
    gfx_SetTextBGColor(0);
    gfx_SetTextFGColor(255);
    gfx_SetTextTransparentColor(0);
}

void YelText(void) {
    gfx_SetTextBGColor(0);
    gfx_SetTextFGColor(229);
    gfx_SetTextTransparentColor(0);
}

void showInstructions(void);
void doAI(void);
void endScreen(void);

void createPlayer() {
    player.endgame = false;
    player.x = randInt(0,290);
    player.y = randInt(0,210);
    player.score = 0;
    player.fired = false;
    player.ships = 5;

    enemy.fired = false;
}

//basic title sequence
void mainMenu() {
    bool instruct, keypressed = false;

    WhiText();
    gfx_SetTextScale(3,3);
    gfx_SetDrawScreen();

    delay(250);
    gfx_PrintStringXY("BLASTAR",160 - gfx_GetStringWidth("BLASTAR") / 2,30);
    delay(750);

    gfx_SetTextScale(1,1);
    gfx_PrintStringXY("BY E.R. MUSK",160 - gfx_GetStringWidth("BY E.R. MUSK")/2,65);
    delay(500);

    gfx_PrintStringXY("RECREATED BY EVERYDAY CODE",160 - gfx_GetStringWidth("RECREATED BY EVERYDAY CODE")/2,80);
    delay(750);
    gfx_ScaledTransparentSprite_NoClip(player_ship,160-30,115, 4,4);
    gfx_SetTextScale(2,2);
    gfx_PrintStringXY("PRESS [2ND]",160 - gfx_GetStringWidth("PRESS [2ND]]")/2,200);
    
    do {
        kb_Scan();
    } while (!(kb_Data[1] & kb_2nd));

    gfx_ZeroScreen();

    //prompt player to continue game
    do {
        kb_Scan();
        WhiText();
        gfx_SetTextScale(3,3);
        gfx_PrintStringXY("BLASTAR",160 - gfx_GetStringWidth("BLASTAR") / 2,30);
        gfx_SetTextScale(1,1);
        gfx_PrintStringXY("DO YOU NEED INSTRUCTIONS",160 - gfx_GetStringWidth("DO YOU NEED INSTRUCTIONS")/2, 65);
        gfx_SetTextScale(2,2);
        gfx_PrintStringXY("Y [1] / [0] N", 160 - gfx_GetStringWidth("Y [1] / [0] N")/2,200);
        if (kb_Data[3]) {
            keypressed = true;
            if (kb_Data[3] == kb_1) {
                instruct = true;
            } else {
                instruct = false;
            }
        }
    } while(keypressed == false);

    if (instruct == true) {
        showInstructions();
    }
}

void showInstructions() {
    gfx_ZeroScreen();
    //wait for player to answer prompt
    do {
        kb_Scan();
        gfx_SetTextScale(2,2);
        WhiText();
        gfx_PrintStringXY("USE ARROW KEYS",160 - gfx_GetStringWidth("USE ARROW KEYS")/2,30);
        gfx_PrintStringXY("FOR CONTROL AND",160 - gfx_GetStringWidth("FOR CONTROL AND")/2,50);
        gfx_PrintStringXY("ENTER KEY",160 - gfx_GetStringWidth("ENTER KEY")/2,70);
        gfx_PrintStringXY("TO SHOOT",160 - gfx_GetStringWidth("TO SHOOT")/2,90);
        

        gfx_SetTextScale(1,1);
        gfx_PrintStringXY("MISSION: DESTROY ALIEN FREIGHTER",50,130);
        gfx_PrintStringXY("CARRYING DEADLY HYDROGEN BOMBS",50,150);
        gfx_PrintStringXY("AND STATUS BEAM MACHINES",50,170);

        gfx_SetTextScale(2,2);
        YelText();
    gfx_PrintStringXY("PRESS [2ND]",160 - gfx_GetStringWidth("PRESS [2ND]]")/2,200);
    } while (!(kb_Data[1] & kb_2nd));
}

void doMovement() {
    kb_Scan();
    key = kb_Data[7];
    //player can move when status beam is not active
    if (enemy.fired == false) {
        if (key & kb_Up && player.y > 0) {
            player.y -= 2;
        } else if (key & kb_Down && player.y + 30 < 240) {
            player.y += 2;
        } else if (key & kb_Left && player.x > 0) {
            player.x -= 2;
        } else if (key & kb_Right && player.x + 30 < 320) {
            player.x += 2;
        }
    }
    
    if (kb_Data[6] & kb_Clear) {
        //do not prompt to restart game
        player.endgame = true;
    }

    //if enemy hits other side of screen, go the other way
    if (enemy.x >= 290 && enemy.dir == false) {
        enemy.dir = true;
    } else if (enemy.x <= 0 && enemy.dir == true) {
        enemy.dir = false;
    }

    //update enemy's position based on direction
    if (enemy.dir == true) {
        enemy.x -= 2;
    } else {
        enemy.x += 2;
    }

    //update bullet position if fired
    if (player.fired == true) {
        player.bulletY -= 3;
        if (player.bulletY <= 0) {
            player.fired = false;
        }
    }

    //update status beam position if fired
    if (enemy.fired == true) {
        enemy.bulletY += 2;
        if (enemy.bulletY >= 240) {
            enemy.fired = false;
        }
    }
}

void doCollisions() {
    //find distance, bullet distance, status beam distance
    int distance,bdistance,sdistance;

    //get x/y of the sprites' center
    int x1 = player.x - 15;
    int y1 = player.y - 15;
    int x2 = enemy.x - 15;
    int y2 = enemy.y -15;
    int xsqr,ysqr;

    //written outside of distance assignment for simplicity
    xsqr = (x2-x1) * (x2-x1);
    ysqr = (y2-y1) * (y2-y1);

    //distance formula 
    distance = sqrt(xsqr + ysqr);
    enemy.distance = distance;
    sdistance = sqrt(pow((x1 - enemy.bulletX),2) + pow((y1 - enemy.bulletY),2));
    bdistance = sqrt(pow((x2 - player.bulletX),2) + pow((y2 - player.bulletY),2));

    //if player is less than 100px away, trigger enemy AI
    if (distance <= 100) {
        doAI();
    }

    //if status beam hits player
    if (distance <= 15) {
        gfx_ScaledTransparentSprite_NoClip(explosion,enemy.x,enemy.y,2,2);
        gfx_ScaledTransparentSprite_NoClip(explosion,player.x,player.y,2,2);
       
        if (player.ships > 0) {
                player.ships--;
        } else {
            player.endgame = true;
        }
        player.x = randInt(0,290);
        player.y = randInt(0,210);
        enemy.y = randInt(0,180);
    }

    //fire player bullet
    if (kb_Data[6] & kb_Enter) {
        player.bulletX = player.x;
        player.bulletY = player.y - 5;
        player.fired = true;
    }

    //check if player or alien is hit - run following events
    if (player.fired == true || enemy.fired == true) {
        if (bdistance <= 30) {
            player.fired = false;
            gfx_ScaledTransparentSprite_NoClip(explosion,enemy.x,enemy.y,2,2);
            player.score += 80;
            enemy.y = randInt(0,180);
        } else if (sdistance <= 30) {
            enemy.fired = false;

            gfx_ScaledTransparentSprite_NoClip(explosion,player.x,player.y,2,2);
            if (player.ships > 0) {
                player.ships--;
            } else {
                player.endgame = true;
            }
            

            //respawn player in random position
            player.x = randInt(0,290);
            player.y = randInt(0,210);
        }
    }
}

//shoot and move status beam
void doAI() {
    if (enemy.y < player.y && enemy.fired == false) {
        enemy.fired = true;
        enemy.bulletX = enemy.x + 15;
        enemy.bulletY = enemy.y + 24;
    }
}

void runGame() {
    //5 ships, 80 points per ship destroyed, each ship destroyed creates a wave
    createPlayer();
    gfx_SetDrawBuffer();
    gfx_SetTextScale(2,2);
    WhiText();
    do {
        gfx_ZeroScreen();
        
        gfx_PrintStringXY("SCORE ",20,10);
        gfx_PrintInt(player.score,1);
        gfx_PrintStringXY("SHIPS  ",300-gfx_GetStringWidth("SHIPS  "),10);
        gfx_PrintInt(player.ships,1);
        
        doMovement();

        gfx_ScaledTransparentSprite_NoClip(player_ship,player.x, player.y,2,2);
        gfx_ScaledTransparentSprite_NoClip(enemy_ship,enemy.x,enemy.y,2,2);

        if (player.fired == true) {
            gfx_ScaledTransparentSprite_NoClip(bullets,player.bulletX,player.bulletY,2,2);
        } 
        if (enemy.fired == true) {
            gfx_ScaledTransparentSprite_NoClip(status_beam,enemy.bulletX,enemy.bulletY,2,2);
            WhiText();
            gfx_PrintStringXY("STATUS BEAM",310 - gfx_GetStringWidth("STATUS BEAM"),140);
        }

        doCollisions();

        gfx_BlitBuffer();
    } while (player.endgame == false);
    delay(100);

    endScreen();
}

void endScreen() {
    bool exit = false;
    bool keypressed = false;
    gfx_SetDrawScreen();
    gfx_ZeroScreen();

    //make sure exit doesnt trigger game
    do {
        kb_Scan();
        gfx_SetTextScale(3,3);
        gfx_PrintStringXY("BLASTAR",160 - gfx_GetStringWidth("BLASTAR") / 2,30);
        gfx_SetTextScale(2,2);
        gfx_PrintStringXY("FLEET DESTROYED",160 - gfx_GetStringWidth("FLEET DESTROYED") / 2,60);

        gfx_PrintStringXY("Y [1] / [0] N", 160 - gfx_GetStringWidth("Y [1] / [0] N")/2,200);
        
        if (kb_Data[6] & kb_Clear || kb_Data[3] & kb_0) {
            keypressed = true;
            exit = true;
        }
        if (kb_Data[3] & kb_1) {
            keypressed = true;
        }
        gfx_SetTextScale(1,1);
        gfx_PrintStringXY("WOULD YOU LIKE ANOTHER GAME",160 - gfx_GetStringWidth("WOULD YOU LIKE ANOTHER GAME") / 2,85);
        
    } while (keypressed == false);

    if (exit == false) {
        runGame();
    }
    
}

void main(void) {
    srand(rtc_Time());

    //begin graphx and set sprite transparency
    gfx_Begin();
    gfx_ZeroScreen();
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetTransparentColor(1);

    mainMenu();
    runGame();
    
    gfx_End();
}
