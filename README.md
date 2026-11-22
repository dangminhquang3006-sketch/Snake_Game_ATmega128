# Snake Game trên ATmega128

Trò chơi rắn săn mồi viết bằng Embedded C cho ATmega128, biên dịch bằng CodeVisionAVR. Nokia 5110 hiển thị game, keypad 3×3 điều khiển, LED 7 đoạn hiển thị điểm và LCD 16×2 hiển thị trạng thái.

## Phần 1. Hai chế độ chơi

Cả bản 7 file gốc và bản gộp 2 file đều có hai chế độ NORMAL và HARD; mặc định là NORMAL.

| Chế độ | Tốc độ | Luật chơi |
|---|---|---|
| NORMAL | 300 ms/ô | Đi qua một cạnh sẽ xuất hiện ở cạnh đối diện |
| HARD | 250 ms/ô | Xuyên biên và đảo đầu–đuôi sau khi ăn thức ăn, nếu chưa thắng |

Cả NORMAL và HARD đều thử sinh thêm một rào chắn ngang dài 3 ô mỗi **10 giây đang chơi**, tối đa 8 rào chắn. Nếu không có vị trí hợp lệ thì bỏ qua lượt sinh; chơi lại xóa rào chắn và đặt lại thời gian.

Ở cả hai chế độ, rắn thua khi đâm vào thân hoặc vật cản; chiếm hết các ô chơi hợp lệ thì thắng.

## Phần 2. Ba bản đồ (Map)

| Map | Đặc điểm |
|---|---|
| CLASSIC | Không có tường cố định, gồm 112 ô chơi |
| PILLARS | Có các cụm cột chắn, còn 104 ô chơi |
| LABYRINTH | Có các vách tạo hành lang, còn 100 ô chơi |

Số ô trên được tính trước khi sinh rào chắn.

Chọn **NORMAL hoặc HARD → Map → bắt đầu chơi**, không có bước chọn Mode riêng. Có **6 tổ hợp** chơi.

Dùng **2/8** để chọn lên/xuống trong menu, **5** xác nhận. Trong game: **2/4/6/8** đổi hướng, **9** chơi lại. Phím **1** quay lại bước chọn trước trong menu Map hoặc về menu Level sau khi thắng/thua.

## Chọn kiểu LED 7 đoạn CA/CC

- **BT1 — PB2, chân 12:** chọn Common-Anode (CA).
- **BT2 — PB3, chân 13:** chọn Common-Cathode (CC).
- Hai nút nhấn xuống GND, dùng pull-up nội; chống dội 20 ms, cần nhả trước lần nhấn tiếp theo.
- CA đảo mẫu segment bằng NOT, digit chọn mức cao. CC giữ mẫu segment, digit chọn mức thấp.
- Lựa chọn được lưu tại EEPROM `0x03F0`, khôi phục khi khởi động. Dữ liệu chưa hợp lệ thì dùng CA; không ghi lặp khi giữ nút hoặc chọn lại mode đã lưu.
- Có thể đổi mode ở menu, khi chơi hoặc ở màn kết quả. Nếu nhấn cả hai nút thì bỏ qua đến khi nhả hết.

Hai cụm CA/CC trong mạch hiện dùng chung các net `7SLED`; lựa chọn này đổi cực tính bus. Để điều khiển bật/tắt từng cụm độc lập cần có đường enable hoặc mạch cách ly riêng.

## Các file cần thêm vào CodeVisionAVR

Chọn một trong hai cách:

### Cách 1: Dùng 7 file gốc

Thêm 7 file sau vào project, giữ nguyên các thư mục và file `.h` đi kèm:

1. `main.c`
2. `game/game.c`
3. `drivers/display.c`
4. `drivers/keypad.c`
5. `drivers/timer.c`
6. `drivers/led7seg.c`
7. `drivers/lcd1602.c`

### Cách 2: Dùng bản gộp 2 file

Bản gộp được sinh từ đúng 7 file của Cách 1, có cùng logic và tính năng hiện tại. Lấy 2 file trong thư mục `export/codevision/`:

- `snake_firmware.c`
- `board_config.h`


## Tài liệu giải thích tính năng

Xem [danh mục 11 tài liệu tính năng](tinh_nang/TINH_NANG.md#9-tài-liệu-giải-thích-chi-tiết-từng-tính-năng). Mỗi file có ví dụ minh họa và giải thích chi tiết cách chương trình thực hiện; gồm menu, map, keypad, xuyên biên, ăn mồi, đảo đầu–đuôi, vật cản, WIN/GAME OVER, hiển thị và chọn CA/CC.
