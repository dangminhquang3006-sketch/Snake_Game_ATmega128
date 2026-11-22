# Báo cáo kiểm định — Snake Game ATmega128

Cập nhật: 2026-11-22 (Asia/Bangkok). Áp dụng cho bản **8 file gốc**; bản rút gọn trong `export/codevision/` chưa cập nhật thay đổi gameplay.

## Cấu trúc và luật hiện tại

Danh sách 8 file cần thêm vào CodeVisionAVR nằm trong [README](../README.md#các-file-cần-thêm-vào-codevisionavr). `main.c` điều phối Level → Map → game; `game/game.c` chứa logic rắn, bản đồ, rào chắn, menu và render. Giữ các header đi kèm.

- **NORMAL:** 300 ms/ô, xuyên biên.
- **HARD:** 250 ms/ô, xuyên biên và đảo đầu–đuôi sau khi ăn thức ăn nếu chưa thắng.
- **Cả hai:** thử thêm một rào chắn ngang dài 3 ô mỗi 10 giây RUNNING, tối đa 8 rào chắn. Không có vị trí hợp lệ thì bỏ qua lượt sinh. Pause dừng thời gian; Restart xóa rào chắn và đặt lại lịch.
- **Map:** CLASSIC 112 ô, PILLARS 104 ô, LABYRINTH 100 ô trước khi sinh rào chắn. Tổng **6 tổ hợp** Level × Map, không còn màn chọn STANDARD/SURVIVAL.
- **Thức ăn:** CIRCLE 60%, +10; SQUARE 35%, +20; STAR 5%, +50 điểm. Mỗi loại tăng chiều dài một đốt.
- Va chạm thân hoặc vật cản thì thua; chiếm hết ô hợp lệ thì thắng. Phím 5 Pause/Resume, 9 Restart, 1 về menu Level sau khi kết thúc.

LCD khi chơi hiển thị Level, Map, số rào chắn và trạng thái PLAYING/PAUSED. Timer1 giữ tick 1 ms; xử lý gameplay, render và ghi EEPROM nằm trong vòng lặp chính.

## Tương thích kỷ lục

NORMAL=1 và HARD=2 giữ nguyên ID lưu trữ. Gameplay dùng 6 kỷ lục Level × Map ở nhóm SURVIVAL cũ của EEPROM v4 (offset 27), vì cùng luật rào chắn. Menu Level/Map và kết quả cuối ván dùng cùng nhóm này.

EEPROM v4 vẫn giữ 36 ô; các nhóm cũ được bảo toàn. API storage vẫn hỗ trợ nhóm STANDARD cũ để tương thích, không phải lựa chọn gameplay. Bộ đọc v1–v3, CRC, sequence, hai slot và commit marker được giữ nguyên; chỉ ghi khi có kỷ lục mới.

## Kiểm thử

`sh tests/run_host_validation.sh` đạt 4/4 nhóm với C89, `-Wall -Wextra -Werror`:

| Nhóm | Kết quả |
|---|---|
| Snake | PASS: 6 tổ hợp, rào chắn ở cả hai Level, mốc 10 giây, Pause/Restart, thức ăn, va chạm, xuyên biên, đảo rắn và Win |
| Rào chắn | PASS: 3 Map × 2 Level × 32 seed, thanh 3 ô, liên thông và va chạm |
| EEPROM | PASS mock: đọc v1–v4, bảo toàn nhóm cũ, 6 kỷ lục đang dùng, CRC và sequence tràn |
| LCD | PASS mock: Level/Map, số rào chắn, PLAYING/PAUSED và kết quả |

`python3 tests/run_export_validation.py` cũng đạt: kiểm tra cú pháp toàn firmware và 4 chế độ test ngoại vi qua bản ghép tạm, chạy lại 4 nhóm logic. Các file trong `export/codevision/` không được ghi lại.

Chưa Build All bằng CodeVisionAVR, chạy Proteus hoặc kiểm tra board thật cho bản gameplay hiện tại. HEX/log và smoke Proteus trước đây thuộc bản cũ, xem [ghi chú bàn giao](session_notes.md).

## Việc còn lại

Build All bản 8 file trong CodeVisionAVR, rồi kiểm tra [6 tổ hợp](final_extended_validation_report.md) trên Proteus: menu Level → Map, rào chắn mỗi 10 giây RUNNING, Pause/Restart, các màn hình và kỷ lục qua reset MCU.
