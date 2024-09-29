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
int get_random(int num)
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
	return random_8 % (num + 1);
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

typedef struct {
  int id;
  const unsigned char *img;
  const unsigned char *img_1;
  const unsigned char *img_2;
  int x;
  int y;
  int w;
  int h;
} character;

void gen_enemy(character *enemy)
{ 
  int character_n = get_random(10);
  if (character_n <= 5){
    enemy[2].id = 0;
    enemy[2].img = grass_1;
    enemy[2].img_1 = grass_1;
    enemy[2].img_2 = grass_1;
    enemy[2].y = 53;
    enemy[2].w = 8;
    enemy[2].h = 8;
  }
  else if (character_n <= 8){
    enemy[2].id = 1;
    enemy[2].img = pineapple_1;
    enemy[2].img_1 = pineapple_1;
    enemy[2].img_2 = pineapple_1;
    enemy[2].y = 45;
    enemy[2].w = 16;
    enemy[2].h = 16;
  }
  else if (character_n <= 9){
    enemy[2].id = 2;
    enemy[2].img = mongoose_1;
    enemy[2].img_1 = mongoose_1;
    enemy[2].img_2 = mongoose_2;
    enemy[2].y = 53;
    enemy[2].w = 16;
    enemy[2].h = 8;
  }
  else if (character_n <= 10){
    enemy[2].id = 3;
    enemy[2].img = earthworm_1;
    enemy[2].img_1 = earthworm_1;
    enemy[2].img_2 = earthworm_2;
    enemy[2].y = 53;
    enemy[2].w = 8;
    enemy[2].h = 8;
  }

  int enemy_distance = get_random(30);
  int distance_buffer = 50;
  enemy[2].x = enemy[1].x + distance_buffer + enemy_distance;
}

bool game_loop()
{
  int init_kuina_x = 15;
  int init_kuina_y = 45;
  character kuina = {100,kuina_1,kuina_1,kuina_2,init_kuina_x,init_kuina_y,16,16};
 
  // struct character enemy = {pineapple_1,pineapple_1,pineapple_1,130,53,8,8};
  character enemy[3] = {
    {0,grass_1,grass_1,grass_1,130,53,8,8},
    {0,grass_1,grass_1,grass_1,160,53,8,8},
    {1,pineapple_1,pineapple_1,pineapple_1,210,45,16,16}
  };


  int flip_c = 0;
  bool flip_flag = false;

  bool button_state = false;
  
  bool jump_flag = false;
  int max_jump = 30;
  int jump_progress = 0;

  bool danger_flag = false;

  // 0 is grass      50%
  // 1 is pineapple  30%
  // 2 is mongoose   10%
  // 3 is earthworm  10%
  int enemy_num = 0;

  int random_n = get_random(10);

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
      kuina.y = init_kuina_y;
    }
    else if (jump_flag && max_jump >= jump_progress){
      kuina.y -= 1;
      jump_progress += 1;
    }
    else if (jump_flag && max_jump < jump_progress){
      kuina.y += 1;
      jump_progress += 1;
    }

    flip_c ++;
    if (flip_c > 20) {
      flip_flag = !flip_flag;
      flip_c = 0;
    }

    if (enemy[0].x < 0) {
      enemy[0] = enemy[1];
      enemy[1] = enemy[2];
      gen_enemy(enemy);
    }

    // get random number
    // if (!danger_flag){
    //   random_n = get_random();
    //   danger_flag = true;
    // }

    // // decision character
    // if (danger_flag){
    //   if (random_n <= 5){
    //     enemy.img = grass_1;
    //     enemy.img_1 = grass_1;
    //     enemy.img_2 = grass_1;
    //     enemy.y = 53;
    //     enemy.w = 8;
    //     enemy.h = 8;
    //   }
    //   else if (random_n <= 8){
    //     enemy.img = pineapple_1;
    //     enemy.img_1 = pineapple_1;
    //     enemy.img_2 = pineapple_1;
    //     enemy.y = 45;
    //     enemy.w = 16;
    //     enemy.h = 16;
    //   }
    //   else if (random_n <= 9){
    //     enemy.img = mongoose_1;
    //     enemy.img_1 = mongoose_1;
    //     enemy.img_2 = mongoose_2;
    //     enemy.y = 53;
    //     enemy.w = 16;
    //     enemy.h = 8;
    //   }
    //   else if (random_n <= 10){
    //     enemy.img = earthworm_1;
    //     enemy.img_1 = earthworm_1;
    //     enemy.img_2 = earthworm_2;
    //     enemy.y = 53;
    //     enemy.w = 8;
    //     enemy.h = 8;
    //   }
    // }

    // flip character
    if (flip_flag){
      enemy[0].img = enemy[0].img_1;
      enemy[1].img = enemy[1].img_1;
      enemy[2].img = enemy[2].img_1;
      kuina.img = kuina.img_1;
    } else {
      enemy[0].img = enemy[0].img_2;
      enemy[1].img = enemy[1].img_2;
      enemy[2].img = enemy[2].img_2;
      kuina.img = kuina.img_2;
    }

    // draw kuina
    ssd1306_drawImage(kuina.x, kuina.y, kuina.img, kuina.w, kuina.h, 0);
    ssd1306_drawRect(kuina.x, kuina.y, kuina.w, kuina.h, 1);
    
    ssd1306_drawImage(-8, 0, kuina.img, kuina.w, kuina.h, 0);
    ssd1306_drawRect(0, 0, kuina.w, kuina.h, 1);
    
    score += 1;
    
    // earthworm_x -= 1;
    // if (earthworm_x < -32){
    //   earthworm_x = 130;
    // } 
    
    for (int i=0; i < 3; i++){

      // draw enemy
      ssd1306_drawImage(enemy[i].x, enemy[i].y, enemy[i].img, enemy[i].w, enemy[i].h, 0);
      ssd1306_drawRect(enemy[i].x, enemy[i].y, enemy[i].w, enemy[i].h, 1);

      enemy[i].x -= 1;

      // hit judge
      if (kuina.x < enemy[i].x && (kuina.x + kuina.w) > enemy[i].x && kuina.y > (init_kuina_y - enemy[i].h)) {
        if (enemy[i].id == 1 | enemy[i].id == 2){
          game_over();
          break;
        }
        else if (enemy[i].id == 3){
          // bonus point
          score += 50;
        }
      }
    }

    // draw jump score
    char score_txt[30] = "Score:"; 
    string_concat(score_txt, "10");
		//ssd1306_drawstr_sz(0,0, score_txt, 1, fontsize_8x8);

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
