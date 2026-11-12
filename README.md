# Snake Game ATmega128 — Prompt 7

Firmware Embedded C cho CodeVisionAVR trên ATmega128. Mã được sửa trên Ubuntu; file HEX phải được Build All trong CodeVisionAVR trên Windows rồi mới thử Proteus và board. Báo cáo [giai đoạn 6](docs/final_validation_report.md) là mốc kiểm định của bản base, không xác nhận Prompt 7.

## Source và kiến trúc

| Module | Chức năng |
|---|---|
| `main.c` | Chuyển SELECT LEVEL → SELECT MAP → GAME, lập lịch bằng Timer1 tick 1 ms |
| `game/game_menu.c/.h` | Menu level và map, chu kỳ EASY 500 / NORMAL 300 / HARD 150 ms |
| `game/game_map.c/.h` | Bitmask vật cản trong Flash, truy vấn ô, số ô trống, kiểm tra spawn và liên thông |
| `game/snake_game.c/.h` | Di chuyển, va chạm, thức ăn, điểm và WIN theo sức chứa map |
| `game/game_render.c/.h` | Vẽ vật cản viền 6×6, thức ăn 3×3, rắn đặc 5×5 |
| `storage/highscore_eeprom.c/.h` | Chín kỷ lục Level × Map, đọc v1 và ghi v2 hai slot |
| `drivers/lcd1602.c/.h` | `alcd.h`, hiện menu map/kỷ lục và Level + Map khi chơi, ghi ký tự thay đổi |
| `drivers/display.c/.h`, `keypad.c/.h`, `timer.c/.h`, `led7seg.c/.h` | Ngoại vi base; GPIO và Timer1 giữ nguyên |

Lưới 14×8, mỗi ô 6×6 pixel. Map cố định:

| Map | Vật cản | Ô chơi được | Điểm tối đa |
|---|---:|---:|---:|
| CLASSIC | 0 | 112 | 1090 |
| PILLARS | 8 | 104 | 1010 |
| LABYRINTH | 12 | 100 | 970 |

Layout từ y=0 đến y=7, `#` là vật cản:

```text
CLASSIC          PILLARS          LABYRINTH
..............   ..............   ..............
..............   ..............   ...#......#...
..............   ....##..##....   ...#..##..#...
..............   ..............   ..............
..............   ..............   ..............
..............   ....##..##....   ..##..##..##..
..............   ..............   ..............
..............   ..............   ..............
```

Ba map được lưu bằng 8 mặt nạ 16 bit/map trong Flash. `game_map_validate()` kiểm tra số ô, vị trí rắn ban đầu (7,4), (6,4), (5,4), bit ngoài biên và liên thông bốn hướng; `game_map_select()` từ chối map không hợp lệ. Thức ăn được chọn bằng hai lượt duyệt ô trống, loại cả tường lẫn thân. Không có vòng lặp thử ngẫu nhiên vô hạn. Đầu va chạm tường thì Game Over trước khi mảng rắn thay đổi. Ô đuôi vừa rời vẫn được xử lý như bản base.

## Điều khiển và hiển thị

| Màn hình | Phím |
|---|---|
| Chọn Level | 2/8 chuyển EASY/NORMAL/HARD, 5 xác nhận |
| Chọn Map | 2/8 chuyển CLASSIC/PILLARS/LABYRINTH, 5 bắt đầu, 1 quay lại Level |
| Playing | 2/4/6/8 đổi hướng, 5 Pause, 9 Restart |
| Paused | 5 Resume, 9 Restart |
| Game Over/Win | 9 Restart cùng Level và Map; 1 về chọn Level |

Nokia hiện ba lựa chọn map ở y=17/26/35, mỗi dòng dài nhất 13 ký tự ×6=78 pixel, trong giới hạn 84 pixel. LCD 16×2 hiện `MAP: PILLARS` và `BEST: 0120` ở menu map, rồi `NORMAL PILLARS` / `STATUS: PLAYING` khi chơi. LCD chỉ ghi ký tự đổi, không gọi `lcd_clear()` trong vòng lặp. LED 7 đoạn CA vẫn hiện score hiện tại, `0000` khi vào menu hoặc Restart. Timer1 và pin mapping không đổi.

## EEPROM và nâng cấp từ bản base

V1 giữ nguyên hai khai báo `eeprom unsigned char hs_slot_a[13]`, `hs_slot_b[13]` theo đúng thứ tự cũ. Record v1 gồm magic `53 47`, version `01`, sequence 16 bit LE, ba score LE, CRC-8 và commit `A5`. V2 dùng hai record **25 byte** tại `0x0100–0x0118` và `0x0120–0x0138`, khai báo bằng cú pháp địa chỉ EEPROM `@` của CodeVisionAVR. Mỗi record có magic, version `02`, sequence 16 bit LE, chín score 16 bit LE theo `level*3+map`, CRC-8 ở byte 23 và commit `A5` ở byte 24. V2 dùng hai slot luân phiên; commit được ghi cuối và bản ghi được đọc lại để xác minh.

Khi khởi động: ưu tiên v2 hợp lệ mới nhất; nếu không có, đọc v1 hợp lệ mới nhất và ánh xạ EASY/NORMAL/HARD sang CLASSIC; hai map còn lại bắt đầu 0. Khởi động không ghi EEPROM. Chỉ khi kết thúc ván và score vượt kỷ lục đúng Level × Map mới ghi v2. V1 không bị ghi trong firmware Prompt 7.

**Trước khi nạp vào thiết bị có kỷ lục:** mở báo cáo memory map EEPROM của *đúng project CodeVisionAVR đang sử dụng* và xác nhận hai slot v1 nằm ngoài `0x0100–0x0138`; source v1 cũ không gán địa chỉ tuyệt đối nên không thể chứng minh địa chỉ chỉ từ repository. Sao lưu EEPROM. Kiểm tra `.eep`, lựa chọn program EEPROM, chip erase và fuse EESAVE trước khi nạp; nạp `.eep` mới hoặc chip erase có thể xóa kỷ lục. Cú pháp `eeprom ... @0x0100` được mô tả trong [CodeVisionAVR User Manual](https://www.thierry-lequeu.fr/data/CodeVisionAVR-3-20-User-Manual.pdf); vẫn cần Build All bằng phiên bản CVAVR đang dùng.

## Build và kiểm tra

1. Giữ project ATmega128 gốc cùng clock, cấu hình `glcd.h` PCD8544 và `alcd.h` LCD 4 bit. Thêm **một source mới** `game/game_map.c` vào project đã có đủ 10 source của Prompt 6; tổng 11 file `.c`. Header dùng đường dẫn tương đối, không thêm vào danh sách source. Không đưa `tests/mock` vào include path CVAVR.
2. Đặt các macro `KEYPAD_TEST_MODE`, `LED7SEG_TEST_MODE`, `LCD1602_TEST_MODE`, `LED7_LCD_TEST_MODE` bằng 0. Chạy **Project → Build All**. Kiểm tra lỗi/warning, memory map EEPROM, Flash/SRAM usage và đường dẫn Intel HEX. Chưa có Build All của Prompt 7 trong workspace này.
3. Mở sơ đồ Proteus gốc, gán HEX mới cho ATmega128, kiểm tra Clock Frequency khớp project. Chọn đủ 3 Level × 3 Map. Xác nhận tường đúng layout, mồi không nằm trên tường/thân, chạm tường Game Over, Restart giữ tổ hợp, LCD/LED đúng. Kiểm tra các chu kỳ 500/300/150 ms và hồi quy CLASSIC.
4. Để kiểm tra EEPROM: sao lưu image hiện tại; chạy từ image v1 có ba kỷ lục, xác nhận chúng xuất hiện ở CLASSIC, PILLARS/LABYRINTH bằng 0. Tạo kỷ lục mới ở từng cặp, reset MCU, kiểm tra kỷ lục giữ nguyên. Thử CRC sai, commit chưa hoàn thành và score thấp hơn. Phân biệt reset MCU với dừng/nạp lại mô phỏng vì Proteus có thể khởi tạo lại EEPROM từ image.

Trên Ubuntu, `sh tests/run_host_validation.sh` chạy mock C89 cho logic map/rắn, EEPROM và LCD. Test host đã đạt trong phiên làm việc này; nó **không** tạo HEX và không thay thế CVAVR, Proteus hay board thực.

## Pin mapping giữ nguyên

Nokia DC/DIN/CLK/RST/CS = PG0/PG1/PG2/PG3/PG4, BL=PD7. Keypad ROW1/2/3=PC6/PE1/PD0, COL1/2/3=PC4/PC5/PE0. LCD RS/RW/E/D4–D7=PA0/PA1/PA2/PA3–PA6. LED CA digit 1–4=PF0/PF3/PF4/PF6; segment A/B/C/D/E/F/G/DP=PF1/PF5/PA7/PD1/PD2/PF2/PD3/PF7. Clock 16 MHz là cấu hình dự kiến; đối chiếu project, Proteus và board trước khi nạp.
