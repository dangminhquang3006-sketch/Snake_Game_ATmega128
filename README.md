# Snake Game trên ATmega128

Project firmware trò chơi rắn săn mồi viết bằng Embedded C cho **ATmega128** và **CodeVisionAVR**. Mã nguồn được quản lý trên Ubuntu; firmware được biên dịch trên Windows bằng CodeVisionAVR, kiểm tra với Proteus rồi mới nạp lên board thực tế.

Hướng dẫn: [Các file cần thêm vào CodeVision và header đi kèm](#cần-thêm-những-file-nào-vào-codevision-để-chạy) · [Các bước build và chạy](#các-bước-build-và-chạy).

## Phần cứng

| Thành phần | Vai trò |
|---|---|
| Nokia 5110 (PCD8544, 84×48) | Hiển thị menu và vùng chơi 14×8 ô |
| Keypad 3×3 | Chọn menu và điều khiển rắn |
| LED 7 đoạn anode chung, 4 chữ số | Hiển thị điểm hiện tại |
| LCD 16×2 | Hiển thị cấp độ, trạng thái, vật cản và kỷ lục |
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

Có ba Map: **CLASSIC** không có tường cố định, **PILLARS** có các cụm cột, và **LABYRINTH** có hành lang. Có hai Mode:

| Mode | Luật riêng |
|---|---|
| STANDARD | Chơi thông thường |
| SURVIVAL | Thử sinh thêm một vật cản mỗi 10 giây RUNNING, tối đa 8 |

Thức ăn thường được chọn bằng PRNG hiện có. Mọi loại đều tăng chiều dài rắn một đốt:

| Loại | Xác suất | Điểm |
|---|---:|---:|
| CIRCLE | 60% | +10 |
| SQUARE | 35% | +20 |
| STAR | 5% | +50 |

Rắn thua khi đâm vào thân hoặc vật cản; ở EASY, đi ra ngoài biên cũng gây Game Over. Khi chiếm hết các ô chơi hợp lệ, game chuyển sang WIN. Pause giữ nguyên trạng thái và dừng thời gian gameplay; Restart bắt đầu lại với Level, Map và Mode đã chọn.

### Điều khiển

| Phím | Trong menu | Trong game |
|---|---|---|
| 2 / 8 | Chọn lên / xuống | Đi lên / xuống |
| 4 / 6 | — | Đi trái / phải |
| 5 | Xác nhận | Pause / Resume |
| 9 | — | Restart |
| 1 | Quay lại khi menu hỗ trợ | Về menu Level sau Game Over hoặc WIN |

## Cấu trúc source

```text
.
├── main.c                     # Vòng lặp chính và điều phối trạng thái
├── config/board_config.h      # Cấu hình board và hằng số
├── drivers/                   # Nokia, keypad, Timer1, LED 7 đoạn, LCD 16×2
├── game/game.c                # Toàn bộ gameplay: Snake, render, menu, Map, Mode, vật cản
├── game/*.h                   # Khai báo API gameplay; không thêm vào danh sách .c
├── storage/highscore_eeprom.c # Điểm cao nhất trong EEPROM
├── tests/                     # Kiểm thử logic trên host; không đưa vào firmware
└── docs/                      # Sơ đồ, tài liệu kiến trúc và báo cáo kiểm thử
```

`main.c` gọi các module trong `drivers/`, `game/` và `storage/`. ISR Timer1 chỉ cập nhật tick; xử lý game, vẽ màn hình và ghi EEPROM diễn ra trong vòng lặp chính. EEPROM v4 lưu kỷ lục riêng cho **18 tổ hợp** Level × Map × Mode và giữ đường đọc dữ liệu v1–v3. Định dạng 36 ô lưu cũ được giữ để đọc đúng kỷ lục STANDARD/SURVIVAL; 18 ô của hai mode đã bỏ không còn được truy cập qua API gameplay. Kỷ lục chỉ được ghi khi có điểm mới cao hơn.

## Cần thêm những file nào vào CodeVision để chạy?

**Thêm đúng 8 file `.c` dưới đây vào danh sách source của project CodeVisionAVR.** Các đường dẫn tính từ `D:\nhung\`:

| STT | File cần thêm vào project | Chức năng |
|---|---|---|
| 1 | `Snake_Game_ATmega128\main.c` | Vòng lặp chính của game |
| 2 | `Snake_Game_ATmega128\game\game.c` | Toàn bộ gameplay, menu, Map, Mode và render |
| 3 | `Snake_Game_ATmega128\drivers\display.c` | Nokia 5110 |
| 4 | `Snake_Game_ATmega128\drivers\keypad.c` | Keypad 3×3 |
| 5 | `Snake_Game_ATmega128\drivers\timer.c` | Tick Timer1 |
| 6 | `Snake_Game_ATmega128\drivers\led7seg.c` | LED 7 đoạn |
| 7 | `Snake_Game_ATmega128\drivers\lcd1602.c` | LCD 16×2 |
| 8 | `Snake_Game_ATmega128\storage\highscore_eeprom.c` | Lưu và đọc kỷ lục |

### Các file `.h` phải giữ kèm

Nếu chuyển project sang máy hoặc thư mục khác, cần copy cả các header bên dưới và giữ nguyên cấu trúc thư mục. Header được source gọi bằng `#include`; không cần thêm vào danh sách source `.c` để biên dịch.

```text
Snake_Game_ATmega128/
├── config/board_config.h
├── drivers/display.h
├── drivers/keypad.h
├── drivers/timer.h
├── drivers/led7seg.h
├── drivers/lcd1602.h
├── game/snake_game.h
├── game/game_render.h
├── game/game_menu.h
├── game/game_map.h
├── game/game_mode.h
├── game/dynamic_obstacle.h
└── storage/highscore_eeprom.h
```

Cách copy thuận tiện: mang theo `demo.prj`, `demo.cproj` nếu dùng tích hợp IDE, và thư mục `Snake_Game_ATmega128` chứa `main.c`, `config/`, `drivers/`, `game/`, `storage/`. Giữ project ở thư mục cha như cấu trúc hiện tại.

### Các file không thêm vào danh sách biên dịch

- File mẫu `D:\nhung\demo.c`: không phải chương trình game, có hàm `main` riêng; không thêm cùng `Snake_Game_ATmega128\main.c`.
- Sáu source gameplay cũ: `snake_game.c`, `game_render.c`, `game_menu.c`, `game_map.c`, `game_mode.c`, `dynamic_obstacle.c`. Chúng đã gộp vào `game\game.c`; xóa các mục cũ khỏi project nếu còn.
- Module `bonus_food.c/.h` đã được loại bỏ.
- Mọi source trong `tests/`, kể cả `mock_display.c` và `tests/mock/`.
- File `.md`, `.cci`, `.hci`, bản sao `~` và các file output build cũ.

## Các bước build và chạy

1. Mở `D:\nhung\demo.prj` bằng CodeVisionAVR (hoặc `demo.cproj` khi dùng tích hợp IDE). Project này đã cấu hình 8 source ở bảng trên. Nếu tự tạo project, chọn ATmega128 rồi thêm đúng 8 file đó vào danh sách source.
2. Nếu CodeVision đang mở project cũ, đóng rồi mở lại `demo.prj` để tải danh sách mới; kiểm tra chỉ có một source chứa hàm `main`, là `Snake_Game_ATmega128\main.c`.
3. Kiểm tra CPU clock khớp với Proteus/board; cấu hình `glcd.h` cho PCD8544 và font `font5x7`, `alcd.h` ở chế độ 4 bit, các include path và pin mapping. Các thư viện ngoại vi này thuộc CodeVisionAVR, không thay bằng header mock của test.
4. Trong `config/board_config.h`, đặt cả bốn macro `KEYPAD_TEST_MODE`, `LED7SEG_TEST_MODE`, `LCD1602_TEST_MODE`, `LED7_LCD_TEST_MODE` bằng `0` để chạy game đầy đủ.
5. Chạy **Project → Build All**. Xem Errors/Warnings, bộ nhớ sử dụng, EEPROM memory map và đường dẫn HEX mới trong kết quả build.
6. Trong Proteus, gán HEX mới vào **Program File** của ATmega128, kiểm tra **Clock Frequency**, rồi chạy mô phỏng. Trên board thật, nạp HEX bằng programmer phù hợp sau khi kiểm tra phần cứng.

Firmware dùng CodeVisionAVR; không build bằng AVR-GCC, Arduino, C++ hoặc Makefile. Kiểm thử host không tạo HEX. Bản source sau khi gộp đã qua test host; Build All và Proteus của bản này vẫn cần thực hiện trên Windows.

## Mô phỏng và kiểm thử

Trong Proteus, mở schematic của board, gán HEX mới vào **Program File** của ATmega128 và xác nhận **Clock Frequency**. Kiểm tra menu, phím điều khiển, ba Level, ba Map, hai Mode, điểm trên LED, trạng thái trên LCD và hình vẽ trên Nokia. Thử riêng Wrap Around, Reverse Snake, vật cản động, Pause/Restart và kỷ lục sau khi reset MCU.

Trên Ubuntu, chạy kiểm thử logic bằng:

```sh
sh tests/run_host_validation.sh
```

Test host dùng mock để kiểm tra thuật toán; nó không tạo HEX và không thay thế Build All hoặc Proteus. [Báo cáo kiểm định mở rộng](docs/final_extended_validation_report.md) ghi ma trận 18 tổ hợp, kết quả đã kiểm tra và các mục còn cần xác minh trên CVAVR, Proteus và board thực.

Trước khi nạp board, kiểm tra nguồn, clock, mức điện áp Nokia, kết nối ISP và fuse. Sao lưu EEPROM; kiểm tra file `.eep`, chip erase và EESAVE để không ghi đè kỷ lục cũ.

### Chướng ngại vật Survival

- Mỗi chướng ngại vật là một thanh ngang đặc dài 3 ô (18 × 6 pixel).
- Mỗi 10 giây chơi thử thêm 1 thanh, tối đa 8 thanh; LCD đếm số thanh.
- Áp dụng cho Classic, Pillars và Labyrinth ở cả 3 độ khó. Cả thanh phải nằm trong màn hình, tránh tường, rắn, thức ăn và vùng cách đầu rắn tối đa 2 bước.
- Chỉ đặt nếu các ô trống còn thông nhau theo luật xuyên biên của độ khó. Không tìm được vị trí hợp lệ thì bỏ qua lần sinh đó.
