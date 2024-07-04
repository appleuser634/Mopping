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
    ssd1306_setbuf(0);
    ssd1306_drawImage(0, 0, movoid_title, 128, 64, 0);
    ssd1306_refresh();
	  Delay_Ms(2000);
}

void clear()
{
    ssd1306_setbuf(0);
    ssd1306_drawstr_sz(0,16, "CLEAR!!", 1, fontsize_8x8);
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
  int car_x;
  int car_y;
  int car_step;
  bool dest;
};

typedef struct car_state car_state_data;

int main()
{
	// 48MHz internal clock
	SystemInit();

  init_gpio();

	Delay_Ms( 200 );

	if (!ssd1306_i2c_init()) {
		ssd1306_init();

    opening();

    while (1) {
    
      int newt_x = 100;
      int newt_y = 20;
      int newt_step = 25;
      
      int flip_c = 0;
      bool flip_flag = false;

      bool button_state = false;

      uint8_t rot_a_pre_state = GPIO_digitalRead(ROT_A_PIN);
      uint8_t rot_b_pre_state = GPIO_digitalRead(ROT_B_PIN);

      car_state_data car_s[] = {
        {75,10,2,true},
        {50,50,2,false},
        {25,20,4,true},
        {0,10,2,false}
      };

      bool run_flag = true;

      while (run_flag) {

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

        // switch(mode)
        // {
        // 	case 0:
        // 		// printf("buffer fill with binary\n\r");
        // 		for(int i=0;i<sizeof(ssd1306_buffer);i++)
        // 			ssd1306_buffer[i] = i;
        // 		break;
        // 	
        // 	case 1:
        // 		// printf("pixel plots\n\r");
        // 		for(int i=0;i<SSD1306_W;i++)
        // 		{
        // 			ssd1306_drawPixel(i, i/(SSD1306_W/SSD1306_H), 1);
        // 			ssd1306_drawPixel(i, SSD1306_H-1-(i/(SSD1306_W/SSD1306_H)), 1);
        // 		}
        // 		break;
        // 	
        // 	case 2:
        // 		{
        // 			// printf("Line plots\n\r");
        // 			uint8_t y= 0;
        // 			for(uint8_t x=0;x<SSD1306_W;x+=16)
        // 			{
        // 				ssd1306_drawLine(x, 0, SSD1306_W, y, 1);
        // 				ssd1306_drawLine(SSD1306_W-x, SSD1306_H, 0, SSD1306_H-y, 1);
        // 				y+= SSD1306_H/8;
        // 			}
        // 		}
        // 		break;
        // 		
        // 	case 3:
        // 		// printf("Circles empty and filled\n\r");
        // 		for(uint8_t x=0;x<SSD1306_W;x+=16)
        // 			if(x<64)
        // 				ssd1306_drawCircle(x, SSD1306_H/2, 15, 1);
        // 			else
        // 				ssd1306_fillCircle(x, SSD1306_H/2, 15, 1);
        // 		break;
        // 	case 4:
        // 		// printf("Image\n\r");
        // 		ssd1306_drawImage(0, 0, bomb_i_stripped, 32, 32, 0);
        // 		ssd1306_drawImage(32, 0, newt_left, 12, 12, 0);
        // 		ssd1306_drawImage(45, 0, newt_right, 12, 12, 0);
        // 		break;
        // 	case 5:
        // 		// printf("Unscaled Text\n\r");
        // 		ssd1306_drawstr(0,0, "This is a test", 1);
        // 		ssd1306_drawstr(0,8, "of the emergency", 1);
        // 		ssd1306_drawstr(0,16,"broadcasting", 1);
        // 		ssd1306_drawstr(0,24,"system.",1);
        // 		if(SSD1306_H>32)
        // 		{
        // 			ssd1306_drawstr(0,32, "Lorem ipsum", 1);
        // 			ssd1306_drawstr(0,40, "dolor sit amet,", 1);
        // 			ssd1306_drawstr(0,48,"consectetur", 1);
        // 			ssd1306_drawstr(0,56,"adipiscing",1);
        // 		}
        // 		ssd1306_xorrect(SSD1306_W/2, 0, SSD1306_W/2, SSD1306_W);
        // 		break;
        // 		
        // 	case 6:
        // 		// printf("Scaled Text 1, 2\n\r");
        // 		ssd1306_drawstr_sz(0,0, "sz 8x8", 1, fontsize_8x8);
        // 		ssd1306_drawstr_sz(0,16, "16x16", 1, fontsize_16x16);
        // 		break;
        // 	
        // 	case 7:
        // 		// printf("Scaled Text 4\n\r");
        // 		ssd1306_drawstr_sz(0,0, "32x32", 1, fontsize_32x32);
        // 		break;
        // 	
        // 	
        // 	case 8:
        // 		// printf("Scaled Text 8\n\r");
        // 		ssd1306_drawstr_sz(0,0, "64", 1, fontsize_64x64);
        // 		break;

          // 	default:
        // 		break;
        // }
        
        for (int i = 0; i < 4; i++){

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
              run_flag = false;
              break;
            }
          }
        }

        // 道の端に着いたらクリア
        if (newt_x < 0) {
          clear();
          run_flag = false;
        }
        
        draw_road();
        ssd1306_refresh();
      }
    }
  }
}
