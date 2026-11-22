# Báo cáo kiểm định — Snake Game ATmega128

Danh sách chính xác 8 file `.c`, 13 header phải giữ kèm và các bước thêm source/build nằm trong [hướng dẫn CodeVision ở README](../README.md#cần-thêm-những-file-nào-vào-codevision-để-chạy).

Cập nhật: 2026-11-22 (Asia/Bangkok). Báo cáo này mô tả cấu trúc source hiện tại sau khi bỏ hai mode và gộp gameplay.

## Cấu trúc firmware hiện tại

Firmware có **8 file `.c`** trong danh sách biên dịch:

| File | Chức năng |
|---|---|
| `main.c` | Điều phối menu Level → Map → Mode và vòng lặp game |
| `game/game.c` | Logic rắn, Map, Mode, menu, render Nokia và vật cản Survival |
| `drivers/display.c` | Driver Nokia 5110 |
| `drivers/keypad.c` | Quét và xử lý keypad |
| `drivers/timer.c` | Tick Timer1 |
| `drivers/led7seg.c` | Hiển thị điểm trên LED 7 đoạn |
| `drivers/lcd1602.c` | Hiển thị trạng thái trên LCD 16×2 |
| `storage/highscore_eeprom.c` | Đọc và lưu kỷ lục EEPROM |

Sáu implementation gameplay cũ đã được gộp trực tiếp vào `game/game.c`. Các header `.h` vẫn được giữ để khai báo API; không thêm header vào danh sách source biên dịch. `demo.prj` và `demo.cproj` trong thư mục cha đã được cập nhật.

## Luật chơi hiện tại

- **Level:** EASY 500 ms/ô, chạm biên thì thua; NORMAL 300 ms/ô, xuyên biên; HARD 150 ms/ô, xuyên biên và đảo đầu–đuôi sau khi ăn thức ăn.
- **Map:** CLASSIC 112 ô, PILLARS 104 ô, LABYRINTH 100 ô trước khi sinh vật cản động.
- **Mode:** STANDARD và SURVIVAL, tổng cộng **18 tổ hợp** Level × Map × Mode. BONUS, TIME_ATTACK và trạng thái TIME_UP đã được loại bỏ.
- **Survival:** thử sinh một thanh ngang dài 3 ô (18 × 6 pixel) mỗi 10 giây RUNNING, tối đa 8 thanh. Không có vị trí hợp lệ thì bỏ qua lượt sinh. Pause dừng thời gian; Restart xóa vật cản và đặt lại lịch.
- **Thức ăn:** CIRCLE 60%, +10 điểm; SQUARE 35%, +20 điểm; STAR 5%, +50 điểm. Mỗi loại tăng chiều dài một đốt.
- Va chạm thân hoặc vật cản gây Game Over; chiếm hết ô hợp lệ chuyển sang WIN. Phím 5 Pause/Resume, 9 Restart, 1 về menu Level sau khi kết thúc.

## Thời gian, ngoại vi và EEPROM

ISR Timer1 chỉ cập nhật tick; xử lý gameplay, render và ghi EEPROM nằm trong vòng lặp chính. Việc gộp file giữ nguyên GPIO, cấu hình Timer1 và driver ngoại vi.

EEPROM vẫn dùng định dạng v4 gồm 36 ô lưu để giữ tương thích dữ liệu cũ; API hiện tại chỉ cung cấp 18 tổ hợp STANDARD/SURVIVAL. SURVIVAL có enum runtime bằng 1 nhưng được ánh xạ về nhóm lưu cũ tại offset 27. Hai nhóm mode đã bỏ không được đọc nhầm thành kỷ lục Survival. Đường đọc dữ liệu v1–v3, CRC, hai slot và commit marker được giữ nguyên. Chỉ ghi khi có kỷ lục mới.

## Kiểm thử đã thực hiện sau khi gộp

Lệnh: `sh tests/run_host_validation.sh`, dùng C89 với `-Wall -Wextra -Werror`.

| Nhóm | Kết quả |
|---|---|
| Snake, thức ăn, xuyên biên, đảo rắn, va chạm, Pause/Restart, Win, 18 tổ hợp | PASS host |
| Vật cản Survival trên 3 Map × 3 Level × 32 seed, liên thông và va chạm | PASS host |
| EEPROM v1/v2/v3/v4, 18 kỷ lục, CRC, slot lỗi và sequence tràn | PASS host mock |
| LCD hai mode, số vật cản, Pause và kết quả | PASS host mock |
| Build All bằng CodeVisionAVR của source sau khi gộp | NOT RUN |
| Proteus với HEX mới và board thật | NOT RUN |

Hai harness Snake/Survival dùng trực tiếp `game.c`; harness EEPROM liên kết `game.c` độc lập. Mock hiển thị chỉ phục vụ host. Kết quả host không chứng minh timing ngoại vi hoặc tương thích compiler CodeVisionAVR cho toàn bộ firmware.

## Hoàn tất kiểm định trên Windows

1. Mở `D:\nhung\demo.prj` bằng CodeVisionAVR, kiểm tra 8 source theo bảng trên. Nếu dùng project riêng, bỏ sáu mục implementation gameplay cũ và thêm `game/game.c` một lần. Giữ các header đi kèm.
2. Xác nhận chip ATmega128, clock khớp với Proteus/board, cấu hình `glcd.h` PCD8544/font `font5x7`, `alcd.h` 4 bit và bốn macro test ngoại vi bằng 0. Không đưa `tests/` hoặc mock vào firmware.
3. Chạy **Project → Build All**; lưu Errors/Warnings, Flash/SRAM/stack, EEPROM memory map và đường dẫn HEX mới.
4. Gán đúng HEX mới cho Proteus; kiểm tra 18 tổ hợp, keypad/Nokia/LCD/LED, chu kỳ di chuyển, Survival mỗi 10 giây RUNNING, Pause/Restart, Win và kỷ lục sau reset MCU.
5. Trước khi nạp board, xác minh nguồn, clock, mức logic Nokia và kết nối ISP; sao lưu EEPROM/fuse, kiểm tra `.eep`, EESAVE và chip erase để bảo toàn kỷ lục. Kiểm tra lại sau nạp và mất nguồn.

Source đã đạt 4 nhóm kiểm thử host sau khi gộp; chưa có kết quả Build All, Proteus hoặc board thật cho cấu trúc mới.

Xem [README](../README.md) để biết hướng dẫn sử dụng/build và [báo cáo mở rộng](final_extended_validation_report.md) để xem ma trận 18 tổ hợp.
