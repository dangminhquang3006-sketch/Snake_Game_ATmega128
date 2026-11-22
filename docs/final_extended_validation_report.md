# Báo cáo kiểm định — STANDARD và SURVIVAL

Danh sách chính xác 8 file `.c`, 13 header phải giữ kèm và các bước thêm source/build nằm trong [hướng dẫn CodeVision ở README](../README.md#cần-thêm-những-file-nào-vào-codevision-để-chạy).

Ngày cập nhật: 2026-11-22 (Asia/Bangkok).

## Phạm vi hiện tại

Đã loại bỏ BONUS và TIME_ATTACK khỏi menu, logic game, hiển thị Nokia/LCD và cấu hình build. Module `bonus_food.c/.h`, bộ đếm 60 giây và trạng thái TIME_UP đã được gỡ. Ba Level và ba Map được giữ nguyên. Firmware còn **8 file `.c`** sau khi gộp sáu implementation gameplay vào `game/game.c`.

SURVIVAL giữ nguyên lịch sinh một thanh ngang dài 3 ô mỗi 10 giây RUNNING, tối đa 8 thanh. Pause dừng thời gian sinh vật cản; Restart xóa vật cản và đặt lại lịch. STANDARD không sinh vật cản động.

## Cấu trúc source sau khi gộp

Sáu file `game_mode.c`, `game_map.c`, `snake_game.c`, `dynamic_obstacle.c`, `game_menu.c`, `game_render.c` đã được gộp trực tiếp thành `game/game.c`. Các phần trong file có chú thích phân cách; các hàm API, header, luật chơi, GPIO, Timer1 và định dạng EEPROM được giữ nguyên.

Danh sách biên dịch: `main.c`, `game/game.c`, `drivers/display.c`, `drivers/keypad.c`, `drivers/timer.c`, `drivers/led7seg.c`, `drivers/lcd1602.c`, `storage/highscore_eeprom.c`. `demo.prj` và `demo.cproj` đã cập nhật. Header vẫn nằm trong cây source; không cần thêm vào danh sách `.c`.

Hai harness Snake/Survival include implementation `game.c` để dựng trạng thái biên; harness EEPROM liên kết `game.c` như source độc lập. `tests/mock_display.c` cung cấp mock Nokia phục vụ liên kết toàn bộ gameplay trên host; không đưa vào firmware.

## Ma trận 18 tổ hợp

PASS host smoke nghĩa là đã chọn Level/Map/Mode, khởi tạo ván và ăn thức ăn đầu tiên; không đại diện cho kiểm thử đầy đủ trên MCU.

| Level | Map | Mode | Host smoke | Proteus |
|---|---|---|---|---|
| EASY | CLASSIC | STANDARD | PASS | NOT RUN |
| EASY | CLASSIC | SURVIVAL | PASS | NOT RUN |
| EASY | PILLARS | STANDARD | PASS | NOT RUN |
| EASY | PILLARS | SURVIVAL | PASS | NOT RUN |
| EASY | LABYRINTH | STANDARD | PASS | NOT RUN |
| EASY | LABYRINTH | SURVIVAL | PASS | NOT RUN |
| NORMAL | CLASSIC | STANDARD | PASS | NOT RUN |
| NORMAL | CLASSIC | SURVIVAL | PASS | NOT RUN |
| NORMAL | PILLARS | STANDARD | PASS | NOT RUN |
| NORMAL | PILLARS | SURVIVAL | PASS | NOT RUN |
| NORMAL | LABYRINTH | STANDARD | PASS | NOT RUN |
| NORMAL | LABYRINTH | SURVIVAL | PASS | NOT RUN |
| HARD | CLASSIC | STANDARD | PASS | NOT RUN |
| HARD | CLASSIC | SURVIVAL | PASS | NOT RUN |
| HARD | PILLARS | STANDARD | PASS | NOT RUN |
| HARD | PILLARS | SURVIVAL | PASS | NOT RUN |
| HARD | LABYRINTH | STANDARD | PASS | NOT RUN |
| HARD | LABYRINTH | SURVIVAL | PASS | NOT RUN |

## Kết quả kiểm thử

Sau khi gộp, chạy `sh tests/run_host_validation.sh`: cả 4 nhóm đạt với C89, `-Wall -Wextra -Werror`.

| Nhóm | Kết quả và phạm vi |
|---|---|
| Snake | PASS: thức ăn, điểm, va chạm, Pause/Restart, xuyên biên, đảo rắn HARD, Win và 18 tổ hợp |
| Survival | PASS: thanh 3 ô trên 3 Map × 3 Level × 32 seed; khoảng cách an toàn, liên thông, va chạm và lịch sinh |
| EEPROM | PASS: dữ liệu v1/v2/v3/v4, CRC, fallback slot hỏng, sequence qua 65535→0 và 18 kỷ lục hiện tại |
| LCD | PASS với mock: hai mode, số vật cản, Pause và kết quả |
| CodeVisionAVR Build All | NOT RUN: chưa tạo HEX mới trong môi trường này |
| Proteus và board thật | NOT RUN: chưa kiểm tra firmware mới trên phần cứng/mô phỏng |

## Tương thích kỷ lục

GameMode hiện tại có STANDARD=0 và SURVIVAL=1. Module lưu trữ ánh xạ SURVIVAL sang nhóm thứ tư của EEPROM v4 (offset 27), giữ nguyên địa chỉ và kích thước hai slot cũ. Không dùng chỉ số enum mới để đọc trực tiếp dữ liệu cũ.

Định dạng v4 vẫn chứa 36 ô; 18 ô của hai mode đã bỏ chỉ được giữ để kiểm tra và bảo toàn bản ghi cũ. API gameplay chỉ cho phép 18 tổ hợp còn lại. Bộ đọc vẫn chấp nhận các bản ghi hợp lệ chứa điểm của hai mode cũ. Dữ liệu v1–v3 tiếp tục được đọc; kỷ lục hai mode đã bỏ không được chuyển thành kỷ lục Survival.

## Việc cần xác minh trên Windows

1. Mở `demo.prj`, xác nhận 8 source và bốn macro test ngoại vi bằng 0; chạy CodeVisionAVR **Build All**, lưu log Errors/Warnings, Flash/SRAM/stack và EEPROM memory map.
2. Gán HEX vừa build cho Proteus và kiểm tra 18 tổ hợp; menu chỉ có STANDARD/SURVIVAL. Kiểm tra keypad, Nokia/LCD/LED, va chạm, Pause/Restart, Win và Survival mỗi 10 giây RUNNING.
3. Thử đọc kỷ lục STANDARD/SURVIVAL từ bản sao EEPROM v4 cũ, rồi lưu kỷ lục mới và reset MCU để kiểm tra persistence.

Các báo cáo cũ là lịch sử kiểm định; không dùng kết quả trước đây để tuyên bố bản firmware này đã đạt trên CodeVisionAVR, Proteus hoặc board thật.
