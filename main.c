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
    bool button_state = false;

    int c = 0;
    while (1) {
      ssd1306_setbuf(0);

      if (c > 50) {
        ssd1306_drawImage(100, 0, start, 24, 64, 0);
      } else {
        ssd1306_drawImage(100, 0, start, 24, 64, 1);
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
    ssd1306_drawImage(40, 0, level_1, 24, 64, 0);
    ssd1306_refresh();
	  Delay_Ms(1000);
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
	  Delay_Ms(1000);
}

void game_over()
{
    ssd1306_setbuf(0);
    ssd1306_drawstr_sz(0,16, "GAME OVER", 1, fontsize_8x8);
    ssd1306_refresh();
    for (int i = 0; i < 500; i++) {
      GPIO_digitalWrite(BZR_PIN, high);
      Delay_Ms(1);
      GPIO_digitalWrite(BZR_PIN, low);
      Delay_Ms(1);
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
  {4,50,50,2,false},
  {4,25,20,2,true},
  {4,0,10,4,false}
};

car_state_data car_s_3[] = {
  {5,75,10,4,true},
  {5,75,30,4,true},
  {5,50,50,4,false},
  {5,25,20,4,true},
  {5,0,10,4,false}
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
          (car_s[i].car_y + 3) <= newt_y && (car_s[i].car_y + 21) >= newt_y 
          ||
          (car_s[i].car_y + 3) <= (newt_y + 21) && (car_s[i].car_y + 21) >= (newt_y + 21)
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
      }

      if (result) {
        stage ++;
      }

      if (stage > 3) {
        stage = 1;
      }
    }
  }
}
