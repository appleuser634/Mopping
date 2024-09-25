#define SSD1306_128X64

#include "ch32v003fun.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ssd1306_i2c.h"
#include "ssd1306.h"
#include "ch32v003_GPIO_branchless.h"

#include "bomb.h"

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
    for (int i = -32; i < 64; i++) {
      ssd1306_setbuf(0);
      ssd1306_drawImage(20, 0, game_over_img, 32, 64, 0);
      ssd1306_drawImage(55, i, go_car_img, 32, 50, 0);
      ssd1306_refresh();
    }
    Delay_Ms(100);
	  Delay_Ms(2000);
}

void draw_road()
{
    int road_line[] = {105,10};
    int dot_line[] = {85,60,35};

    for (int i = 0; i < 2; i++) {
      ssd1306_drawLine(road_line[i], 0, road_line[i], SSD1306_H, 1);
    }

    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < SSD1306_H; j+=10) {
        ssd1306_drawLine(dot_line[i], j, dot_line[i], j+7, 1);
      }
    }
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
  int max_jump = 25;
  int jump_progress = 0;

  int mongoose_x = 130;
  int mongoose_y = 45;

  int earthworm_x = 130;
  int earthworm_y = 49;

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

    if(flip_flag){
      // draw mongoose
      ssd1306_drawImage(mongoose_x, mongoose_y, mongoose_1, 32, 16, 0);
      //draw earthworm
      // ssd1306_drawImage(earthworm_x, earthworm_y, earthworm_1, 16, 16, 0);
      // draw kuina
      ssd1306_drawImage(kuina_x, kuina_y, kuina_1, 16, 16, 0);
    } else {
      // draw mongoose
      //ssd1306_drawImage(mongoose_x, mongoose_y, mongoose_2, 30, 12, 0);
      ssd1306_drawImage(mongoose_x, mongoose_y, mongoose_2, 32, 16, 0);
      //draw earthworm
      // ssd1306_drawImage(earthworm_x, earthworm_y, earthworm_2, 16, 16, 0);
      // draw kuina
      ssd1306_drawImage(kuina_x, kuina_y, kuina_2, 16, 16, 0);
    }
    
    mongoose_x -= 1;
    if (mongoose_x < -32){
      mongoose_x = 130;
    } 
    
    // earthworm_x -= 1;
    // if (earthworm_x < -32){
    //   earthworm_x = 130;
    // } 

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
