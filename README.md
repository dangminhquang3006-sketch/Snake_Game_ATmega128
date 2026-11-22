# Snake Game trên ATmega128

Trò chơi rắn săn mồi viết bằng Embedded C cho ATmega128, biên dịch bằng CodeVisionAVR. Nokia 5110 hiển thị game, keypad 3×3 điều khiển, LED 7 đoạn hiển thị điểm và LCD 16×2 hiển thị trạng thái.

## Phần 1. Hai chế độ chơi

Bản 8 file gốc có hai chế độ NORMAL và HARD; mặc định là NORMAL.

| Chế độ | Tốc độ | Luật chơi |
|---|---|---|
| NORMAL | 300 ms/ô | Đi qua một cạnh sẽ xuất hiện ở cạnh đối diện |
| HARD | 250 ms/ô | Xuyên biên và đảo đầu–đuôi sau khi ăn thức ăn, nếu chưa thắng |

Cả NORMAL và HARD đều thử sinh thêm một rào chắn ngang dài 3 ô mỗi **10 giây đang chơi**, tối đa 8 rào chắn. Nếu không có vị trí hợp lệ thì bỏ qua lượt sinh; tạm dừng không tính thời gian, chơi lại xóa rào chắn và đặt lại thời gian.

Ở cả hai chế độ, rắn thua khi đâm vào thân hoặc vật cản; chiếm hết các ô chơi hợp lệ thì thắng.

## Phần 2. Ba bản đồ (Map)

| Map | Đặc điểm |
|---|---|
| CLASSIC | Không có tường cố định, gồm 112 ô chơi |
| PILLARS | Có các cụm cột chắn, còn 104 ô chơi |
| LABYRINTH | Có các vách tạo hành lang, còn 100 ô chơi |

Số ô trên được tính trước khi sinh rào chắn.

Chọn **NORMAL hoặc HARD → Map → bắt đầu chơi**, không có bước chọn Mode riêng. Có **6 tổ hợp** chơi.

Dùng **2/8** để chọn lên/xuống trong menu, **5** xác nhận. Trong game: **2/4/6/8** đổi hướng, **5** tạm dừng/tiếp tục, **9** chơi lại. Phím **1** quay lại bước chọn trước trong menu Map hoặc về menu Level sau khi thắng/thua.

## Các file cần thêm vào CodeVisionAVR

Chọn một trong hai cách:

### Cách 1: Dùng 8 file gốc

Thêm 8 file sau vào project, giữ nguyên các thư mục và file `.h` đi kèm:

1. `main.c`
2. `game/game.c`
3. `drivers/display.c`
4. `drivers/keypad.c`
5. `drivers/timer.c`
6. `drivers/led7seg.c`
7. `drivers/lcd1602.c`
8. `storage/highscore_eeprom.c`

### Cách 2: Dùng bản rút gọn 2 file

Bản rút gọn chưa cập nhật các thay đổi chế độ chơi này. Lấy 2 file trong thư mục `export/codevision/`:

- `snake_firmware.c`
- `board_config.h`

Đặt hai file cùng thư mục, chỉ thêm `snake_firmware.c` vào project. Không thêm cùng 8 file gốc.
