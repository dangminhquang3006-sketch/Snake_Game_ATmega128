# Snake Game trên ATmega128

Trò chơi rắn săn mồi viết bằng Embedded C cho ATmega128, biên dịch bằng CodeVisionAVR. Nokia 5110 hiển thị game, keypad 3×3 điều khiển, LED 7 đoạn hiển thị điểm và LCD 16×2 hiển thị trạng thái.

## Phần 1. Hai chế độ chơi

Cả bản 8 file gốc và bản gộp 2 file đều có hai chế độ NORMAL và HARD; mặc định là NORMAL.

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

### Cách 2: Dùng bản gộp 2 file

Bản gộp được sinh từ đúng 8 file của Cách 1, có cùng logic và tính năng hiện tại. Lấy 2 file trong thư mục `export/codevision/`:

- `snake_firmware.c`
- `board_config.h`

Đặt hai file cùng thư mục, chỉ thêm `snake_firmware.c` vào danh sách source của project. `board_config.h` được include tự động; không thêm cùng 8 file gốc để tránh trùng hàm và biến.

Giữ cấu hình project như Cách 1:

- Chip **ATmega128**, clock **8 MHz**, Data Stack **1024 byte** như project hiện tại.
- Thư viện GLCD **Nokia 5110/PCD8544**: DC=PG0, DIN=PG1, CLK=PG2, RST=PG3, CE/CS=PG4.
- Thư viện LCD 16×2: RS=PA0, R/W=PA1, E=PA2, D4..D7=PA3..PA6.
- Cả bốn macro kiểm tra ngoại vi trong `board_config.h` bằng `0` để chạy game.

Luồng chơi vẫn là **Level → Map → chơi**, có 6 tổ hợp, xuyên biên ở cả hai Level, đảo đầu–đuôi khi ăn trong HARD, thanh chắn 3 ô mỗi 10 giây RUNNING, Pause/Resume, Restart, WIN/GAME OVER và kỷ lục EEPROM.

Sau khi sửa bản 8 file, tạo lại bản gộp từ thư mục gốc của dự án:

```sh
python3 export/generate_codevision.py
```

Kiểm tra hai file đã đồng bộ mà không ghi lại:

```sh
python3 export/generate_codevision.py --check
```

Chỉnh logic ở các source gốc rồi tạo lại bản gộp; chỉnh chân hoặc tham số cho một bản sao độc lập tại `board_config.h` đi kèm bản sao đó. Hai file firmware không cần các thư mục source gốc khi chuyển sang project khác, nhưng vẫn cần thư viện và cấu hình CodeVisionAVR nêu trên.

## Tài liệu giải thích tính năng

Xem [danh mục 12 tài liệu tính năng](tinh_nang/TINH_NANG.md#10-tài-liệu-giải-thích-chi-tiết-từng-tính-năng). Mỗi file có ví dụ minh họa và giải thích chi tiết cách chương trình thực hiện; gồm menu, map, keypad, xuyên biên, ăn mồi, đảo đầu–đuôi, vật cản, Pause/Resume, WIN/GAME OVER, EEPROM và hiển thị.
