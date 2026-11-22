# Điều khiển rắn bằng keypad 3×3

## 1. Ví dụ dễ hình dung

Các phím trong mạch được đánh số theo hàng:

```text
             COL1   COL2   COL3
ROW1           1      2      3
ROW2           4      5      6
ROW3           7      8      9
```

Trong game, 2/8/4/6 lần lượt điều khiển lên/xuống/trái/phải; 5 không có hành động trong ván; 9 chơi lại. Trong menu, 2/8 di chuyển lựa chọn và 5 xác nhận. Phím 1 dùng quay lại menu theo trạng thái ứng dụng.

Giả sử người chơi giữ phím 5. Chương trình chỉ phát **một sự kiện nhấn** sau khi phím ổn định đủ 20 ms; nó không xác nhận lặp nhiều lần trong lúc giữ.

| Thời điểm minh họa | Mức đọc | Xử lý |
|---|---|---|
| 100 ms | Bắt đầu thấy phím 5 | Ghi nhận ứng viên, bắt đầu đếm chống dội |
| 119 ms | Vẫn phím 5 | Chưa đủ 20 ms, chưa phát sự kiện |
| 120 ms | Vẫn phím 5 | Xác nhận nhấn, phát một sự kiện |
| 150 ms | Vẫn giữ phím 5 | Không phát thêm |
| 200 ms | Bắt đầu nhả | Chờ nhả ổn định |
| 220 ms | Không phím đủ 20 ms | Mở khóa cho lần nhấn tiếp theo |

Các thời điểm là ví dụ theo đồng hồ MCU, giả sử vòng lặp được phục vụ ở các mốc trên. Tốc độ quét thực tế phụ thuộc vòng lặp chính.

## 2. Cách chương trình thực hiện

Quét phần cứng và chống dội nằm trong [drivers/keypad.c](../drivers/keypad.c); tên sự kiện ở [drivers/keypad.h](../drivers/keypad.h). [main.c](../main.c) lấy sự kiện, còn [game/game.c](../game/game.c) quyết định có nhận hướng rẽ hay không.

### 2.1. Phân biệt vị trí phím và sự kiện chức năng

Vị trí thô là số 1–9. `KEY_RELEASED=0` nghĩa là không có phím, còn `KEY_MULTIPLE=10` nghĩa là lượt quét phát hiện nhiều vị trí đang nhấn.

`KeyEvent` là kiểu dữ liệu khác, gồm các tên chức năng UP/DOWN/LEFT/RIGHT/CONFIRM/RESTART/MENU. Ví dụ vị trí 8 tạo sự kiện DOWN; không được lấy số thứ tự enum để suy ra số in trên keypad.

Hai vị trí 3 và 7 không gán chức năng trong game hiện tại. Chúng vẫn là các vị trí có thể đọc ở chế độ kiểm tra keypad, nhưng không tạo lệnh điều khiển thông thường.

### 2.2. Các chân hàng và cột

Theo [config/board_config.h](../config/board_config.h):

| Đường keypad | Chân ATmega128 | Cấu hình |
|---|---|---|
| ROW1 | PC6 | Input, bật pull-up |
| ROW2 | PE1 | Input, bật pull-up |
| ROW3 | PD0 | Input, bật pull-up |
| COL1 | PC4 | Output, kéo thấp khi được chọn |
| COL2 | PC5 | Output, kéo thấp khi được chọn |
| COL3 | PE0 | Output, kéo thấp khi được chọn |

Pull-up giữ hàng ở mức 1 khi không nhấn. Khi một cột bị kéo xuống 0 và nút nối cột đó với một hàng, hàng tương ứng được đọc là 0. Vì vậy code dùng toán tử `!` để đổi mức thấp thành giá trị “đang nhấn”.

### 2.3. Khởi tạo GPIO và trạng thái bộ quét

```c
void keypad_init(void)
{
    KEY_ROW1_DDR=0; KEY_ROW1_PORT=1;
    KEY_ROW2_DDR=0; KEY_ROW2_PORT=1;
    KEY_ROW3_DDR=0; KEY_ROW3_PORT=1;
    columns_high();
    KEY_COL1_DDR=1;
    KEY_COL2_DDR=1;
    KEY_COL3_DDR=1;
    candidate=KEY_RELEASED; stable=KEY_RELEASED;
    changed_at=0; last_scan=0; scanned=0; locked=0;
    pending=KEY_NONE; pending_position=KEY_RELEASED;
}
```

`DDR=0` đặt hàng làm input, `PORT=1` bật pull-up. Trước khi đặt các cột thành output, `columns_high()` đưa giá trị cột về mức 1 để trạng thái nghỉ không chọn cột nào.

| Biến | Ý nghĩa |
|---|---|
| `candidate` | Giá trị thô đang được chờ ổn định |
| `stable` | Giá trị đã qua chống dội gần nhất |
| `changed_at` | Mốc bắt đầu thấy ứng viên hiện tại |
| `last_scan`, `scanned` | Ngăn quét lặp nhiều lần với cùng giá trị tick ms |
| `locked` | Đã xử lý một lần nhấn, chờ nhả hết mới nhận lần khác |
| `pending` | Một sự kiện chức năng đang chờ ứng dụng lấy |
| `pending_position` | Vị trí 1–9 dùng cho chế độ kiểm tra |

Các biến đều được đặt về trạng thái không phím lúc khởi tạo. `pending` là một ô chờ sự kiện, không phải hàng đợi nhiều phần tử.

### 2.4. Chọn từng cột để đọc hàng

```c
static void column_on(unsigned char col)
{
    columns_high();
    if (col==0) KEY_COL1_PORT=0;
    else if (col==1) KEY_COL2_PORT=0;
    else KEY_COL3_PORT=0;
}
```

Mỗi lần chọn cột, hàm đưa cả ba cột lên cao rồi hạ đúng một cột. Ví dụ chọn COL2 thì COL1=1, COL2=0, COL3=1.

Hàm quét toàn ma trận:

```c
static unsigned char scan_raw(void)
{
    unsigned char row,col,found=KEY_RELEASED;
    unsigned char pressed[3];
    for (col=0; col<3; ++col) {
        column_on(col);
        /* GPIO reads allow the selected column to settle without delay. */
        pressed[0]=(unsigned char)!KEY_ROW1_PIN;
        pressed[1]=(unsigned char)!KEY_ROW2_PIN;
        pressed[2]=(unsigned char)!KEY_ROW3_PIN;
        for (row=0; row<3; ++row) {
            if (pressed[row]) {
                if (found!=KEY_RELEASED) {
                    columns_high();
                    return KEY_MULTIPLE;
                }
                found=(unsigned char)(row*3+col+1);
            }
        }
    }
    columns_high();
    return found;
}
```

Với phím 5, khi COL2 thấp thì ROW2 được đọc thấp. Trong vòng lặp chỉ số bắt đầu từ 0, đó là `row=1`, `col=1`:

```text
position = row*3 + col + 1
         = 1*3 + 1 + 1 = 5
```

Code đọc ba GPIO ngay sau khi đổi cột, không dùng `delay_ms()` trong hàm quét. Sau khi quét xong hoặc phát hiện nhiều phím, `columns_high()` trả các cột về trạng thái nghỉ.

### 2.5. Từ chối lượt quét có nhiều phím

`found` ban đầu bằng 0. Khi thấy vị trí đầu tiên, hàm lưu số phím; nếu còn thấy vị trí nhấn thứ hai trong cùng lượt quét, nó trả về `KEY_MULTIPLE`.

Lớp xử lý sau không phát lệnh điều khiển cho giá trị này. Đây là cơ chế từ chối kết quả quét nhiều vị trí của game, không phải hỗ trợ nhập nhiều phím đồng thời hoặc bảo đảm loại bỏ mọi tình huống ghosting của bàn phím ma trận.

### 2.6. Chống dội bằng thời gian thay vì đứng chờ

```c
void keypad_update(unsigned long now_ms)
{
    unsigned char raw;
    if (scanned && now_ms==last_scan) return;
    scanned=1;
    last_scan=now_ms;
    raw=scan_raw();
    if (raw!=candidate) {
        candidate=raw;
        changed_at=now_ms;
        return;
    }
    if (raw==stable || (unsigned long)(now_ms-changed_at)<KEY_DEBOUNCE_MS) return;
    stable=raw;
    if (stable==KEY_RELEASED) locked=0;
    else if (!locked) {
        locked=1;
        if (stable!=KEY_MULTIPLE) {
            pending=event_from_position(stable);
            pending_position=stable;
        }
    }
}
```

Đọc hàm theo thứ tự:

1. Nếu đã quét với đúng `now_ms` này thì trả về; một tick không bị quét lặp liên tục trong vòng `while` nhanh.
2. Đọc giá trị thô bằng `scan_raw()`.
3. Nếu khác `candidate`, ghi ứng viên mới và `changed_at=now_ms`, rồi chờ các lượt sau.
4. Nếu giá trị đã giống `stable` thì không có chuyển trạng thái cần xử lý.
5. Nếu chưa ổn định đủ `KEY_DEBOUNCE_MS=20`, tiếp tục chờ.
6. Khi đủ thời gian, đưa giá trị mới vào `stable` và xử lý nhấn/nhả.

Nếu tiếp điểm dội qua lại 0/5/0/5, mỗi lần đổi đều cập nhật `changed_at`; chỉ đoạn cuối giữ nguyên đủ 20 ms mới được chấp nhận. Trong thời gian chờ, vòng chính vẫn quét LED và thực hiện các công việc khác.

So sánh thời gian dùng phép trừ `unsigned long`. Nó cho phép tính khoảng thời gian qua điểm tràn bộ đếm 32 bit khi khoảng chênh lệch nằm trong một chu kỳ đếm.

### 2.7. Khóa sự kiện cho đến khi nhả ổn định

Khi `stable==KEY_RELEASED`, chương trình đặt `locked=0`. Với một phím đã ổn định, chỉ khi chưa khóa mới đặt `locked=1` và tạo sự kiện.

Giữ phím không phát thêm vì giá trị thô vẫn bằng `stable`, hoặc bộ quét vẫn đang khóa. Đổi trực tiếp từ giữ một phím sang phím khác mà không nhả hết đủ 20 ms cũng không tạo lần nhấn mới.

Nếu `stable` là MULTIPLE, chương trình vẫn khóa nhưng không phát sự kiện. Cần nhả tất cả ổn định rồi nhấn lại một phím. Với 3/7, hàm ánh xạ trả về NONE nhưng quy tắc chờ nhả vẫn áp dụng.

### 2.8. Đổi vị trí sang chức năng và lấy sự kiện một lần

```c
static KeyEvent event_from_position(unsigned char key)
{
    /* Row-major key positions per docs/so_do_chan_ban_phim.jpeg. */
    switch (key) {
    case 1: return KEY_MENU;
    case 2: return KEY_UP;
    case 8: return KEY_DOWN;
    case 4: return KEY_LEFT;
    case 6: return KEY_RIGHT;
    case 5: return KEY_CONFIRM;
    case 9: return KEY_RESTART;
    default: return KEY_NONE;
    }
}
```

```c
KeyEvent keypad_get_event(void)
{
    KeyEvent result=pending;
    pending=KEY_NONE;
    pending_position=KEY_RELEASED;
    return result;
}
```

`keypad_get_event()` sao chép `pending` vào biến trả về rồi xóa ô chờ. Lần gọi kế tiếp không nhận lại cùng sự kiện.

`main.c` gọi `keypad_update(now_ms)` rồi `keypad_get_event()` trong vòng lặp. Menu và game nhận cùng kiểu sự kiện, nhưng diễn giải theo ngữ cảnh: phím 5 xác nhận trong menu và được bỏ qua trong ván.

Vì chỉ có một ô chờ, nếu ứng dụng không lấy sự kiện mà một lần nhấn mới hợp lệ đến sau đó, sự kiện mới có thể thay nội dung chờ. Luồng chính hiện đọc sự kiện mỗi lượt phục vụ nên thiết kế không dùng một hàng đợi dài.

### 2.9. Nhận hướng rẽ và chặn quay ngược

Trong `snake_handle_key()`, sau nhánh Restart:

```c
if (state!=SNAKE_RUNNING || turn_queued) return;
if (key==KEY_UP) requested=DIR_UP;
else if (key==KEY_DOWN) requested=DIR_DOWN;
else if (key==KEY_LEFT) requested=DIR_LEFT;
else if (key==KEY_RIGHT) requested=DIR_RIGHT;
else return;
if (requested==direction || ((requested+2)&3)==direction) return;
next_direction=requested;
turn_queued=1;
```

Hướng được đánh số theo vòng `UP=0, RIGHT=1, DOWN=2, LEFT=3`. `((requested+2)&3)` lấy hướng đối diện với yêu cầu. Nếu hướng đối diện đó là hướng hiện tại, lệnh bị bỏ qua để rắn không quay đầu trực tiếp vào cổ.

Một lệnh rẽ hợp lệ ghi vào `next_direction`, đặt `turn_queued=1`. Khi đã có cờ, các lệnh rẽ khác bị bỏ qua cho đến bước đi kế tiếp. Điều này ngăn chuỗi phím nhanh tạo nhiều lần rẽ khi đầu rắn chưa kịp di chuyển một ô.

Ở đầu `snake_step()`, lệnh chờ được đưa vào `direction` rồi cờ bị xóa. Phím không tự dịch đầu rắn ngay lập tức; Timer và chu kỳ Level quyết định khi nào gọi bước đi.

### 2.10. Chế độ kiểm tra vị trí hàng/cột

`keypad_get_press()` lấy `pending_position`, tính hàng/cột theo công thức `(position-1)/3+1` và `(position-1)%3+1`, rồi xóa dữ liệu chờ. Ví dụ vị trí 8 trả hàng 3, cột 2.

Đường này dùng cho `KEYPAD_TEST_MODE`, còn game bình thường dùng `keypad_get_event()`. Cả hai cùng tiêu thụ dữ liệu chờ nên không nên coi chúng là hai hàng đợi độc lập.

Chống dội và chu kỳ quét phụ thuộc tick mili giây của Timer1, nên clock dự án và cấu hình mô phỏng phải khớp. Sơ đồ hoạt động của Timer cũng được giải thích trong [SINH_VAT_CAN.md](SINH_VAT_CAN.md).
