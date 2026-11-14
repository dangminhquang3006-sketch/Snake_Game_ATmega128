# Snake Game trên ATmega128

Project firmware trò chơi rắn săn mồi viết bằng Embedded C cho **ATmega128** và **CodeVisionAVR**. Mã nguồn được quản lý trên Ubuntu; firmware được biên dịch trên Windows bằng CodeVisionAVR, kiểm tra với Proteus rồi mới nạp lên board thực tế.

## Phần cứng

| Thành phần | Vai trò |
|---|---|
| Nokia 5110 (PCD8544, 84×48) | Hiển thị menu và vùng chơi 14×8 ô |
| Keypad 3×3 | Chọn menu và điều khiển rắn |
| LED 7 đoạn anode chung, 4 chữ số | Hiển thị điểm hiện tại |
| LCD 16×2 | Hiển thị cấp độ, trạng thái, thời gian và kỷ lục |
| Timer1 | Tạo system tick 1 ms |
| EEPROM nội | Lưu điểm cao nhất |

Sơ đồ chân nằm trong [board_config.h](config/board_config.h) và tài liệu ở [docs/](docs/). Nokia dùng PG0–PG4 và PD7; keypad dùng PC6, PE1, PD0, PC4, PC5, PE0. Giữ nguyên pin mapping của board và schematic khi build hoặc mô phỏng.

## Luật chơi

Người chơi chọn **Level → Map → Mode** trước khi bắt đầu. Rắn tự di chuyển theo chu kỳ của Level; phím điều hướng chỉ đổi hướng, không cho quay ngược trực tiếp vào thân.

| Level | Chu kỳ | Luật riêng |
|---|---:|---|
| EASY | 500 ms/ô | Chạm biên là Game Over |
| NORMAL | 300 ms/ô | Xuyên qua hai cạnh đối diện |
| HARD | 150 ms/ô | Xuyên biên; đảo đầu và đuôi sau khi ăn thức ăn thường |

Có ba Map: **CLASSIC** không có tường cố định, **PILLARS** có các cụm cột, và **LABYRINTH** có hành lang. Có bốn Mode:

| Mode | Luật riêng |
|---|---|
| STANDARD | Chơi thông thường |
| BONUS | Có Bonus Food sau mỗi ba lần ăn thức ăn thường; Bonus tồn tại 5 giây RUNNING |
| TIME_ATTACK | Như BONUS, thêm giới hạn 60 giây RUNNING |
| SURVIVAL | Thử sinh thêm một vật cản mỗi 20 giây RUNNING, tối đa 8; không có Bonus |

Thức ăn thường được chọn bằng PRNG hiện có. Mọi loại đều tăng chiều dài rắn một đốt:

| Loại | Xác suất | Điểm |
|---|---:|---:|
| CIRCLE | 60% | +10 |
| SQUARE | 35% | +20 |
| STAR | 5% | +50 |

**Bonus Food** cộng 30 điểm, không tăng chiều dài và không kích hoạt đảo rắn. Rắn thua khi đâm vào thân hoặc vật cản; ở EASY, đi ra ngoài biên cũng gây Game Over. Khi chiếm hết các ô chơi hợp lệ, game chuyển sang WIN. Pause giữ nguyên trạng thái và dừng thời gian gameplay; Restart bắt đầu lại với Level, Map và Mode đã chọn.

### Điều khiển

| Phím | Trong menu | Trong game |
|---|---|---|
| 2 / 8 | Chọn lên / xuống | Đi lên / xuống |
| 4 / 6 | — | Đi trái / phải |
| 5 | Xác nhận | Pause / Resume |
| 9 | — | Restart |
| 1 | Quay lại khi menu hỗ trợ | Về menu Level sau Game Over, WIN hoặc TIME_UP |

## Cấu trúc source

```text
.
├── main.c                     # Vòng lặp chính và điều phối trạng thái
├── config/board_config.h      # Cấu hình board và hằng số
├── drivers/                   # Nokia, keypad, Timer1, LED 7 đoạn, LCD 16×2
├── game/                      # Snake, render, menu, Map, Mode, Bonus, vật cản động
├── storage/highscore_eeprom.c # Điểm cao nhất trong EEPROM
├── tests/                     # Kiểm thử logic trên host; không đưa vào firmware
└── docs/                      # Sơ đồ, tài liệu kiến trúc và báo cáo kiểm thử
```

`main.c` gọi các module trong `drivers/`, `game/` và `storage/`. ISR Timer1 chỉ cập nhật tick; xử lý game, vẽ màn hình và ghi EEPROM diễn ra trong vòng lặp chính. EEPROM v4 lưu kỷ lục riêng cho **36 tổ hợp** Level × Map × Mode và giữ đường đọc dữ liệu v1–v3. Kỷ lục chỉ được ghi khi có điểm mới cao hơn.

## Build bằng CodeVisionAVR

1. Mở project ATmega128 hiện có trong CodeVisionAVR. Kiểm tra CPU clock khớp với Proteus hoặc board.
2. Đảm bảo project có đủ 14 file `.c`: `main.c`; `drivers/display.c`, `keypad.c`, `timer.c`, `led7seg.c`, `lcd1602.c`; `game/snake_game.c`, `game_render.c`, `game_menu.c`, `game_map.c`, `game_mode.c`, `bonus_food.c`, `dynamic_obstacle.c`; `storage/highscore_eeprom.c`.
3. Kiểm tra cấu hình `glcd.h` cho PCD8544 với font `font5x7`, `alcd.h` ở chế độ 4 bit và các include path của project. Bốn macro test ngoại vi trong `board_config.h` phải bằng `0` khi build game.
4. Chạy **Project → Build All**. Xem lỗi, cảnh báo, bộ nhớ sử dụng, EEPROM memory map và đường dẫn file HEX mới.

Không thêm `tests/` hoặc `tests/mock/` vào project CodeVisionAVR. Firmware không dùng AVR-GCC, Arduino, C++ hoặc Makefile để build.

## Mô phỏng và kiểm thử

Trong Proteus, mở schematic của board, gán HEX mới vào **Program File** của ATmega128 và xác nhận **Clock Frequency**. Kiểm tra menu, phím điều khiển, ba Level, ba Map, bốn Mode, điểm trên LED, trạng thái trên LCD và hình vẽ trên Nokia. Thử riêng Wrap Around, Reverse Snake, Bonus, TIME_UP, vật cản động, Pause/Restart và kỷ lục sau khi reset MCU.

Trên Ubuntu, chạy kiểm thử logic bằng:

```sh
sh tests/run_host_validation.sh
```

Test host dùng mock để kiểm tra thuật toán; nó không tạo HEX và không thay thế Build All hoặc Proteus. [Báo cáo kiểm định mở rộng](docs/final_extended_validation_report.md) ghi ma trận 36 tổ hợp, kết quả đã kiểm tra và các mục còn cần xác minh trên CVAVR, Proteus và board thực.

Trước khi nạp board, kiểm tra nguồn, clock, mức điện áp Nokia, kết nối ISP và fuse. Sao lưu EEPROM; kiểm tra file `.eep`, chip erase và EESAVE để không ghi đè kỷ lục cũ.
