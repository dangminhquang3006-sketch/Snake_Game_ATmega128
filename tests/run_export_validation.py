#!/usr/bin/env python3
"""Check the two-file export in isolation using host-only AVR adapters.

These checks exercise the merged C code, not the CodeVision compiler or timing.
The exported artifacts themselves are never modified by these adapters.
"""

import os
from pathlib import Path
import re
import shlex
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]


def main():
    with tempfile.TemporaryDirectory(prefix="snake-export-validation-") as temp:
        work = Path(temp)
        export = work / "export"
        command = [sys.executable, str(ROOT / "export/generate_codevision.py"),
                   "--output-dir", str(export)]
        subprocess.run(command, check=True, stdout=subprocess.DEVNULL)
        assert {p.name for p in export.iterdir()} == {
            "snake_firmware.c", "board_config.h"}
        original = {p.name: p.read_bytes() for p in export.iterdir()}
        subprocess.run(command, check=True, stdout=subprocess.DEVNULL)
        assert original == {p.name: p.read_bytes() for p in export.iterdir()}
        subprocess.run(command + ["--check"], check=True)
        source = (export / "snake_firmware.c").read_text()
        assert re.findall(r'^#include "([^"]+)"', source, re.M) == ["board_config.h"]
        assert not re.search(r'\b(?:KEY_PAUSE|SNAKE_PAUSED|highscore_\w*)\b', source)
        assert 'KEY_CONFIRM' in source
        subprocess.run([sys.executable, str(ROOT / "export/generate_codevision.py"),
                        "--check"], check=True)
        assert original["board_config.h"] == (ROOT / "config/board_config.h").read_bytes()
        # Check that --check detects source/config drift without overwriting it.
        (export / "board_config.h").write_bytes(original["board_config.h"] + b"\n")
        stale = subprocess.run(command + ["--check"], capture_output=True)
        assert stale.returncode == 1 and b"board_config.h" in stale.stderr

        # Adapt only unsupported AVR syntax for the host compiler. Keep all
        # firmware modules in the same translation unit to catch name clashes.
        source = re.sub(r'interrupt \[TIM1_COMPA\] void', 'void', source)
        source = re.sub(r'^\s*#asm\("(?:cli|sei)"\)\s*$', '', source, flags=re.M)
        source = source.replace('void main(void)', 'void firmware_main(void)')
        (work / "host_firmware.c").write_text(source)
        config = original["board_config.h"].decode()
        config = re.sub(r'\b(PORT|DDR|PIN)([A-G])\.([0-7])\b',
                        lambda m: 'host_bits[%d]' % (
                            ('PORT', 'DDR', 'PIN').index(m[1]) * 56
                            + (ord(m[2]) - ord('A')) * 8 + int(m[3])), config)
        (work / "board_config.h").write_text(config)
        registers = 'PORTA PORTB PORTD PORTF DDRA DDRB DDRD DDRF PINB SREG TCCR1A TCCR1B TIFR TIMSK'
        declarations = 'unsigned char ' + ','.join(registers.split()) + ';\n'
        declarations += 'unsigned int TCNT1,OCR1A;\nunsigned char host_bits[168];\n'
        (work / "mega128.h").write_text(
            '#ifndef HOST_MOCK_MEGA128_H\n#define HOST_MOCK_MEGA128_H\n'
            '#define flash\n'
            + ''.join('extern ' + line + '\n' for line in declarations.splitlines())
            + 'enum { OCF1A=4, OCIE1A=4, WGM12=3, CS11=1, CS10=0 };\n#endif\n')
        glcd = '''#include <mega128.h>
typedef struct {
    unsigned char *font;
    unsigned char readxmem,writexmem,temp_coef,bias,vlcd;
} GLCDINIT_t;
unsigned char glcd_init(GLCDINIT_t *data);
void glcd_clear(void);
void glcd_outtextxyf(unsigned char x,unsigned char y,char *text);
void glcd_outtextxy(unsigned char x,unsigned char y,char *text);
void glcd_rectangle(unsigned char x,unsigned char y,unsigned char right,unsigned char bottom);
void glcd_bar(unsigned char x,unsigned char y,unsigned char right,unsigned char bottom);
'''
        (work / "glcd.h").write_text(glcd)
        (work / "font5x7.h").write_text('extern unsigned char font5x7[];\n')
        (work / "alcd.h").write_bytes((ROOT / "tests/mock/alcd.h").read_bytes())
        support = '#include "glcd.h"\n'
        support += 'unsigned char font5x7[1];\n'
        support += 'unsigned char glcd_init(GLCDINIT_t *data) { (void)data; return 1; }\n'
        support += 'void glcd_clear(void) {}\n'
        for name in ('glcd_outtextxyf', 'glcd_outtextxy'):
            support += ('void %s(unsigned char x,unsigned char y,char *text) '
                        '{ (void)x; (void)y; (void)text; }\n') % name
        for name in ('glcd_rectangle', 'glcd_bar'):
            support += ('void %s(unsigned char x,unsigned char y,unsigned char right,'
                        'unsigned char bottom) '
                        '{ (void)x; (void)y; (void)right; (void)bottom; }\n') % name
        (work / "hardware.c").write_text(support)
        (work / "lcd_stub.c").write_text(
            '#include "alcd.h"\n'
            'void lcd_init(unsigned char columns) { (void)columns; }\n'
            'void lcd_gotoxy(unsigned char x,unsigned char y) { (void)x; (void)y; }\n'
            'void lcd_putchar(char value) { (void)value; }\n')
        cc = shlex.split(os.environ.get("CC", "cc"))
        flags = ['-std=c89', '-Wall', '-Wextra', '-Werror', '-DHOST_TEST',
                 '-D_MCU_CLOCK_FREQUENCY_=8000000UL', '-I' + str(work)]
        # Compile the full merged firmware in every peripheral-test variant.
        for mode in (None, 'KEYPAD_TEST_MODE', 'LED7SEG_TEST_MODE',
                     'LCD1602_TEST_MODE', 'LED7_LCD_TEST_MODE'):
            define = ['-D' + mode + '=1'] if mode else []
            subprocess.run(cc + flags + define + ['-fsyntax-only',
                           str(work / 'host_firmware.c')], check=True)
        print('PASS: merged firmware host syntax, game and all 4 peripheral test modes')
        for case in ('host_snake', 'host_dynamic', 'host_lcd', 'host_led7seg'):
            test = (ROOT / 'tests' / (case + '.c')).read_text()

            def local_include(match):
                name = match[1]
                if name in ('../game/game.c', '../drivers/led7seg.c'):
                    return '#include "host_firmware.c"'
                return '#include "' + str((ROOT / 'tests' / name).resolve()) + '"'

            test = re.sub(r'^#include "([^"]+)"', local_include, test, flags=re.M)
            harness = work / (case + '.c')
            harness.write_text(test)
            sources = [str(harness), str(work / 'hardware.c'), str(ROOT / 'tests/mock_avr.c')]
            if case in ('host_lcd', 'host_led7seg'):
                sources.append(str(work / 'host_firmware.c'))
            if case != 'host_lcd':
                sources.append(str(work / 'lcd_stub.c'))
            binary = work / case
            subprocess.run(cc + flags + sources + ['-o', str(binary)], check=True)
            subprocess.run([str(binary)], check=True)
        print('PASS: isolated two-file export, deterministic generation and stale detection')


if __name__ == '__main__':
    main()
