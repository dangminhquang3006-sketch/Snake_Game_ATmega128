# Báo cáo kiểm định — NORMAL và HARD có rào chắn

Cập nhật: 2026-11-22 (Asia/Bangkok). Bản 8 file gốc có **2 Level × 3 Map = 6 tổ hợp**. Không còn EASY hoặc bước chọn STANDARD/SURVIVAL. Bản rút gọn chưa cập nhật.

Chọn **NORMAL/HARD → Map → game**. NORMAL 300 ms/ô; HARD 250 ms/ô và đảo đầu–đuôi sau khi ăn thức ăn nếu chưa thắng. Cả hai xuyên biên và thử sinh một rào chắn ngang 3 ô mỗi 10 giây RUNNING, tối đa 8 thanh. Pause dừng lịch, Restart đặt lại lịch và xóa rào chắn.

## Ma trận 6 tổ hợp

Host kiểm tra khởi tạo, ăn thức ăn, sinh rào chắn ở mốc 10 giây, Pause và Restart. PASS host không xác nhận timing hoặc tín hiệu GPIO trên MCU.

| Level | Map | Host | Proteus bản hiện tại |
|---|---|---|---|
| NORMAL | CLASSIC | PASS | NOT RUN |
| NORMAL | PILLARS | PASS | NOT RUN |
| NORMAL | LABYRINTH | PASS | NOT RUN |
| HARD | CLASSIC | PASS | NOT RUN |
| HARD | PILLARS | PASS | NOT RUN |
| HARD | LABYRINTH | PASS | NOT RUN |

## Kết quả kiểm thử

- `sh tests/run_host_validation.sh`: PASS 4 nhóm Snake, rào chắn, EEPROM mock và LCD mock.
- Rào chắn: kiểm tra 3 Map × 2 Level × 32 seed; cả thanh nằm trong màn hình, tránh rắn/thức ăn/tường/vùng an toàn quanh đầu, giữ các ô chơi liên thông theo luật xuyên biên. Không có vị trí hợp lệ thì bỏ qua lần sinh.
- EEPROM: giữ 36 ô v4 và đọc v1–v3; gameplay dùng 6 kỷ lục ở nhóm SURVIVAL cũ (offset 27), giữ NORMAL=1, HARD=2. Nhóm STANDARD cũ được bảo toàn, không đưa vào lựa chọn chơi.
- LCD: hiển thị Level/Map, số rào chắn và PLAYING/PAUSED; không còn màn chọn Mode hoặc nhãn SURVIVAL trong gameplay.
- `python3 tests/run_export_validation.py`: PASS với bản ghép tạm; không cập nhật thư mục bản rút gọn.
- CodeVisionAVR Build All, Proteus và board thật của bản hiện tại: NOT RUN. Artifact và smoke cũ được giữ trong [ghi chú bàn giao](session_notes.md).

## Kiểm tra tiếp trên Windows

Dùng [8 file gốc](../README.md#các-file-cần-thêm-vào-codevisionavr), Build All rồi nạp HEX mới vào Proteus. Kiểm tra cả 6 tổ hợp, luồng chọn Level → Map, điều hướng, rào chắn sau 10 giây RUNNING, Pause/Resume, Restart, Win/Game Over, hiển thị và EEPROM qua reset.
