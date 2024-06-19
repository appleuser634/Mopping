# Makefile
all : flash

# template.c を書き換えた名前を入れる
TARGET:=main

# ch32v003fun のパスを指定する
CH32V003FUN=./ch32v003fun/ch32v003fun
MINICHLINK=./ch32v003fun/minichlink

include ./ch32v003fun/ch32v003fun/ch32v003fun.mk
flash : cv_flash
clean : cv_clean
