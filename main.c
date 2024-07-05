#define SSD1306_128X64

#include "ch32v003fun.h"
#include <stdio.h>
#include <stdbool.h>
#include "ssd1306_i2c.h"
#include "ssd1306.h"
#include "ch32v003_GPIO_branchless.h"

#include "bomb.h"

#define BZR_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 6)
#define BTN_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 4)
#define ROT_A_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 5)
#define ROT_B_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 2)

void init_gpio()
{
    GPIO_port_enable(GPIO_port_A);
    GPIO_port_enable(GPIO_port_D);
    GPIO_port_enable(GPIO_port_C);

    GPIO_pinMode(BZR_PIN, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
    GPIO_pinMode(BTN_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
    GPIO_pinMode(ROT_A_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
    GPIO_pinMode(ROT_B_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
}

void opening()
{

    GPIO_digitalWrite(BZR_PIN, high);
    Delay_Ms(100);
    GPIO_digitalWrite(BZR_PIN, low);
    Delay_Ms(50);
    GPIO_digitalWrite(BZR_PIN, high);
    Delay_Ms(100);
    GPIO_digitalWrite(BZR_PIN, low);

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
      
      // buzzer on!
      if (button_is_pressed)
      {
          GPIO_digitalWrite(BZR_PIN, high);
      } else {
          GPIO_digitalWrite(BZR_PIN, low);
      }

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
    GPIO_digitalWrite(BZR_PIN, high);
    Delay_Ms(300);
    GPIO_digitalWrite(BZR_PIN, low);
    Delay_Ms(300);
    GPIO_digitalWrite(BZR_PIN, high);
    Delay_Ms(800);
    GPIO_digitalWrite(BZR_PIN, low);
	  Delay_Ms(2000);
}

void game_over()
{
    for (int i = -32; i < 64; i++) {
      ssd1306_setbuf(0);
      ssd1306_drawImage(20, 0, game_over_img, 32, 64, 0);
      ssd1306_drawImage(55, i, go_car_img, 32, 50, 0);
      ssd1306_refresh();
    }
    for (int i = 0; i < 50; i++) {
      GPIO_digitalWrite(BZR_PIN, high);
      Delay_Ms(1);
      GPIO_digitalWrite(BZR_PIN, low);
      Delay_Ms(6);
    }
    Delay_Ms(100);
    for (int i = 0; i < 50; i++) {
      GPIO_digitalWrite(BZR_PIN, high);
      Delay_Ms(1);
      GPIO_digitalWrite(BZR_PIN, low);
      Delay_Ms(6);
    }
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

struct car_state {
  int car_n;
  int car_x;
  int car_y;
  int car_step;
  bool dest;
};

typedef struct car_state car_state_data;

car_state_data car_s_1[] = {
  {4,75,10,2,true},
  {4,50,70,2,true},
  {4,25,20,4,false},
  {4,0,10,2,false}
};

car_state_data car_s_2[] = {
  {4,75,10,4,true},
  {4,50,20,4,true},
  {4,25,30,4,true},
  {4,0,40,4,true},
};

car_state_data car_s_3[] = {
  {6,75,10,4,true},
  {6,75,30,4,true},
  {6,50,50,8,false},
  {6,25,20,4,true},
  {6,0,30,5,false},
  {6,0,50,5,false}
};

car_state_data car_s_4[] = {
  {7,75,10,4,true},
  {7,75,30,4,true},
  {7,50,50,6,false},
  {7,25,20,6,true},
  {7,25,40,6,true},
  {7,0,30,5,false},
  {7,0,50,5,false}
};

car_state_data car_s_5[] = {
  {6,75,10,4,false},
  {6,75,30,4,true},
  {6,50,50,8,false},
  {6,25,20,4,true},
  {6,0,30,5,false},
  {6,0,50,5,true}
};



bool game_loop(car_state_data car_s[])
{
  int newt_x = 100;
  int newt_y = 20;
  int newt_step = 25;
  
  int flip_c = 0;
  bool flip_flag = false;

  bool button_state = false;

  uint8_t rot_a_pre_state = GPIO_digitalRead(ROT_A_PIN);
  uint8_t rot_b_pre_state = GPIO_digitalRead(ROT_B_PIN);

  while (1) {

    uint8_t button_is_pressed = !GPIO_digitalRead(BTN_PIN);

    // buzzer on!
    if (button_is_pressed)
    {
        GPIO_digitalWrite(BZR_PIN, high);
    } else {
        GPIO_digitalWrite(BZR_PIN, low);
    }

    // increment step
    if (button_state && !button_is_pressed){
      newt_x -= newt_step;
    }
    button_state = button_is_pressed;

    // get rotaly-encoder value
    uint8_t rot_a_state = GPIO_digitalRead(ROT_A_PIN);
    uint8_t rot_b_state = GPIO_digitalRead(ROT_B_PIN);

    if (rot_a_state && rot_b_state) {
      if (!rot_a_pre_state && rot_b_pre_state) {
        newt_y += 4;
      } else if (rot_a_pre_state && !rot_b_pre_state) {
        newt_y -= 4;
      }
    } 
    
    if (!rot_a_state && !rot_b_state) {
      if (!rot_a_pre_state && rot_b_pre_state) {
        newt_y -= 4;
      } else if (rot_a_pre_state && !rot_b_pre_state) {
        newt_y += 4;
      }
    } 

    rot_a_pre_state = rot_a_state;
    rot_b_pre_state = rot_b_state;

    if (newt_y < 0) {
      newt_y = 0;
    } 
    else if (newt_y > 40) {
      newt_y = 40;
    }

    // clear buffer for next mode
    ssd1306_setbuf(0);

    flip_c ++;

    if (flip_c > 100) {
      flip_flag = !flip_flag;
      flip_c = 0;
    }

    if(flip_flag){
      ssd1306_drawImage(newt_x, newt_y, newt_left, 24, 24, 0);
    } else {
      ssd1306_drawImage(newt_x, newt_y, newt_right, 24, 24, 0);
    }
    
    int car_n = car_s[0].car_n;
    for (int i = 0; i < car_n; i++){

      // 車を描画
      ssd1306_drawImage(car_s[i].car_x, car_s[i].car_y, car, 24, 24, 0);

      // 車を進める
      if (flip_c % 25 == 0) {
        if (car_s[i].dest) {
          car_s[i].car_y += car_s[i].car_step;
        } else {
          car_s[i].car_y -= car_s[i].car_step;
        }
      }

      // 車のループ
      if (car_s[i].car_y > 61) {
        car_s[i].car_y = -21;
      }
      if (car_s[i].car_y < -21) {
        car_s[i].car_y = 61;
      }

      // 当たり判定
      if (car_s[i].car_x == newt_x) {
        if (
          (car_s[i].car_y + 7) <= newt_y && (car_s[i].car_y + 17) >= newt_y 
          ||
          (car_s[i].car_y + 7) <= (newt_y + 17) && (car_s[i].car_y + 17) >= (newt_y + 17)
        ) {
          game_over();
          return false;
        }
      }
    }

    // 道の端に着いたらクリア
    if (newt_x < 0) {
      show_clear();
      return true;
    }
    
    draw_road();
    ssd1306_refresh();
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

    int stage = 1;
    bool result = false;
    while (1) {
 
      show_level(stage);
      switch (stage) {
        case 1:
          result = game_loop(car_s_1);
          break;
        case 2:
          result = game_loop(car_s_2);
          break;
        case 3:
          result = game_loop(car_s_3);
          break;
        case 4:
          result = game_loop(car_s_4);
          break;
        case 5:
          result = game_loop(car_s_5);
          break;
      }

      if (result) {
        stage ++;
      }

      if (stage > 5) {
        stage = 1;
      }
    }
  }
}
