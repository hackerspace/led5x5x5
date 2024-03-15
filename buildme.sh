set -e
set -x
avr-gcc -O2 -mmcu=atmega16 -o snake16.elf snake.c
avr-gcc -O2 -mmcu=atmega32 -o snake32.elf snake.c

avr-objcopy -j .text -j .data -O binary snake16.elf snake16.bin
truncate -s 16320 snake16.bin
echo -ne '\n$Repository: https://github.com/hackerspace/led5x5x5 $\n' >>snake16.bin
truncate -s 16383 snake16.bin
echo -ne '\x08' >>snake16.bin
hexdump -C snake16.bin

avr-objcopy -j .text -j .data -O binary snake32.elf snake32.bin
truncate -s 32704 snake32.bin
echo -ne '\n$Repository: https://github.com/hackerspace/led5x5x5 $\n' >>snake32.bin
truncate -s 32767 snake32.bin
echo -ne '\x08' >>snake32.bin
hexdump -C snake32.bin

enscript -MA4 -B -E --margins=12 -o snake.ps snake.c
ps2pdf snake.ps

#minipro -p 'ATMEGA32' -i -v -w snake32.bin
#minipro -p 'ATMEGA16' -I -v -w snake16.bin
#minipro -p 'ATMEGA32' -w snake32.bin
minipro -p 'ATMEGA16' -w snake16.bin
