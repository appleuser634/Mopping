#define SSD1306_128X64

#include "ch32v003fun.h"
#include <stdio.h>
#include <stdbool.h>
#include "ssd1306_i2c.h"
#include "ssd1306.h"
#include "ch32v003_GPIO_branchless.h"

#include "images.h"

#define NOISE_BITS 8
#define NOISE_MASK ((1<<NOISE_BITS)-1)
#define NOISE_POLY_TAP0 31
#define NOISE_POLY_TAP1 21
#define NOISE_POLY_TAP2 1
#define NOISE_POLY_TAP3 0
uint32_t lfsr = 1;

/*
 * random byte generator
 */
int get_random(void)
{
	uint8_t bit;
	uint32_t new_data;
	
	for(bit=0;bit<NOISE_BITS;bit++)
	{
		new_data = ((lfsr>>NOISE_POLY_TAP0) ^
					(lfsr>>NOISE_POLY_TAP1) ^
					(lfsr>>NOISE_POLY_TAP2) ^
					(lfsr>>NOISE_POLY_TAP3));
		lfsr = (lfsr<<1) | (new_data&1);
	}

  int random_8 = lfsr&NOISE_MASK;
	return random_8 % 11;
}

void string_concat(char *dest, const char *src) {
    // destの末尾を見つける
    while (*dest != '\0') {
        dest++;
    }
    
    // srcをdestにコピーする
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    
    // 最後にヌル文字を追加
    *dest = '\0';
}

#define BTN_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 1)

void init_gpio()
{
    GPIO_port_enable(GPIO_port_A);
    GPIO_port_enable(GPIO_port_D);
    GPIO_port_enable(GPIO_port_C);

    GPIO_pinMode(BTN_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
}

void opening()
{


    bool button_state = false;
    int c = 0;

    for (int i = 0; i < 30; i+=2) {
      ssd1306_setbuf(0);
      ssd1306_drawImage(i, 0, title, 24, 64, 0);
      ssd1306_refresh();
      Delay_Ms(100);
    }
    
    while (1) {
      ssd1306_setbuf(0);

      ssd1306_drawImage(30, 0, title, 24, 64, 0);

      if (c > 50) {
        ssd1306_drawImage(80, 20, start, 24, 24, 0);
      } else {
        ssd1306_drawImage(80, 20, start, 24, 24, 1);
      }

      ssd1306_refresh();

      uint8_t button_is_pressed = !GPIO_digitalRead(BTN_PIN);
      
      // ボタンを押して離した後にゲームに移る
      if (button_state && !button_is_pressed){
        break;
      }
      button_state = button_is_pressed;
      
      c ++;      
      if (c > 100) {
        c = 0;
      } 
    }
}

void show_level(int stage)
{
    ssd1306_setbuf(0);
    
    switch (stage) {
        case 1:
          ssd1306_drawImage(40, 0, level_1, 24, 64, 0);
          break;
        case 2:
          ssd1306_drawImage(40, 0, level_2, 24, 64, 0);
          break;
        case 3:
          ssd1306_drawImage(40, 0, level_3, 24, 64, 0);
          break;
        case 4:
          ssd1306_drawImage(40, 0, level_4, 24, 64, 0);
          break;
        case 5:
          ssd1306_drawImage(40, 0, level_5, 24, 64, 0);
          break;
    }
    ssd1306_refresh();
	  Delay_Ms(1500);
}

void show_clear()
{
    ssd1306_setbuf(0);
    ssd1306_drawImage(40, 0, clear, 40, 64, 0);
    ssd1306_refresh();
}

void game_over()
{
    // for (int i = -32; i < 64; i++) {
    //   ssd1306_setbuf(0);
    //   ssd1306_drawImage(20, 0, game_over_img, 32, 64, 0);
    //   ssd1306_drawImage(55, i, go_car_img, 32, 50, 0);
    //   ssd1306_refresh();
    // }
    // Delay_Ms(100);
	  Delay_Ms(2000);
}


bool game_loop()
{
  int init_kuina_x = 15;
  int init_kuina_y = 45;

  int kuina_x = init_kuina_x;
  int kuina_y = init_kuina_y;
  
  int flip_c = 0;
  bool flip_flag = false;

  bool button_state = false;
  
  bool jump_flag = false;
  int max_jump = 30;
  int jump_progress = 0;

  int character_x = 130;
  int character_y = 53;
  int character_w = 8;
  int character_h = 8;

  bool danger_flag = false;

  // 0 is grass      50%
  // 1 is pineapple  30%
  // 2 is mongoose   10%
  // 3 is earthworm  10%
  int character_num = 0;

  int random_n = get_random();

  int score = 0;

  while (1) {

    uint8_t button_is_pressed = !GPIO_digitalRead(BTN_PIN); 

    // jump trigger
    if (!button_state && button_is_pressed && !jump_flag){
      jump_flag = true;
    }
    button_state = button_is_pressed;

    // jump animation
    if (jump_flag && (max_jump * 2) < jump_progress){
      jump_progress = 0;
      jump_flag = false;
      kuina_y = init_kuina_y;
    }
    else if (jump_flag && max_jump >= jump_progress){
      kuina_y -= 1;
      jump_progress += 1;
    }
    else if (jump_flag && max_jump < jump_progress){
      kuina_y += 1;
      jump_progress += 1;
    }

    flip_c ++;
    if (flip_c > 20) {
      flip_flag = !flip_flag;
      flip_c = 0;
    }

    // get random number
    if (!danger_flag){
      random_n = get_random();
      danger_flag = true;
    }

    // decision character
    if (danger_flag){
      if (random_n <= 5){
        character_num = 0;
        character_y = 53;
        character_w = 8;
        character_h = 8;
      }
      else if (random_n <= 8){
        character_num = 1;
        character_y = 45;
        character_w = 16;
        character_h = 16;
      }
      else if (random_n <= 9){
        character_num = 2;
        character_y = 53;
        character_w = 16;
        character_h = 8;
      }
      else if (random_n <= 10){
        character_num = 3;
        character_y = 53;
        character_w = 8;
        character_h = 8;
      }
    }


    // show character
    if (flip_flag){
      if (character_num == 0){
        // draw grass
        ssd1306_drawImage(character_x, character_y, grass_1, 8, 8, 0);
      }
      else if (character_num == 1){
        // draw pineapple
        ssd1306_drawImage(character_x, character_y, pineapple_1, 16, 16, 0);
      }
      else if (character_num == 2){
        // draw mongoose
        ssd1306_drawImage(character_x, character_y, mongoose_1, 16, 8, 0);
      }
      else if (character_num == 3){
        // draw earthworm
        ssd1306_drawImage(character_x, character_y, earthworm_1, 8, 8, 0);
      }
      // draw kuina
      ssd1306_drawImage(kuina_x, kuina_y, kuina_1, 16, 16, 0);
    } else {
      if (character_num == 0){
        // draw grass
        ssd1306_drawImage(character_x, character_y, grass_1, 8, 8, 0);
      }
      else if (character_num == 1){
        // draw pineapple
        ssd1306_drawImage(character_x, character_y, pineapple_1, 16, 16, 0);
      }
      else if (character_num == 2){
        // draw mongoose
        ssd1306_drawImage(character_x, character_y, mongoose_2, 16, 8, 0);
      }
      else if (character_num == 3){
        // draw earthworm
        ssd1306_drawImage(character_x, character_y, earthworm_2, 8, 8, 0);
      }
      // draw kuina
      ssd1306_drawImage(kuina_x, kuina_y, kuina_2, 16, 16, 0);
    }
    
    character_x -= 1;
    if (character_x < 0){
      character_x = 130;
      danger_flag = false;
      if (character_num == 1){
        score += 10;
      }
      else if (character_num == 2){
        score += 50;
      }
    }
    score += 1;
    
    // earthworm_x -= 1;
    // if (earthworm_x < -32){
    //   earthworm_x = 130;
    // } 
    
    // hit judge
    if (kuina_x < character_x && (kuina_x + character_w) > character_x && kuina_y > (init_kuina_y - character_h)) {
      game_over();
      break;
    }

    // draw jump score
    char score_txt[30] = "Score:"; 
    string_concat(score_txt, "10");
		ssd1306_drawstr_sz(0,0, score_txt, 1, fontsize_8x8);

    // draw_road();
    ssd1306_refresh();
    ssd1306_setbuf(0);
  }
}

int main()
{
	// 48MHz internal clock
	SystemInit();

  init_gpio();

	Delay_Ms( 200 );

	if (!ssd1306_i2c_init()) {
		ssd1306_init();

    opening();
    
    // clear display
    ssd1306_setbuf(0);
    ssd1306_refresh();

    bool stage_clear = false;
    while (1) { 
      stage_clear = game_loop();
    }
  }
}
